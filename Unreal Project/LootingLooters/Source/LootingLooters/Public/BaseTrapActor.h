// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "BaseTrapActor.generated.h"

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum  EDebuffs
{
	DE_Nothing 	UMETA(DisplayName = "Nothing"),
	DE_Stop 	UMETA(DisplayName = "Stop"),
	DE_Slow 	UMETA(DisplayName = "Slow"),
};

/**
 * 
 */
UCLASS(BlueprintType)
class LOOTINGLOOTERS_API ABaseTrapActor : public AStaticMeshActor
{
	GENERATED_BODY()

public:
		ABaseTrapActor();

		virtual void Tick(float DeltaSeconds) override;

		UFUNCTION()
			virtual void HandleOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

		UPROPERTY(EditAnywhere)
			float DebuffLength;

		UPROPERTY(VisibleAnywhere)
			TEnumAsByte<enum EDebuffs> TrapDebuff;

		//The mesh for when the trap is activated
		UPROPERTY(EditAnywhere)
			UStaticMesh* ActivatedMesh;

        UFUNCTION()
            TEnumAsByte<enum EDebuffs> GetType() { return TrapDebuff; }
protected:
	virtual void Die();
	virtual void SetTarget(class ABaseCharacter* character);

	UFUNCTION()
		virtual void ApplyDebuff() {};
	UFUNCTION()
		virtual void RemoveDebuff() {};

	FTimerHandle DebuffTime;
	class ABaseCharacter* m_Target;
	bool bIsTriggered = false;

	UPROPERTY(Replicated)
		int8 Team;

	UPROPERTY(Replicated, EditDefaultsOnly)
		class USoundWave* m_Sound;

	UFUNCTION(NetMulticast, Reliable)
		void NetMulticast_PlaySound();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_PlaySound();
};
