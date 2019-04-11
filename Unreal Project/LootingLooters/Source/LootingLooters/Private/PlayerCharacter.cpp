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
#include "LootingLootersGameModeBase.h"
#include "Sound/SoundWave.h"
#include "Components/AudioComponent.h"

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

	m_Music = CreateDefaultSubobject<UAudioComponent>("Music");
	m_ChaseMusic = CreateDefaultSubobject<UAudioComponent>("ChaseMusic");
	m_LootSound = CreateDefaultSubobject<USoundWave>("LootSound");

	Tags.Add("Player");
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimer(PostBeginPlayDelay, this, &APlayerCharacter::PostBeginPlay, 1.0f, false);
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
	else if (!GetGameState()->PlayerThreeLoggedIn)
	{
		Team = 2;
		GetGameState()->PlayerThreeLoggedIn = true;
	}
	else if (!GetGameState()->PlayerFourLoggedIn)
	{
		Team = 3;
		GetGameState()->PlayerFourLoggedIn = true;
	}
}

void APlayerCharacter::Multicast_AssignDefaultMaterial_Implementation()
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
	}
}

void APlayerCharacter::NetMulticast_SetColor_Implementation()
{
	GetMesh()->SetMaterial(0, DefaultMaterial);
	GetMesh()->SetMaterial(1, DefaultMaterial);
}

void APlayerCharacter::Server_PlayLootSound_Implementation(FVector location)
{
	NetMultiCast_PlayLootSound(location);
}

bool APlayerCharacter::Server_PlayLootSound_Validate(FVector location)
{
	return true;
}

void APlayerCharacter::NetMultiCast_PlayLootSound_Implementation(FVector location)
{
	if (m_LootSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, m_LootSound, location);
	}
}

void APlayerCharacter::PostBeginPlay()
{
	if (Role == ROLE_Authority)
		NetMulticast_SetColor();

	Client_StartMusic();
}

void APlayerCharacter::NetMultiCast_DisableControllerInputs_Implementation()
{
	APlayerController* controller = Cast<APlayerController>(GetController());

	if (controller)
		controller->DisableInput(controller);
}

void APlayerCharacter::Server_BeingChased_Implementation(bool chased)
{
	Client_BeingChased(chased);
}

bool APlayerCharacter::Server_BeingChased_Validate(bool chased)
{
	return true;
}

void APlayerCharacter::Client_BeingChased_Implementation(bool chased)
{
	if (chased)
	{
		if (m_ChaseMusic->IsPlaying() == false)
		{
			m_Music->FadeOut(1.0f, 0.5f);
			m_ChaseMusic->FadeIn(1.0f);
		}
	}
	else
	{
		if (m_Music->IsPlaying() == false)
		{
			m_Music->FadeIn(2.0f);
			m_ChaseMusic->FadeOut(5.0f, 0.5f);
		}
	}
}

void APlayerCharacter::Client_StartMusic_Implementation()
{
	m_Music->FadeIn(4.0f);
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
	RespawnLoc = GetTransform();
	Super::Die();

	GetPlayerState()->Reset();

	APlayerController* cont = Cast<APlayerController>(GetController());

	if (cont)
	{
		cont->DisableInput(cont);
		GetCapsuleComponent()->SetHiddenInGame(true);
	}

	GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &APlayerCharacter::Respawn, 5.0f, false);
}

void APlayerCharacter::Respawn()
{
	APlayerController* cont = Cast<APlayerController>(GetController());

	if (cont && HasAuthority())
	{
		ALootingLootersGameModeBase* GM = Cast<ALootingLootersGameModeBase>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			GM->RespawnPlayer(cont, Team, RespawnLoc);
			m_Music->Stop();
			m_ChaseMusic->Stop();

			Destroy();
		}
	}
}

void APlayerCharacter::NetMulticastOnRespawn_Implementation()
{
	//disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SphereComp->SetCollisionProfileName("OverlapOnlyPawn");
	SphereComp->SetVisibility(true, true);

	//we dead
	bIsDead = false;

	//ragdoll time
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetSimulatePhysics(false);
}

bool APlayerCharacter::NetMulticastOnRespawn_Validate()
{
	return true;
}

void APlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	m_LootSound->bLooping = false;
	m_ChaseMusic->Stop();
	m_Music->VolumeMultiplier = 0.25f;
	m_Music->Stop();
}

//Calls ABaseCharacter's Interact
//If we are not interacting from that then we will check if its loot (this is here because other character types cannot loot)
void APlayerCharacter::Interact()
{
	Super::Interact();

	//if after super we're still not interacting with anything check if we can grab some loot
	if (!bIsInteracting)
	{
		FHitResult Hit(ForceInit);

		if (PerformRayCast(FName("GrabbableTrace"), Hit))
		{
			//Making sure what we hit was Loot
			if (Hit.GetActor()->ActorHasTag("Loot"))
			{
				Server_PlayLootSound(GetActorLocation());
				m_Inventory->ServerCollectLoot(Hit.GetActor());
			}
		}
	}
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
	DOREPLIFETIME(APlayerCharacter, RespawnTimer);
	DOREPLIFETIME(APlayerCharacter, RespawnLoc);
	DOREPLIFETIME(APlayerCharacter, m_LootSound);
}