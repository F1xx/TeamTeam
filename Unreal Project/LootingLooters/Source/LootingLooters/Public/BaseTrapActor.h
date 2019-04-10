// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "BaseTrapActor.generated.h"


UENUM(BlueprintType)		//"BlueprintType" is essential to include
//Our trap type enum. Used in HUD to display trap images.
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

		//How long the debuff lasts if this trap applies one.
		UPROPERTY(EditAnywhere)
			float DebuffLength;

		//Our type of debuff.
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

	//Empty by default. If the trap applys a debuff use this function.
	UFUNCTION()
		virtual void ApplyDebuff() {};
	//Empty by default. If the trap applys a debuff use this function.
	UFUNCTION()
		virtual void RemoveDebuff() {};

	//Timer to control how long a debuff from the trap lasts if it uses a debuff.
	FTimerHandle DebuffTime;
	class ABaseCharacter* m_Target;
	bool bIsTriggered = false;

	//What team this trap belongs on.
	UPROPERTY(Replicated)
		int8 Team;

	//The sound this trap should play when triggered.
	UPROPERTY(Replicated, EditDefaultsOnly)
		class USoundWave* m_Sound;

	//Our actual sound generation method. Invoked by Server_PlaySound.
	UFUNCTION(NetMulticast, Reliable)
		void NetMulticast_PlaySound();

	//Server call to play a sound. This method invokes a multicast so all clients hear when a trap *clicks*.
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_PlaySound();
};
