// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter.h"
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
#include "InventoryComponent.h"
#include "PlayerCharacterController.h"
#include "MyPlayerState.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "LootingLootersGameStateBase.h"

APlayerCharacter::APlayerCharacter() : Super()
{
	PrimaryActorTick.bCanEverTick = true;
	LastDoorAccessed = nullptr;

	//CAMERA
	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(RootComponent); //Root is Capsule in ACharacter.cpp
	Camera->bUsePawnControlRotation = true;
	Camera->SetRelativeLocation(FVector(0.0f, 0.0f, BaseEyeHeight));

	//Make it so we can't see our own mesh but others can
	GetMesh()->SetOwnerNoSee(true);

	SetReplicates(true);
	SetReplicateMovement(true);
	Camera->SetIsReplicated(true);

	m_Inventory = CreateDefaultSubobject<UInventoryComponent>("Inventory");

	Tags.Add("Player");
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	class APlayerCharacterController* controller = Cast<APlayerCharacterController>(GetController());

	GetWorld()->GetTimerManager().SetTimer(PostBeginPlayDelay, this, &APlayerCharacter::PostBeginPlay, 1.0f, false);

	//UGameplayStatics::GetPlayerCameraManager(this, 0)->bEnableFading = true;
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent * PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

//A control to rotate whatever object we're interacting with via the Yaxis
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

void APlayerCharacter::AssignTeam()
{
	if (!GetGameState())
		return;

	//IF TeamOneCOunt is EQUAL to TeamTwoCount
	if (!GetGameState()->PlayerOneLoggedIn)
	{
		Team = 0;
		GetGameState()->PlayerOneLoggedIn = true;
	}
	else if (!GetGameState()->PlayerTwoLoggedIn)
	{
		Team = 1;
		GetGameState()->PlayerTwoLoggedIn = true;
	}
	else if (!GetGameState()->PlayerOneLoggedIn)
	{
		Team = 2;
		GetGameState()->PlayerThreeLoggedIn = true;
	}
	else if (!GetGameState()->PlayerTwoLoggedIn)
	{
		Team = 3;
		GetGameState()->PlayerFourLoggedIn = true;
	}
}

void APlayerCharacter::Multicast_AssignColor_Implementation()
{
	if (GetGameState())
	{
		if (Team == 0)
		{
			DefaultMaterial = GetGameState()->TeamOneMaterials;
		}
		else if (Team == 1)
		{
			DefaultMaterial = GetGameState()->TeamTwoMaterials;
		}
		else if (Team == 2)
		{
			DefaultMaterial = GetGameState()->TeamThreeMaterials;
		}
		else if (Team == 3)
		{
			DefaultMaterial = GetGameState()->TeamFourMaterials;
		}

		SetColor();
	}
}

void APlayerCharacter::SetColor()
{
	GetMesh()->SetMaterial(0, DefaultMaterial);
	GetMesh()->SetMaterial(1, DefaultMaterial);

	GetPlayerState()->Team = Team;
}

void APlayerCharacter::PostBeginPlay()
{
	if (Role == ROLE_Authority)
		Multicast_AssignColor();
}

//A control to rotate whatever object we're interacting with via the Xaxis
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

class AMyPlayerState* APlayerCharacter::GetPlayerState()
{
	return Cast<AMyPlayerState>(Controller->PlayerState);
}

class ALootingLootersGameStateBase* APlayerCharacter::GetGameState()
{
	return Cast<ALootingLootersGameStateBase>(GetWorld()->GetGameState());
}

//if we die remove input and go ragdoll
void APlayerCharacter::Die()
{
	Super::Die();

	APlayerController* cont = Cast<APlayerController>(GetController());

	if (cont)
	{
		cont->DisableInput(cont);
		GetCapsuleComponent()->SetHiddenInGame(true);
		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Red, "YOU WERE KILLED BY THE GUARD");
	}
}

//Calls ABaseCharacter's Interact
//If we are not interacting from that then we will check if its loot (this is here because other character types cannot loot)
void APlayerCharacter::Interact()
{
	Super::Interact();

	GetPlayerState();

// 	if (Role == ROLE_Authority)
// 		{
		//if after super we're still not interacting with anything check if we can grab some loot
		if (!bIsInteracting)
		{
			FHitResult Hit(ForceInit);

			if (PerformRayCast(FName("GrabbableTrace"), Hit))
			{
				//Making sure what we hit was Loot
				if (Hit.GetActor()->ActorHasTag("Loot"))
				{
					m_Inventory->ServerCollectLoot(Hit.GetActor());
				}
			}
		}
	//}
}

//place a trap. This is for the player and will place it where they're aiming
void APlayerCharacter::PlaceTrap()
{
	//if we have traps
	if (GetPlayerState()->TrapCount > 0)
	{
		FHitResult Hit(ForceInit);

		if (PerformRayCast(FName("GrabbableTrace"), Hit))
		{
			m_Inventory->PlaceTrap(Hit.ImpactPoint);
		}
	}
}

//cycles through the inventory forwards
void APlayerCharacter::NextInventory()
{
	m_Inventory->NextInventoryItem();
}

void APlayerCharacter::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
}

void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerCharacter, Camera);
	DOREPLIFETIME(APlayerCharacter, m_Inventory);
	DOREPLIFETIME(APlayerCharacter, DefaultMaterial);
	DOREPLIFETIME(APlayerCharacter, PostBeginPlayDelay);
	DOREPLIFETIME(APlayerCharacter, Team);
}