// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MyPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class LOOTINGLOOTERS_API AMyPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:

	UPROPERTY(Replicated, BlueprintReadOnly, Category = Teams)
		uint8 Team;

	AMyPlayerState();

	//Adds the value to the score variable. Cannot pass negatives.
	UFUNCTION(BlueprintCallable)
		void AddScore(int value);

	UPROPERTY(Replicated, BlueprintReadOnly, Category = Inventory)
		uint8 TrapCount = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = Inventory)
		uint8 SelectedInventorySlot = 0;

	UFUNCTION(BlueprintCallable)
		int GetSlot(int index);

	UPROPERTY(Replicated)
		uint8 Slots[6];

	UFUNCTION()
		void Reset();
};
