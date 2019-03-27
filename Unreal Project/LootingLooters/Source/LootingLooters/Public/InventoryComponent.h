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
	UPROPERTY(Replicated)
		int TrapCount = 0;

	UPROPERTY(Replicated)
		int SelectedInventorySlot = 0;

	UPROPERTY(VisibleAnywhere, Category = "Inventory")
		/*const*/ int MaxInventorySlots = 6;

	//Holds a list of all trap types
	//The first element is the base class and counts as empty
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<class ABaseTrapActor>> Traps;

	//The player's inventory
	//Holds all traps they have up to MaxInventory Slots
	UPROPERTY(VisibleAnywhere, Replicated)
		TArray<TSubclassOf<class ABaseTrapActor>> Inventory;

	UPROPERTY(VisibleAnywhere)
		unsigned char Slot1;

	UPROPERTY(VisibleAnywhere)
		unsigned char Slot2;

	UPROPERTY(VisibleAnywhere)
		unsigned char Slot3;

	UPROPERTY(VisibleAnywhere)
		unsigned char Slot4;

	UPROPERTY(VisibleAnywhere)
		unsigned char Slot5;

	UPROPERTY(VisibleAnywhere)
		unsigned char Slot6;

	unsigned char* Slots[6];

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//Has a chance to also award a trap if the player has space for one.
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerCollectLoot(AActor* lootedObject);

	void CollectLoot(AActor* lootedObject);

	void NextInventoryItem();
	void PrevInventoryItem();

	short GetTrapCount() { return TrapCount; }

	UFUNCTION(BlueprintCallable)
		TArray<TSubclassOf<class ABaseTrapActor>> GetInventory() { return Inventory; }

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerPlaceTrap(FVector location);

	UFUNCTION(NetMulticast, Reliable)
		void NetMulticastPlaceTrap(FVector location);

		void PlaceTrap(FVector location);
		void SpawnTrap(FVector location);
	
};