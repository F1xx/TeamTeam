// Fill out your copyright notice in the Description page of Project Settings.

#include "GuardCharacter.h"
#include "NavigationSystem.h"
#include "AIModule/Classes/Blueprint/AIBlueprintHelperLibrary.h"
#include "Perception/PawnSensingComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

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

	OriginalRotator = GetActorRotation();
	MoveToNextPatrolPoint();
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

	GetWorldTimerManager().ClearTimer(TimerHandle_ResetState);
	SetActorRotation(OriginalRotator);
	ResetPatrol();
}

void AGuardCharacter::ResetPatrol()
{
	TargetActor = nullptr;
	SetGuardState(EAIState::EPatrol);

	MoveToNextPatrolPoint();
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

		if (TargetActor)
		{
			SearchLocation = TargetActor->GetActorLocation();
		}

		break;
	case EAIState::EAttack:
		SetMaxSpeed(AttackSpeed);
		break;
	}
}

//use this function to find the door in the room that the guard will go to
void AGuardCharacter::MoveToNextPatrolPoint()
{
	if (CurrentPatrolPoint == nullptr || CurrentPatrolPoint == SecondPatrolPoint)
	{
		CurrentPatrolPoint = FirstPatrolPoint;
	}
	else
	{
		CurrentPatrolPoint = SecondPatrolPoint;
	}

	UAIBlueprintHelperLibrary::SimpleMoveToActor(GetController(), CurrentPatrolPoint);
}

void AGuardCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//ensure the right state is set
	//if we are in a chase we'll either attack or search
	if (TargetActor)
	{
		//if we can still see our target we are attacking
		if (PawnSensingComp->HasLineOfSightTo(TargetActor))
			SetGuardState(EAIState::EAttack);
		//if we can't see our target anymore go to where they last were
		else
			SetGuardState(EAIState::ESearch);
	}
	else
		SetGuardState(EAIState::EPatrol);


	//this switch actually handles the guard's states
	switch (GuardState)
	{
	case EAIState::EPatrol: //no target, patrol

		if (CurrentPatrolPoint)
		{
			FVector Delta = (CurrentPatrolPoint->GetActorLocation() - GetActorLocation());
			float DistanceToGoal = Delta.Size();

			if (DistanceToGoal < 150)
			{
				MoveToNextPatrolPoint();
			}
		}
		break;

	case EAIState::ESearch://lost them so go to their last position and look around

		UAIBlueprintHelperLibrary::SimpleMoveToLocation(GetController(), SearchLocation);

		//Make the guard give up after 3 seconds of inactivity
		//only starts if they are close or at the target's last known position
		if (Controller != nullptr && FVector::Dist(GetActorLocation(), SearchLocation) <= 50 && !GetWorldTimerManager().IsTimerActive(TimerHandle_ResetState))
		{
			Controller->StopMovement();
			GetWorldTimerManager().SetTimer(TimerHandle_ResetState, this, &AGuardCharacter::ResetState, 3.0f, true);
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

	//outputs debug so we know the search info
	/*
	if (GuardState == EAIState::ESearch)
	{
		float x = TargetActor->GetActorLocation().X;
		float y = TargetActor->GetActorLocation().Y;
		float z = TargetActor->GetActorLocation().Z;

		FString loc = FString::SanitizeFloat(x) + ", " + FString::SanitizeFloat(y) + ", " + FString::SanitizeFloat(z);

		GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Black, "TargetActor Location: " + loc);

		 x = SearchLocation.X;
		 y = SearchLocation.Y;
		 z = SearchLocation.Z;

		 loc = FString::SanitizeFloat(x) + ", " + FString::SanitizeFloat(y) + ", " + FString::SanitizeFloat(z);

		GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red, "Search Location: " + loc);
	}
	*/
}