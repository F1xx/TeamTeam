// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LOOTINGLOOTERS_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

	//functions
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	bool DidFindTrap(class ALootActor* loot);
	bool HasOpenSlot();
	void AddRandomTrap();

	//variables
protected:
	short TrapCount = 0;
	short SelectedInventorySlot = 0;

	const short MaxInventorySlots = 6;

	//Holds a list of all trap types
	//The first element is the base class and counts as empty
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<class ABaseTrapActor>> Traps;

	//The player's inventory
	//Holds all traps they have up to MaxInventory Slots
	UPROPERTY(VisibleAnywhere)
		TArray<TSubclassOf<class ABaseTrapActor>> Inventory;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//returns the score value from the loot
	//Has a chance to also award a trap if the player has space for one.
	int CollectLoot(AActor* lootedObject);

	void NextInventoryItem();
	void PrevInventoryItem();

	short GetTrapCount() { return TrapCount; }
	void PlaceTrap(FVector location);
	
};
