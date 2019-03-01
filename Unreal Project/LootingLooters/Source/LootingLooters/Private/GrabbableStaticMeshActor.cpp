// Fill out your copyright notice in the Description page of Project Settings.

#include "GrabbableStaticMeshActor.h"
#include "DestructibleActor.h"
#include "DestructibleComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "BaseCharacter.h"
#include "Camera/CameraComponent.h"
#include "PlayerCharacter.h"




AGrabbableStaticMeshActor::AGrabbableStaticMeshActor()
{
	PrimaryActorTick.bCanEverTick = true;

	//By default the static mesh will physically block everything.
	GetDestructibleComponent()->SetCollisionProfileName(FName("GrabbableTrace"));
	GetDestructibleComponent()->SetSimulatePhysics(true);
	GetDestructibleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetDestructibleComponent()->SetNotifyRigidBodyCollision(true);

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
	GetDestructibleComponent()->OnComponentHit.AddDynamic(this, &AGrabbableStaticMeshActor::OnHit);
	//Setup our function to be called when the mesh breaks
	GetDestructibleComponent()->OnComponentFracture.AddDynamic(this, &AGrabbableStaticMeshActor::OnFracture);
}

// We want to turn off the ability to grab this actor after its been exploded
void AGrabbableStaticMeshActor::OnFracture(const FVector& HitPosition, const FVector& HitDirection)
{
	GetDestructibleComponent()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
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
AGrabbableStaticMeshActor* AGrabbableStaticMeshActor::Pickup(ABaseCharacter* acharacter)
{
	if (acharacter)
	{
		m_Character = acharacter;

		bIsHeld = !bIsHeld;
		bIsGravityOn = !bIsGravityOn;
		GetDestructibleComponent()->SetEnableGravity(bIsGravityOn);
		GetDestructibleComponent()->SetSimulatePhysics(!bIsHeld);

		//this line may be a problem, we'll see
		GetDestructibleComponent()->SetCollisionEnabled(bIsHeld ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics);

		if (bIsHeld == false) //Drop
		{
			//its not held anymore so forget who was holding us
			m_Character = nullptr;
			m_CamForward = FVector::ZeroVector;
			return nullptr;
		}

		return this;
	}
	return nullptr;
}

//Apply a force to this object using the character's forward vector to simulate a throw
//The result of throwing can result in this actor fracturing
void AGrabbableStaticMeshActor::Throw()
{
	bIsHeld = false;
	bIsGravityOn = true;
	GetDestructibleComponent()->SetEnableGravity(bIsGravityOn);
	GetDestructibleComponent()->SetSimulatePhysics(true);
	GetDestructibleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	if (m_Character)
	{
		APlayerCharacter* player = Cast<APlayerCharacter>(m_Character);

		if (player) //if its a player throw with the camera
		{
			m_CamForward = player->GetCamera()->GetForwardVector();
			GetDestructibleComponent()->AddForce(m_CamForward * 200000 * GetDestructibleComponent()->GetMass());
			bWasThrown = true;
		}
		else // if its a guard just throw forward
		{
			m_CamForward = m_Character->GetActorForwardVector();
			GetDestructibleComponent()->AddForce(m_CamForward * 200000 * GetDestructibleComponent()->GetMass());
		}
	}
	//its not held anymore so forget who was holding us
	//m_Character = nullptr;
	m_CamForward = FVector::ZeroVector;
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
	if (OtherActor != m_Character)
	{
		if (bWasThrown)
		{
			GetDestructibleComponent()->ApplyDamage(5000.0f, GetActorLocation(), -Hit.Normal, 0.0f);

			GetWorldTimerManager().SetTimer(DespawnTimer, this, &AGrabbableStaticMeshActor::Die, TimerBeforeDespawn, false);
		}
	}

	m_Character = nullptr;
}
