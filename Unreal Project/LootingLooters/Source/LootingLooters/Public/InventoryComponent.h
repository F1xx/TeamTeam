// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BaseTrapActor.h"
#include "InventoryComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LOOTINGLOOTERS_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	short TrapCount = 0;
	short SelectedInventorySlot = 0;
	short MaxTrapAmount = 6;

	TArray<FString> Traps; //real janky using an array of strings

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//returns the score value from the loot
	//Has a chance to also award a trap if the player has space for one.
	float CollectLoot();

	void NextInventoryItem();

	short GetTrapCount() { return TrapCount; }
	void SetTrapCount(short amount) { TrapCount = amount; }
	void PlaceTrap(FVector location);
	
};
