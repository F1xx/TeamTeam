// Fill out your copyright notice in the Description page of Project Settings.

#include "../Public/DoorActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/ArrowComponent.h"
#include "BaseCharacter.h"
#include "Camera/CameraComponent.h"
#include "PlayerCharacter.h"
#include "GameFramework/Controller.h"
#include "ConstructorHelpers.h"
#include "Net/UnrealNetwork.h"



ADoorActor::ADoorActor()
{
	SetReplicates(true);
	PrimaryActorTick.bCanEverTick = true;

	//By default the doors won't block anything. Feel free to adjust this if needed.
	GetStaticMeshComponent()->SetCollisionProfileName(FName("OverlapAllDynamic"));
	GetStaticMeshComponent()->SetSimulatePhysics(false);
	GetStaticMeshComponent()->SetEnableGravity(false);

	//Arrow for controlling the spawn facing
	ArrowComponent = CreateDefaultSubobject<UArrowComponent>("Arrow");
	ArrowComponent->SetupAttachment(RootComponent);

	//Collision for the sphere. We only collide with pawns and our goal is to teleport the pawn if it touches.
	Sphere = CreateDefaultSubobject<USphereComponent>("Collision");
	Sphere->SetCollisionProfileName("OverlapOnlyPawn");
	Sphere->SetSphereRadius(25.0f);
	Sphere->SetSimulatePhysics(false);
	Sphere->SetEnableGravity(false);
	Sphere->SetVisibility(false, false);
	Sphere->SetupAttachment(RootComponent);
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &ADoorActor::TeleportPawnToOtherDoor);

	Tags.Add("Door");
}

bool ADoorActor::IsConnected()
{
	return Connector != nullptr;
}

void ADoorActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void ADoorActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	//Adjust collision and visibility if connected or not.
	if (IsConnected())
	{	
		GetStaticMeshComponent()->SetVisibility(true);
		Sphere->SetCollisionProfileName("OverlapOnlyPawn");
	}
	else
	{
		GetStaticMeshComponent()->SetVisibility(false);
		Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PrimaryActorTick.bCanEverTick = false;
	}
}

void ADoorActor::ApplyConnection(ADoorActor* OtherDoor)
{
	Connector = OtherDoor;
	PrimaryActorTick.bCanEverTick = true;
}

void ADoorActor::TeleportPawnToOtherDoor(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	//If the other door isnt set then this collision function doesn't do anything.
	if (IsConnected())
	{
		ABaseCharacter* character = Cast<ABaseCharacter>(OtherActor);
		if (character)
		{
			//Fetch the other doors location and arrow.
			FVector NewLocation = Connector->GetActorLocation() + FVector(0.0f, 0.0f, 20.0f);
			FRotator Direction = Connector->ArrowComponent->GetComponentQuat().Rotator();
			Direction.Normalize();

			//Move the pawn to the other door (with some buffer room) and rotate him to face the arrow components direction.
			OtherActor->SetActorLocation((Direction.Vector() * TELEPORT_DISTANCE_FROM_DOOR) + NewLocation);
			OtherActor->SetActorRotation(Direction);



			//Get the camera
			AController* Controller = character->GetController();

			//If the camera exists, snap it to adjust.
			if (Controller != nullptr)
			{
				Controller->SetControlRotation(Direction);
			}

			//set last door used to this one
			character->SetLastDoorAccessed(this);
		}
	}
}


void ADoorActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADoorActor, Connector);
}
