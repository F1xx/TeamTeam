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

	UFUNCTION()
		void OnPawnSeen(APawn* SeenPawn);

	FRotator OriginalRotator;

	UFUNCTION()
		void ResetState();

	UFUNCTION()
		void ResetPatrol();

	FTimerHandle TimerHandle_ResetState;

	EAIState GuardState;

	void SetGuardState(EAIState NewState); 

	UFUNCTION(BlueprintImplementableEvent, Category = "AI")
		void OnStateChanged(EAIState NewState); 

	UPROPERTY(EditAnywhere, Category = "AI")
		AActor* FirstPatrolPoint;

	UPROPERTY(EditAnywhere, Category = "AI")
		AActor* SecondPatrolPoint;

	AActor* CurrentPatrolPoint;

	UFUNCTION()
		void MoveToNextPatrolPoint();

public:
	virtual void Tick(float DeltaTime) override;

	//The Guard's walking speed while patrolling
	UPROPERTY(EditAnywhere)
		float PatrolSpeed = 300.0f;

	//The Guard's walking speed while chasing players
	UPROPERTY(EditAnywhere)
		float AttackSpeed = 500.0f;

private:
	AActor* TargetActor;
	FVector SearchLocation;
};
