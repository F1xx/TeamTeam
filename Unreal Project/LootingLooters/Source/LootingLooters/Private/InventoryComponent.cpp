// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryComponent.h"
#include "BaseTrapActor.h"
#include "StopTrapActor.h"
#include "SlowTrapActor.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	//add new trap types to this list
	Traps.AddUnique(ABaseTrapActor::StaticClass());
	Traps.AddUnique(ASlowTrapActor::StaticClass());
	Traps.AddUnique(AStopTrapActor::StaticClass());

	//set all slots to empty
	Inventory.Init(Traps[0], MaxInventorySlots);
}

// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}


//simple rand check to see if we found a trap on this loot
//chance is between ChanceToFindTrap and 100 (ie ChanceToFindTrap is a direct percentage int)
bool UInventoryComponent::DidFindTrap()
{
	if (ChanceToFindTrap > 100)
		ChanceToFindTrap = 100;
	else if (ChanceToFindTrap < 0)
		ChanceToFindTrap = 0;

	if (FMath::RandRange(0, 100) < ChanceToFindTrap)
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
			int traptype = FMath::RandRange(1, Traps.Num() - 1);

			TrapCount++;
			Inventory[i] = Traps[traptype];
			break; //exit the for loop
		}
	}
}

// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Emerald, FString("Inventory Slot: " + FString::FromInt(SelectedInventorySlot + 1)));

	FString name = Inventory[SelectedInventorySlot]->GetName();
	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Emerald, FString("Equipped in slot: " + name));
	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Emerald, FString("Trap Count: " + FString::FromInt(TrapCount)));
}

//increments score for grabbing the loot and does a random check which if passed gives a Trap as well
int UInventoryComponent::CollectLoot()
{
	if (DidFindTrap() && HasOpenSlot())
	{
		AddRandomTrap();
	}

	//return a score for the loot
	return FMath::RandRange(20, 60);
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
	if (Inventory[SelectedInventorySlot]->GetName() != "Nothing" && TrapCount > 0)
	{
		//Spawn the trap in-world
		FRotator Rotation(0.0f, 0.0f, 0.0f);
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		//AActor* trap = GetWorld()->SpawnActor(test, &location, &Rotation, SpawnInfo);
		AActor* trap = GetWorld()->SpawnActor(Inventory[SelectedInventorySlot], &location, &Rotation, SpawnInfo);

		//ABaseTrapActor* trap = GetWorld()->SpawnActor<>(location, Rotation, SpawnInfo);
		trap->SetOwner(GetOwner());

		Inventory[SelectedInventorySlot] = Traps[0]; //set it back to empty
		TrapCount--;
	}
}