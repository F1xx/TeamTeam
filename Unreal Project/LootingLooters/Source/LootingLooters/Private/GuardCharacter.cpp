// Fill out your copyright notice in the Description page of Project Settings.

#include "GuardCharacter.h"
#include "NavigationSystem.h"
#include "AIModule/Classes/Blueprint/AIBlueprintHelperLibrary.h"
#include "Perception/PawnSensingComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"

#include "LootingLootersGameModeBase.h"
#include "DoorActor.h"
#include "RoomActorBase.h"
#include "PlayerCharacter.h"

AGuardCharacter::AGuardCharacter() : Super()
{
	PrimaryActorTick.bCanEverTick = true;

	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>("Sensor");
	PawnSensingComp->OnSeePawn.AddDynamic(this, &AGuardCharacter::OnPawnSeen);

	GuardState = EAIState::EPatrol;
	SetMaxSpeed(PatrolSpeed);

	SearchLocation = FVector::ZeroVector;
	TargetActor = nullptr;

	Tags.Add("Guard");
}

void AGuardCharacter::BeginPlay()
{
	Super::BeginPlay();

	m_GameMode = Cast<ALootingLootersGameModeBase>(GetWorld()->GetAuthGameMode());

	OriginalRotator = GetActorRotation();
	MoveToNextPatrolPoint();

	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AGuardCharacter::OnComponentHit);
}

//if we see a player, attack them
void AGuardCharacter::OnPawnSeen(APawn * SeenPawn)
{
	if (SeenPawn == nullptr)
		return;

	if (TargetActor && TargetActor != SeenPawn) //if we already have a target only switch if the new one is closer
	{
		FVector MyLoc = GetActorLocation();
		FVector newTargetLoc = SeenPawn->GetActorLocation();
		FVector TargetLoc = TargetActor->GetActorLocation();

		float current = FVector::DistSquared(TargetLoc, MyLoc);
		float potential = FVector::DistSquared(newTargetLoc, MyLoc);

		if (current < potential)//if the current target is closer then just quit this
			return;
	}

	TargetActor = SeenPawn;

	SetGuardState(EAIState::EAttack);
}

//if the guard find a target again before this gets called it will exit
//otherwise sets them back to patrol safely
void AGuardCharacter::ResetState()
{
	if (GuardState == EAIState::EAttack)
		return;

	SetActorRotation(OriginalRotator);
	ResetPatrol();
}

void AGuardCharacter::ResetPatrol()
{
	TargetActor = nullptr;
	SetGuardState(EAIState::EPatrol);

	MoveToNextPatrolPoint();
}

void AGuardCharacter::DetermineGuardState()
{
	//ensure the right state is set
	//if we are in a chase we'll either attack or search
	if (TargetActor)
	{
		//if we can still see our target we are attacking
		if (PawnSensingComp->HasLineOfSightTo(TargetActor))
		{
			SetGuardState(EAIState::EAttack);

			//Set this location every time so that when we lose sight this location will be the last one we had
			SearchLocation = TargetActor->GetActorLocation();	
		}
		//if we can't see our target anymore go to where they last were
		else
			SetGuardState(EAIState::ESearch);
	}
	else
		SetGuardState(EAIState::EPatrol);
}

void AGuardCharacter::SetGuardState(EAIState NewState)
{
	if (GuardState == NewState)
		return;

	GuardState = NewState;

	switch (GuardState)
	{
	case EAIState::EPatrol:
		SetMaxSpeed(PatrolSpeed);
		break;
	case EAIState::ESearch:
		SetMaxSpeed(PatrolSpeed);

		//get the last door used by the player (if there is one).
		LastDoorPlayerUsed = Cast<ABaseCharacter>(TargetActor)->GetLastDoorAccessed();

		//Reset search timer
		GetWorldTimerManager().ClearTimer(TimerHandle_ResetState);

		break;
	case EAIState::EAttack:
		SetMaxSpeed(AttackSpeed);
		break;
	}
}

