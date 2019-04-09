// Fill out your copyright notice in the Description page of Project Settings.

#include "GrabbableStaticMeshActor.h"
#include "DestructibleActor.h"
#include "DestructibleComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "BaseCharacter.h"
#include "Camera/CameraComponent.h"
#include "PlayerCharacter.h"
#include "Engine/StaticMeshActor.h"
#include "HealthComponent.h"

#include "Net/UnrealNetwork.h"

AGrabbableStaticMeshActor::AGrabbableStaticMeshActor()
	: DestructibleMesh(nullptr)
{
	PrimaryActorTick.bCanEverTick = true;

	SetReplicates(true);
	SetReplicateMovement(true);
	bAlwaysRelevant = true;

	Health = CreateDefaultSubobject<UHealthComponent>("Health Component");

	Tags.Add("Grabbable");
}

void AGrabbableStaticMeshActor::BeginPlay()
{
	Super::BeginPlay();

	m_Character = nullptr;
	m_CamForward = FVector::ZeroVector;
	m_Rotation = GetActorRotation();
	GetDestructibleComponent()->SetCanEverAffectNavigation(true); //make sure the navmesh makes the AI try to avoid these
	//GetDestructibleComponent()->SetNotifyRigidBodyCollision(true);

	//register for onhit so we can fracture ourselves potentially
	DestructibleMesh->OnComponentHit.AddDynamic(this, &AGrabbableStaticMeshActor::OnHit);

	//Setup our function to be called when the mesh breaks
	DestructibleMesh->OnComponentFracture.AddDynamic(this, &AGrabbableStaticMeshActor::OnFracture);
}

void AGrabbableStaticMeshActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	DestructibleMesh = GetDestructibleComponent();

	//By default the static mesh will physically block everything.
	DestructibleMesh->SetCollisionProfileName(FName("GrabbableTrace"));
	DestructibleMesh ->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DestructibleMesh->SetSimulatePhysics(true);
	DestructibleMesh->SetNotifyRigidBodyCollision(true);
	DestructibleMesh->SetEnableGravity(true);
	DestructibleMesh->SetIsReplicated(true);

	Health->OnDeath.AddDynamic(this, &AGrabbableStaticMeshActor::BreakMesh);
}

//Updates the object
//in this case that means setting position and rotation
void AGrabbableStaticMeshActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority())
	{
		if (m_Character)
		{
			//Set the interaction point to be in front of the camera
			FVector campos;
			FRotator camrot;

			AController* control = m_Character->GetController();
			if (control)
				control->GetPlayerViewPoint(campos, camrot); //Fills with info from the camera

			FVector end = campos + (camrot.Vector() * m_Distance);

			SetActorLocationAndRotation(end, m_Rotation);
		}
	}
}

//Pickup this object if not already
//if already holding it, throw it instead
void AGrabbableStaticMeshActor::Pickup(ABaseCharacter* acharacter)
{
	verify(acharacter != nullptr && "Weapon's attach function called with null character!");

	m_Character = acharacter;

	if (Role == ROLE_Authority)
	{
		DestructibleMesh->SetSimulatePhysics(false);
		DestructibleMesh->SetEnableGravity(false);
		SetActorEnableCollision(false);
	}
}

//remove the object from its player. It has been dropped. Return to being an object.
void AGrabbableStaticMeshActor::Drop()
{
	m_Character = nullptr;
	m_CamForward = FVector::ZeroVector;

	if (Role == ROLE_Authority)
	{
		SetActorEnableCollision(true);
		DestructibleMesh->SetEnableGravity(true);
		DestructibleMesh->SetSimulatePhysics(true);
	}
}

//Makes sure the server calls multicast breakmesh to make the mesh break for everyone.
//Called by this object's health component's OnDeath
void AGrabbableStaticMeshActor::BreakMesh_Implementation(AActor* actor)
{
	if (HasAuthority())
	{
		MulticastBreakMesh(actor);
	}
}

bool AGrabbableStaticMeshActor::BreakMesh_Validate(AActor* actor)
{
	return true;
}

//Apply a force to this object using the character's forward vector to simulate a throw
//The result of throwing can result in this actor fracturing
void AGrabbableStaticMeshActor::Throw()
{
	if (Role == ROLE_Authority)
	{
		SetActorEnableCollision(true);
		DestructibleMesh->SetEnableGravity(true);
		DestructibleMesh->SetSimulatePhysics(true);
		DestructibleMesh->SetNotifyRigidBodyCollision(true);
		bWasThrown = true;

		if (m_Character)
		{
			APlayerCharacter* player = Cast<APlayerCharacter>(m_Character);

			if (player) //if its a player throw with the camera
			{
				m_CamForward = player->GetControlRotation().Vector(); //unit vector and required to be like this for replication
				DestructibleMesh->AddForce(m_CamForward * 40000 * DestructibleMesh->GetMass());
			}
			else // if its a guard just throw forward
			{
				m_CamForward = m_Character->GetActorForwardVector();
				DestructibleMesh->AddForce(m_CamForward * 40000 * DestructibleMesh->GetMass());
			}
		}
	}
	Drop();
}

//Rotate the held object on yaw
void AGrabbableStaticMeshActor::RotateX(float Value)
{
	if (m_Character)
	{
		m_Rotation.Yaw += Value * 5;
	}
}

//Rotate the held object on Roll
void AGrabbableStaticMeshActor::RotateY(float Value)
{
	if (m_Character)
	{
		m_Rotation.Roll += Value * 5;
	}
}

//Zoom the object in/out
void AGrabbableStaticMeshActor::Zoom(float Value)
{
	m_Distance += Value;

	m_Distance = FMath::Clamp(m_Distance, 150.0f, 500.0f);

	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Emerald, FString("Zooming: " + FString::SanitizeFloat(m_Distance)));
}

//destroy the object
void AGrabbableStaticMeshActor::Die()
{
	Destroy();
}

//Function called when this actor's collision component is hit
//if it was thrown fracture it
void AGrabbableStaticMeshActor::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
 	if (Role == ROLE_Authority)
 	{
 		if (bWasThrown)
 		{
 			TakeDamage(Health->MaximumHealth, FDamageEvent(), nullptr, OtherActor);
 			OtherActor->TakeDamage(Health->MaximumHealth * 0.5f, FDamageEvent(), nullptr, OtherActor);
 		}
 		bWasThrown = false;
 	}
}

// We want to turn off the ability to grab this actor after its been exploded
void AGrabbableStaticMeshActor::OnFracture(const FVector& HitPosition, const FVector& HitDirection)
{
	DestructibleMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);

	//Spawn loot, preferably through game mode
}

void AGrabbableStaticMeshActor::MulticastBreakMesh_Implementation(AActor* actor)
{
	if (actor)
	{
		FVector direction = actor->GetActorLocation() - GetActorLocation();

		DestructibleMesh->ApplyDamage(500.0f, GetActorLocation(), direction.GetSafeNormal(), 0.0f);
		DestructibleMesh->SetEnableGravity(true);
		GetWorldTimerManager().SetTimer(DespawnTimer, this, &AGrabbableStaticMeshActor::Die, TimeBeforeDespawn, false);
	}
}

void AGrabbableStaticMeshActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGrabbableStaticMeshActor, DestructibleMesh);
	DOREPLIFETIME(AGrabbableStaticMeshActor, m_Character);
	DOREPLIFETIME(AGrabbableStaticMeshActor, Health);
	DOREPLIFETIME(AGrabbableStaticMeshActor, DespawnTimer);
	DOREPLIFETIME(AGrabbableStaticMeshActor, bWasThrown);
}