// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "GuardCharacter.generated.h"

UENUM(BlueprintType)
enum class EAIState : uint8
{
	EPatrol,
	ESearch,
	EAttack
};

/**
 * 
 */
UCLASS()
class LOOTINGLOOTERS_API AGuardCharacter : public ABaseCharacter
{
	GENERATED_BODY()
	
public:
	AGuardCharacter();
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
		class UPawnSensingComponent* PawnSensingComp;

	//When we see a pawn we usually switch to Attack mode.
	UFUNCTION()
		void OnPawnSeen(APawn* SeenPawn);

	FRotator OriginalRotator;

	//Reset our state values.
	UFUNCTION()
		void ResetState();

	//Return back to patrol and get a new patrol point.
	UFUNCTION()
		void ResetPatrol();

	//Reset handle for when the guard gives up searching for a player
	FTimerHandle TimerHandle_ResetState;

	//Our guards state.
	UPROPERTY(VisibleAnywhere, Category = "Guard State")
	EAIState GuardState;

	//Run each frame to determine how the guard should react based on the situation.
	UFUNCTION()
		void DetermineGuardState();

	//Helper for setting the guard state.
	UFUNCTION()
		void SetGuardState(EAIState NewState); 

	//Our actual AI logic is here :
	//PATROL - Go to the next patrol point, if we're already there get a new one.
	//SEARCH - Check the last known players location and door they took.
	//ATTACK - Pursue the player.
	UFUNCTION()
		void HandleAI();

	//Our patrol point we path to during Patrol state.
	UPROPERTY(VisibleAnywhere, Category = "Patrol")
	AActor* CurrentPatrolPoint;

	//Move command to the next patrol point using simplemovetoactor
	UFUNCTION()
		void MoveToNextPatrolPoint();

	//Helper to set the new PatrolPoint
	UFUNCTION()
		void FindNewPatrolPoint();

	//Helper to determine if the guard somehow got lost?
	UFUNCTION()
		bool isPatrolPointInRoom();
	
	//What room are we in? Useful for finding patrol nodes.
	UFUNCTION()
		class ARoomActorBase* GetCurrentRoom();

	//Our collision for hitting other pawns. We kill them.
	UFUNCTION()
		virtual void OnPawnHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	//Our delay for setting sensitive data after BeginPlay. Once the guard is started he will begin moving.
	UFUNCTION()
		virtual void PostBeginPlay();

	//PostBeginPlay timer handle. Delays the guard from immediately acting upon level load.
	FTimerHandle PostStart;

	//Control bool for determining if the guard has begun acting.
	UPROPERTY(VisibleAnywhere, Category = "Active")
	bool bHasBegun = false;

public:
	virtual void Tick(float DeltaTime) override;

	//The Guard's walking speed while patrolling
	UPROPERTY(EditAnywhere)
		float PatrolSpeed = 300.0f;

	//The Guard's walking speed while chasing players
	UPROPERTY(EditAnywhere)
		float AttackSpeed = 500.0f;

private:

	UPROPERTY(VisibleAnywhere, Category = "Player")
		AActor* TargetActor;
	UPROPERTY(VisibleAnywhere, Category = "Player")
		FVector SearchLocation;
	UPROPERTY(VisibleAnywhere, Category = "Player")
		bool bSearchedLastPlayerLocation;
	UPROPERTY(VisibleAnywhere, Category = "Player")
		AActor* LastDoorPlayerUsed;

	class ALootingLootersGameModeBase* m_GameMode;
};
