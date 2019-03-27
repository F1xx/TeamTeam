// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryComponent.h"
#include "LootActor.h"
#include "BaseTrapActor.h"
#include "StopTrapActor.h"
#include "SlowTrapActor.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	//Set the default inventory slot to be empty
	//Extra traps are added through this component in the blueprint
	Traps.Add(nullptr);

	//set all slots to empty
	Inventory.Init(Traps[0], MaxInventorySlots);

	Slot1 = 0;
	Slot2 = 0;
	Slot3 = 0;
	Slot4 = 0;
	Slot5 = 0;
	Slot6 = 0;

	Slots[0] = &Slot1;
	Slots[1] = &Slot2;
	Slots[2] = &Slot3;
	Slots[3] = &Slot4;
	Slots[4] = &Slot5;
	Slots[5] = &Slot6;
}

// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}


//simple rand check to see if we found a trap on this loot
//chance is between ChanceToFindTrap and 100 (ie ChanceToFindTrap is a direct percentage int)
bool UInventoryComponent::DidFindTrap(class ALootActor* loot)
{
	//between 1 and 100 so that if the chance is 0 it will never spawn
	//also if chance is 100 then no matter what this is true
	if (FMath::RandRange(1, 100) <= loot->ChanceToFindTrap)
		return true;

	return false;
}

//checks if there are any inventory slots without a trap already in them
bool UInventoryComponent::HasOpenSlot()
{
	if (TrapCount < MaxInventorySlots)
		return true;

	return false;
}

//finds an open slot and adds a random type of trap into it
void UInventoryComponent::AddRandomTrap()
{
	for (int i = 0; i < MaxInventorySlots; i++)
	{
		if (Inventory[i] == Traps[0]) //ie if the slot is empty
		{
			int traptype = FMath::RandRange(1, Traps.Num() - 1); //grab a random trap type starting in the second trap slot (the first is empty)

			TrapCount++;
			Inventory[i] = Traps[traptype];
			*Slots[i] = (unsigned char)traptype;
			break; //exit the for loop
		}
	}
}

// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//Makeshift HUD
	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Emerald, FString("Inventory Slot: " + FString::FromInt(SelectedInventorySlot + 1)));

	if (Inventory[SelectedInventorySlot])
	{
		FString name = Inventory[SelectedInventorySlot]->GetName();
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Emerald, FString("Equipped in slot: " + name));
	}
	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Emerald, FString("Trap Count: " + FString::FromInt(TrapCount)));
}

//increments score for grabbing the loot and does a random check which if passed gives a Trap as well
int UInventoryComponent::CollectLoot(AActor* lootedObject)
{
	ALootActor* loot = Cast<ALootActor>(lootedObject);

	if (loot)
	{
		if (DidFindTrap(loot) && HasOpenSlot())
		{
			AddRandomTrap();
		}
		//return a score for the loot

		loot->Die();
		return FMath::RandRange(20, 60);
	}

	//it wasn't loot, return nothing
	return 0;
}

//Cycles through the inventory forward
void UInventoryComponent::NextInventoryItem()
{
	SelectedInventorySlot++;

	if (SelectedInventorySlot >= MaxInventorySlots)
	{
		SelectedInventorySlot = 0;
	}
}

//Cycles through the inventory backward
void UInventoryComponent::PrevInventoryItem()
{
	SelectedInventorySlot--;

	if (SelectedInventorySlot <= 0)
	{
		SelectedInventorySlot = MaxInventorySlots - 1;
	}
}

//actually spawns the trap in the world at the given location
void UInventoryComponent::PlaceTrap(FVector location)
{
	//if we actually have a trap selected
	if (Inventory[SelectedInventorySlot] != Traps[0] && TrapCount > 0)
	{
		//Spawn the trap in-world
		FRotator Rotation(0.0f, 0.0f, 0.0f);
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AActor* trap = GetWorld()->SpawnActor(Inventory[SelectedInventorySlot], &location, &Rotation, SpawnInfo);

		trap->SetOwner(GetOwner());

		Inventory[SelectedInventorySlot] = Traps[0]; //set it back to empty
		*Slots[SelectedInventorySlot] = 0;
		TrapCount--;
	}
}