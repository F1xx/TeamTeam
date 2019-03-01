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

	UFUNCTION()
		void DetermineGuardState();

	UFUNCTION()
		void SetGuardState(EAIState NewState); 

	UFUNCTION()
		void HandleAI();

	AActor* CurrentPatrolPoint;

	UFUNCTION()
		void MoveToNextPatrolPoint();

	UFUNCTION()
		void FindNewPatrolPoint();

	UFUNCTION()
		bool isPatrolPointInRoom();

	UFUNCTION()
		class ARoomActorBase* GetCurrentRoom();

	UFUNCTION()
		virtual void OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);


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

	bool bSearchedLastPlayerLocation;
	AActor* LastDoorPlayerUsed;

	class ALootingLootersGameModeBase* m_GameMode;

	const FString EnumToString(const TCHAR* Enum, int32 EnumValue);
};
