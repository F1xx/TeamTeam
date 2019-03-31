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

	AMyPlayerState();

	UFUNCTION(BlueprintCallable)
		void AddScore(int value) { Score += value; }

	UPROPERTY(Replicated, BlueprintReadOnly, Category = Inventory)
		uint8 TrapCount = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = Inventory)
		uint8 SelectedInventorySlot = 0;	
};
