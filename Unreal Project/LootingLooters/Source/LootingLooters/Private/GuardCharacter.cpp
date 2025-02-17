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
#include "Components/SphereComponent.h"

AGuardCharacter::AGuardCharacter() : Super()
{
	PrimaryActorTick.bCanEverTick = true;

	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>("Sensor");
	PawnSensingComp->OnSeePawn.AddDynamic(this, &AGuardCharacter::OnPawnSeen);

	GuardState = EAIState::EPatrol;
	SetMaxSpeed(PatrolSpeed);

	SearchLocation = FVector::ZeroVector;
	TargetActor = nullptr;
	CurrentPatrolPoint = nullptr;

	Tags.Add("Guard");
}

void AGuardCharacter::BeginPlay()
{
	Super::BeginPlay();

	//Set our collision component and set the delay timer.
	if (HasAuthority())
	{
		SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AGuardCharacter::OnPawnHit);
		GetWorldTimerManager().SetTimer(PostStart, this, &AGuardCharacter::PostBeginPlay, 3.0f, false);

		m_GameMode = Cast<ALootingLootersGameModeBase>(GetWorld()->GetAuthGameMode());
	}
}

//if we see a player, attack them
void AGuardCharacter::OnPawnSeen(APawn * SeenPawn)
{
	if (SeenPawn == nullptr)
		return;

	APlayerCharacter* player = Cast<APlayerCharacter>(SeenPawn);

	//we dont chase dead targets.
	if (player)
		if (player->bIsDead)
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

		//we switched target so tell the former character we are no longer pursuing them
		APlayerCharacter* pc = Cast<APlayerCharacter>(TargetActor);
		pc->Client_BeingChased(false);
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
	//Notify the player they are no longer being chased.
	if (TargetActor)
	{
		APlayerCharacter* player = Cast<APlayerCharacter>(TargetActor);
		if (player)
		{
			player->Server_BeingChased(false);
		}
	}
	
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
		//Notify the player theyre being chased.
		APlayerCharacter* player = Cast<APlayerCharacter>(TargetActor);
		if (player)
		{
			player->Server_BeingChased(true);
		}
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

		if (CurrentPatrolPoint == nullptr)
			MoveToNextPatrolPoint();

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
	if (HasAuthority())
	{
		FindNewPatrolPoint();
		UAIBlueprintHelperLibrary::SimpleMoveToActor(GetController(), CurrentPatrolPoint);
	}
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

void AGuardCharacter::OnPawnHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	APlayerCharacter* player = Cast<APlayerCharacter>(OtherActor);

	//if we hit the player kill them and reset our guard state back to patrol.
	if (player)
	{
		player->Die();
		ResetPatrol();
	}
}

void AGuardCharacter::PostBeginPlay()
{
	bHasBegun = true;

	OriginalRotator = GetActorRotation();
	ResetState();
}

void AGuardCharacter::Tick(float DeltaTime)
{
	if (bHasBegun)
	{
		Super::Tick(DeltaTime);

		//Every tick we determine our new guard state and what we're doing.
		if (HasAuthority())
		{
			DetermineGuardState();
			HandleAI();
		}
	}
}