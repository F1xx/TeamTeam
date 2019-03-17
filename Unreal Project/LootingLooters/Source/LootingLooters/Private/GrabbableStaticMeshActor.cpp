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

#include "Net/UnrealNetwork.h"

AGrabbableStaticMeshActor::AGrabbableStaticMeshActor()
	: DestructibleMesh(nullptr)
{
	PrimaryActorTick.bCanEverTick = true;

	SetReplicates(true);
	SetReplicateMovement(true);

	Tags.Add("Grabbable");
}

void AGrabbableStaticMeshActor::BeginPlay()
{
	Super::BeginPlay();

	m_Character = nullptr;
	m_CamForward = FVector::ZeroVector;
	m_Rotation = GetActorRotation();
	GetDestructibleComponent()->SetCanEverAffectNavigation(true); //make sure the navmesh makes the AI try to avoid these

	//register for onhit so we can fracture ourselves potentially
	DestructibleMesh->OnComponentHit.AddDynamic(this, &AGrabbableStaticMeshActor::OnHit);
	//Setup our function to be called when the mesh breaks
	DestructibleMesh->OnComponentFracture.AddDynamic(this, &AGrabbableStaticMeshActor::OnFracture);
}

// We want to turn off the ability to grab this actor after its been exploded
void AGrabbableStaticMeshActor::OnFracture(const FVector& HitPosition, const FVector& HitDirection)
{
	DestructibleMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);

	//Spawn loot, preferably through game mode
}

void AGrabbableStaticMeshActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	DestructibleMesh = GetDestructibleComponent();

	//By default the static mesh will physically block everything.
	DestructibleMesh->SetCollisionProfileName(FName("GrabbableTrace"));
	DestructibleMesh->SetSimulatePhysics(true);
	DestructibleMesh ->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DestructibleMesh->SetNotifyRigidBodyCollision(true);
	DestructibleMesh->SetEnableGravity(true);
}

//Updates the object
//in this case that means setting position and rotation
void AGrabbableStaticMeshActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsHeld && m_Character)
	{
		//Set the interaction point to be in front of the camera
		FVector campos;
		FRotator camrot;
		m_Character->GetController()->GetPlayerViewPoint(campos, camrot); //Fills with info from the camera
		FVector end = campos + (camrot.Vector() * m_Distance);

		SetActorLocationAndRotation(end, m_Rotation);
	}
}

//Pickup this object if not already
//if already holding it, throw it instead
void AGrabbableStaticMeshActor::Pickup(ABaseCharacter* acharacter)
{
	verify(acharacter != nullptr && "Weapon's attach function called with null character!");

	m_Character = acharacter;

	bIsHeld = true;
	bIsGravityOn = false;

	if (Role == ROLE_Authority)
	{
		DestructibleMesh->SetEnableGravity(bIsGravityOn);
		SetActorEnableCollision(bIsGravityOn);
		DestructibleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		//CALL SetSimulatePhysics() and set it to true
		DestructibleMesh->SetSimulatePhysics(bIsGravityOn);
	}
}

void AGrabbableStaticMeshActor::Drop()
{
	m_Character = nullptr;
	m_CamForward = FVector::ZeroVector;
	bIsHeld = false;
	bIsGravityOn = true;

	if (Role == ROLE_Authority)
	{
		//CALL SetActorEnableCollision() and set it to true
		DestructibleMesh->SetEnableGravity(bIsGravityOn);
		SetActorEnableCollision(bIsGravityOn);
		DestructibleMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		//CALL SetSimulatePhysics() and set it to true
		DestructibleMesh->SetSimulatePhysics(bIsGravityOn);
	}
}

void AGrabbableStaticMeshActor::BreakMesh(const FHitResult& Hit)
{
	if (Role == ROLE_Authority)
	{
		DestructibleMesh->ApplyDamage(500.0f, GetActorLocation(), -Hit.Normal, 0.0f);
		DestructibleMesh->SetEnableGravity(true);
		GetWorldTimerManager().SetTimer(DespawnTimer, this, &AGrabbableStaticMeshActor::Die, TimeBeforeDespawn, false);
	}
}

//Apply a force to this object using the character's forward vector to simulate a throw
//The result of throwing can result in this actor fracturing
void AGrabbableStaticMeshActor::Throw()
{
	bIsHeld = false;
	bIsGravityOn = true;

	if (Role == ROLE_Authority)
	{
		//CALL SetActorEnableCollision() and set it to true
		DestructibleMesh->SetEnableGravity(bIsGravityOn);
		SetActorEnableCollision(bIsGravityOn);
		DestructibleMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		//CALL SetSimulatePhysics() and set it to true
		DestructibleMesh->SetSimulatePhysics(bIsGravityOn);

		if (m_Character)
		{
			APlayerCharacter* player = Cast<APlayerCharacter>(m_Character);

			if (player) //if its a player throw with the camera
			{
				m_CamForward = player->GetFollowCamera()->GetForwardVector();
				DestructibleMesh->AddForce(m_CamForward * 40000 * DestructibleMesh->GetMass());
				bWasThrown = true;
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
	if (Role == ROLE_Authority)// && IsLocallyControlled())
	{
		if (OtherActor != m_Character)
		{
			if (bWasThrown)
			{
				BreakMesh(Hit);

				AGrabbableStaticMeshActor* acto = Cast<AGrabbableStaticMeshActor>(OtherActor);

				if (acto)
				{
					acto->BreakMesh(Hit);
				}
			}
		}

		m_Character = nullptr;
	}
}

//void AGrabbableStaticMeshActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
//{
//	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//
//	DOREPLIFETIME(AGrabbableStaticMeshActor, DestructibleMesh);
//}