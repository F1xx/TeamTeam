// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GrabbableStaticMeshActor.h"
#include "Camera/PlayerCameraManager.h"
#include "PlayerCharacterState.h"
#include "InventoryComponent.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

APlayerCharacter::APlayerCharacter() : Super()
{
	PrimaryActorTick.bCanEverTick = true;

	//CAMERA
	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(RootComponent); //Root is Capsule in ACharacter.cpp
	Camera->bUsePawnControlRotation = true;
	Camera->SetRelativeLocation(FVector(0.0f, 0.0f, BaseEyeHeight));

	m_Inventory = CreateDefaultSubobject<UInventoryComponent>("Inventory");

	//Make it so we can't see our own mesh but others can
	GetMesh()->SetOwnerNoSee(true);

	//Player state
	m_PlayerState = CreateDefaultSubobject<APlayerCharacterState>("PlayerState");
	m_PlayerState->Score = 0;

	Tags.Add("Player");
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	//UGameplayStatics::GetPlayerCameraManager(this, 0)->bEnableFading = true;
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent * PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void APlayerCharacter::RotateHeldObjectY(float Value)
{
	if (bIsRotating)
	{
		if (HeldObject && bIsInteracting)
		{
			HeldObject->RotateY(Value);
		}
	}
}

void APlayerCharacter::RotateHeldObjectX(float Value)
{
	if (bIsRotating)
	{
		if (HeldObject && bIsInteracting)
		{
			HeldObject->RotateX(Value);
		}
	}
}

UCameraComponent* APlayerCharacter::GetCamera() const
{
	return Camera;
}

void APlayerCharacter::Interact()
{
	Super::Interact();

	//if after super we're still not interacting with anything check if we can grab some loot
	if (!bIsInteracting)
	{
		//A struct that the trace will populate with the results of the hit
		FHitResult Hit(ForceInit);

		//raycast to see
		bool result = PerformRayCast(FName("GrabbableTrace"), Hit);

		//if it succeeds do something with it
		if (result)
		{
			//Making sure what we hit was grabbable
			if (Hit.GetActor()->ActorHasTag("Loot"))
			{
				Hit.GetActor()->Destroy();
				
				m_PlayerState->Score += m_Inventory->CollectLoot();
			}
		}
	}
}

//place a trap. This is for the player and will place it where they're aiming
void APlayerCharacter::PlaceTrap()
{
	//if we have traps
	if (m_Inventory->GetTrapCount() > 0)
	{
		//A struct that the trace will populate with the results of the hit
		FHitResult Hit(ForceInit);

		//raycast to see
		bool result = PerformRayCast(FName("GrabbableTrace"), Hit);

		//if it succeeds do something with it
		if (result)
		{
			m_Inventory->PlaceTrap(Hit.ImpactPoint);
		}
	}
}

void APlayerCharacter::NextInventory()
{
	m_Inventory->NextInventoryItem();
}

void APlayerCharacter::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Magenta, FString("Score: " + FString::SanitizeFloat(m_PlayerState->Score)));
}