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

	//Copy of our team value from PlayerCharacter.
	UPROPERTY(Replicated, BlueprintReadOnly, Category = Teams)
		uint8 Team;

	AMyPlayerState();

	//Adds the value to the score variable. Cannot pass negatives.
	UFUNCTION(BlueprintCallable)
		void AddScore(int value);

	//The total amount of traps we have.
	UPROPERTY(Replicated, BlueprintReadOnly, Category = Inventory)
		uint8 TrapCount = 0;

	//Which inventory slot is currently hovered by the player.
	UPROPERTY(Replicated, BlueprintReadOnly, Category = Inventory)
		uint8 SelectedInventorySlot = 0;

	//Fetcher to get slot data for the hud.
	UFUNCTION(BlueprintCallable)
		int GetSlot(int index);

	//Our inventory slot data.
	UPROPERTY(Replicated)
		uint8 Slots[6];

	//Reset function to flush all inventory data on player respawn.
	UFUNCTION()
		void Reset();
};
