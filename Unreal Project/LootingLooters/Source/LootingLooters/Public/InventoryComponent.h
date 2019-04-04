// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerCharacter.h"
#include "InventoryComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType)
class LOOTINGLOOTERS_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

	class APlayerCharacter* GetOwningPlayer() { return Cast<APlayerCharacter>(GetOwner()); }
protected:

	virtual void BeginPlay() override;
	bool DidFindTrap(class ALootActor* loot);
	bool HasOpenSlot();
	void AddRandomTrap();
public:

	UPROPERTY(VisibleAnywhere, Category = "Inventory", Replicated)
		/*const*/ uint8 MaxInventorySlots = 6;

	//Holds a list of all trap types
	//The first element is the base class and counts as empty
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<class ABaseTrapActor>> Traps;

	//The player's inventory
	//Holds all traps they have up to MaxInventory Slots
	UPROPERTY(VisibleAnywhere, Replicated)
		TArray<TSubclassOf<class ABaseTrapActor>> Inventory;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//Has a chance to also award a trap if the player has space for one.
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerCollectLoot(AActor* lootedObject);

	void NextInventoryItem();
	void PrevInventoryItem();

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerNextInventoryItem();

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerPrevInventoryItem();

	UFUNCTION(BlueprintCallable)
		uint8 GetMaxInventorySlots() { return MaxInventorySlots; }

	UFUNCTION(BlueprintCallable)
		TArray<TSubclassOf<class ABaseTrapActor>> GetInventoryArray() { return Inventory; }

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerPlaceTrap(FVector location);

	UFUNCTION(NetMulticast, Reliable)
		void NetMulticastPlaceTrap(FVector location);

	void CollectLoot(AActor* lootedObject);
	void PlaceTrap(FVector location);
	void SpawnTrap(FVector location);
};