void AGuardCharacter::HandleAI()
{
	//this switch actually handles the guard's states
	switch (GuardState)
	{
	case EAIState::EPatrol: //no target, patrol

		if (!CurrentPatrolPoint)
		{
			MoveToNextPatrolPoint();
		}

		if (CurrentPatrolPoint && isPatrolPointInRoom())
		{
			FVector Delta = (CurrentPatrolPoint->GetActorLocation() - GetActorLocation());
			float DistanceToGoal = Delta.Size();

			if (DistanceToGoal < 50)
			{
				MoveToNextPatrolPoint();
			}
		}
		else
			MoveToNextPatrolPoint();

		break;

	case EAIState::ESearch://lost them so go to their last position and look around

		//if we're at where the player was last seen but still can't find him, use the last door the player was known using.
		if (bSearchedLastPlayerLocation != true)
		{
			bSearchedLastPlayerLocation = FVector::Dist(GetActorLocation(), SearchLocation) <= 100;
		}


		//if we're still not near the last known player location keep going there
		if (Controller != nullptr && bSearchedLastPlayerLocation == false)
		{
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(GetController(), SearchLocation);
		}

		//if we've searched where the player was, start searching the last door the player used
		if (Controller != nullptr && bSearchedLastPlayerLocation == true)
		{
			//if that door exists, move towards it
			if (LastDoorPlayerUsed)
				UAIBlueprintHelperLibrary::SimpleMoveToActor(GetController(), LastDoorPlayerUsed);
		}

		//Make the guard give up after 3 seconds of searching
		if (!GetWorldTimerManager().IsTimerActive(TimerHandle_ResetState))
		{
			Controller->StopMovement();
			GetWorldTimerManager().SetTimer(TimerHandle_ResetState, this, &AGuardCharacter::ResetState, 3.0f, true);
			bSearchedLastPlayerLocation = false;
		}

		break;

	case EAIState::EAttack: //we can see them so chase them

		FVector MyLoc = GetActorLocation();
		FVector TargetLoc = TargetActor->GetActorLocation();
		FVector Dir = TargetLoc - MyLoc;
		Dir.Normalize();
		SetActorRotation(FMath::Lerp(GetActorRotation(), Dir.Rotation(), 0.05f));
		UAIBlueprintHelperLibrary::SimpleMoveToActor(GetController(), TargetActor);

		break;
	}
}

//use this function to find the door in the room that the guard will go to
void AGuardCharacter::MoveToNextPatrolPoint()
{
	FindNewPatrolPoint();

	UAIBlueprintHelperLibrary::SimpleMoveToActor(GetController(), CurrentPatrolPoint);
}

void AGuardCharacter::FindNewPatrolPoint()
{
	if (GetCurrentRoom())
	{
		//find what room I'm in
		TArray<ADoorActor*> Doors;
		GetCurrentRoom()->GetDoorArray(Doors);

		//Array to be filled with the guard's actual possibilities
		TArray<ADoorActor*> ViableDoors;

		//cycle through all doors in the room
		for (int i = 0; i < Doors.Num(); i++)
		{
			//if its the door we came from ignore it also if its not connected
			if (Doors[i] != LastDoorAccessed && Doors[i]->IsConnected())
			{
				ViableDoors.Add(Doors[i]);
			}
		}

		int r = FMath::RandRange(0, ViableDoors.Num() - 1);

		//LastDoorAccessed = ViableDoors[r]->Connector;
		CurrentPatrolPoint = ViableDoors[r];
	}
}

//returns true if the guard's current patrol point is inside the room he's in. Otherwise false;
bool AGuardCharacter::isPatrolPointInRoom()
{
	if (GetCurrentRoom() && CurrentPatrolPoint)
	{
		TArray<ADoorActor*> Doors;
		GetCurrentRoom()->GetDoorArray(Doors);

		for (int i = 0; i < Doors.Num(); i++)
		{
			if (Doors[i] == CurrentPatrolPoint)
			{
				return true;
			}
		}
	}
	return false;
}

class ARoomActorBase* AGuardCharacter::GetCurrentRoom()
{
	if (m_GameMode)
	{
		return m_GameMode->GetRoomActorIsIn(this);
	}

	return nullptr;
}

void AGuardCharacter::OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	APlayerCharacter* player = Cast<APlayerCharacter>(OtherActor);

	if (player)
	{
		player->Die();
	}
}

void AGuardCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DetermineGuardState();
	HandleAI();

	//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red, "Guard State: " + EnumToString(TEXT("EAIState"), static_cast<uint8>(GuardState)));
}

//const FString AGuardCharacter::EnumToString(const TCHAR* Enum, int32 EnumValue)
//{
//	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, Enum, true);
//	if (!EnumPtr)
//		return NSLOCTEXT("Invalid", "Invalid", "Invalid").ToString();
//
//#if WITH_EDITOR
//	return EnumPtr->GetDisplayNameTextByIndex(EnumValue).ToString();
//#else
//	return EnumPtr->GetEnumName(EnumValue);
//#endif
//}