// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "PlayerCharacter.generated.h"

/**
 * 
 */
UCLASS()
class LOOTINGLOOTERS_API APlayerCharacter : public ABaseCharacter
{
	GENERATED_BODY()
	
public:
	APlayerCharacter();
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Camera
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* Camera;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
		class UInventoryComponent* m_Inventory;

public:
	UFUNCTION()
		class AMyPlayerState* GetPlayerState();

	UFUNCTION()
		class ALootingLootersGameStateBase* GetGameState();

	UFUNCTION(BlueprintCallable)
		UInventoryComponent* GetInventoryComponent() { return m_Inventory; }

	virtual void Die() override;

	virtual void Interact() override;
	virtual void PlaceTrap() override;

	virtual void NextInventory() override;

	// Called every frame
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void RotateHeldObjectX(float Value) override;
	virtual void RotateHeldObjectY(float Value) override;

	void AssignTeam();

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_AssignColor();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
		void NetMulticast_SetColor();

	UFUNCTION(BlueprintCallable)
		void PostBeginPlay();

	UPROPERTY(Replicated)
		FTimerHandle PostBeginPlayDelay;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Materials, Replicated)
		class UMaterialInterface* DefaultMaterial;

	UFUNCTION(BlueprintCallable, Category = "Camera")
		FORCEINLINE class UCameraComponent* GetFollowCamera() const { return Camera; }

	UPROPERTY(Replicated)
		uint8 Team;
};
