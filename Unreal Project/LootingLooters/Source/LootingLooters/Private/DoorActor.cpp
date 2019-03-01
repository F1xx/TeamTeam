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



ADoorActor::ADoorActor()
{
	//By default the doors won't block anything. Feel free to adjust this if needed.
	GetStaticMeshComponent()->SetCollisionProfileName(FName("OverlapAllDynamic"));
	GetStaticMeshComponent()->SetSimulatePhysics(false);
	GetStaticMeshComponent()->SetEnableGravity(false);

	//Hardcoded. I think we only ever use 1 door. If not this will change.

// 	FString FilePath;
// #ifdef UE_BUILD_RELEASE
// 	FilePath += "Blueprint'";
// #endif
// 	FString NameTag = "ProperDoor";
// 	FilePath += "/Game/Assets/BuildingStructure/Door/";
// 
// 	FString DoorFile = FilePath + NameTag;
// 
// #ifdef UE_BUILD_RELEASE
// 	DoorFile += "." + NameTag + "'";
// #endif
	//GetStaticMeshComponent()->SetStaticMesh(ConstructorHelpers::FObjectFinder<UStaticMesh>(*DoorFile).Object);

	//Arrow for controlling the spawn facing
	ArrowComponent = CreateDefaultSubobject<UArrowComponent>("Arrow");
	ArrowComponent->SetupAttachment(RootComponent);

	//Collision for the sphere. We only collide with pawns and our goal is to teleport the pawn if it touches.
	Sphere = CreateDefaultSubobject<USphereComponent>("Collision");
	Sphere->SetSphereRadius(25.0f);
	Sphere->SetSimulatePhysics(false);
	Sphere->SetEnableGravity(false);
	Sphere->SetCollisionProfileName("OverlapOnlyPawn"); //Note that this means it blocks everything that isn't a pawn
	Sphere->SetupAttachment(RootComponent);
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &ADoorActor::TeleportPawnToOtherDoor);

	Tags.Add("Door");

	SetReplicates(true);
}

bool ADoorActor::IsConnected()
{
	return Connector != nullptr;
}

void ADoorActor::ApplyConnection(ADoorActor* OtherDoor)
{
	check(Connector == nullptr);

	Connector = OtherDoor;

	//Make the door visible now that it's connected
	GetStaticMeshComponent()->SetVisibility(true);
	Sphere->SetHiddenInGame(false, true);
	Sphere->SetCollisionProfileName("OverlapOnlyPawn");
	PrimaryActorTick.bCanEverTick = true;
}

void ADoorActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	//if the door lacks a connection it should be invisible and unheard of until it receives one.
	if (IsConnected() == false)
	{
		//While not connected make the door invisible
		GetStaticMeshComponent()->SetVisibility(false);
		Sphere->SetHiddenInGame(true, true);
		Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PrimaryActorTick.bCanEverTick = false;
	}
}

void ADoorActor::TeleportPawnToOtherDoor(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	//If the other door isnt set then this collision function doesn't do anything.
	if (Connector != nullptr)
	{

		//Fetch the other doors location and arrow.
		FVector NewLocation = Connector->GetActorLocation() + FVector(0.0f,0.0f,20.0f);
		FRotator Direction = Connector->ArrowComponent->GetComponentQuat().Rotator();
		Direction.Normalize();

		//Move the pawn to the other door (with some buffer room) and rotate him to face the arrow components direction.
		OtherActor->SetActorLocation((Direction.Vector() * TELEPORT_DISTANCE_FROM_DOOR) + NewLocation);
		OtherActor->SetActorRotation(Direction);

		//Adjust character variables if its a character
		ABaseCharacter* character = Cast<ABaseCharacter>(OtherActor);
		if (character)
		{

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
