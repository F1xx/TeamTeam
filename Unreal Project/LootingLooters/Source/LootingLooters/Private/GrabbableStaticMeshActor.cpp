// Fill out your copyright notice in the Description page of Project Settings.

#include "GrabbableStaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "BaseCharacter.h"
#include "Camera/CameraComponent.h"
#include "PlayerCharacter.h"




AGrabbableStaticMeshActor::AGrabbableStaticMeshActor()
{
	PrimaryActorTick.bCanEverTick = true;

	//By default the static mesh will physically block everything.
	GetStaticMeshComponent()->SetCollisionProfileName(FName("BlockAllDynamic"));
	GetStaticMeshComponent()->SetSimulatePhysics(true);

	//Add a sphere for the trace channel which is slightly bigger than the object and allows the player to grab things easier.
	Sphere = CreateDefaultSubobject<USphereComponent>("Collision");
	Sphere->SetupAttachment(RootComponent);
	Sphere->SetCollisionProfileName("GrabbableTrace"); //this line is integral for this actor to be seen as "grabbable"

	Tags.Add("Grabbable");
}

void AGrabbableStaticMeshActor::BeginPlay()
{
	Super::BeginPlay();

	m_Character = nullptr;
	m_CamForward = FVector::ZeroVector;
	m_Rotation = GetActorRotation();
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
	if (acharacter)
	{
		m_Character = acharacter;

		bIsHeld = !bIsHeld;
		bIsGravityOn = !bIsGravityOn;
		GetStaticMeshComponent()->SetEnableGravity(bIsGravityOn);
		GetStaticMeshComponent()->SetSimulatePhysics(bIsHeld ? false : true);

		//this line may be a problem, we'll see
		GetStaticMeshComponent()->SetCollisionEnabled(bIsHeld ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics);

		if (bIsHeld == false) //Drop
		{
			//its not held anymore so forget who was holding us
			m_Character = nullptr;
			m_CamForward = FVector::ZeroVector;
		}
	}
}

void AGrabbableStaticMeshActor::Throw()
{
	bIsHeld = false;
	bIsGravityOn = true;
	GetStaticMeshComponent()->SetEnableGravity(bIsGravityOn);
	GetStaticMeshComponent()->SetSimulatePhysics(true);
	GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	if (m_Character)
	{
		APlayerCharacter* player = Cast<APlayerCharacter>(m_Character);

		if (player) //if its a player throw with the camera
		{
			m_CamForward = player->GetCamera()->GetForwardVector();
			GetStaticMeshComponent()->AddForce(m_CamForward * 100000 * GetStaticMeshComponent()->GetMass());
		}
		else // if its a guard just throw forward
		{
			m_CamForward = m_Character->GetActorForwardVector();
			GetStaticMeshComponent()->AddForce(m_CamForward * 100000 * GetStaticMeshComponent()->GetMass());
		}
	}
	//its not held anymore so forget who was holding us
	m_Character = nullptr;
	m_CamForward = FVector::ZeroVector;
}

//Rotate the held object
void AGrabbableStaticMeshActor::RotateX(float Value)
{
	if (m_Character)
	{
		m_Rotation.Yaw += Value * 5;
	}
}

void AGrabbableStaticMeshActor::RotateY(float Value)
{
	if (m_Character)
	{
		m_Rotation.Roll += Value * 5;
	}
}

void AGrabbableStaticMeshActor::Zoom(float Value)
{
	m_Distance += Value;

	FMath::Clamp(m_Distance, 150.0f, 500.0f);

	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Emerald, FString("Zooming: " + FString::SanitizeFloat(m_Distance)));
}
