// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryComponent.h"
#include "LootActor.h"
#include "BaseTrapActor.h"
#include "StopTrapActor.h"
#include "SlowTrapActor.h"

#include "Net/UnrealNetwork.h"
#include "PlayerCharacter.h"
#include "MyPlayerState.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	//set all slots to empty
	Inventory.Init(nullptr, MaxInventorySlots);

	Slots[0] = &Slot1;
	Slots[1] = &Slot2;
	Slots[2] = &Slot3;
	Slots[3] = &Slot4;
	Slots[4] = &Slot5;
	Slots[5] = &Slot6;
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
	if (GetOwningPlayer()->GetPlayerState()->TrapCount < MaxInventorySlots)
		return true;

	return false;
}

//finds an open slot and adds a random type of trap into it
void UInventoryComponent::AddRandomTrap()
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		for (int i = 0; i < MaxInventorySlots; i++)
		{
			if (Inventory[i] == Traps[0]) //ie if the slot is empty
			{
				int traptype = FMath::RandRange(1, Traps.Num() - 1); //grab a random trap type starting in the second trap slot (the first is empty)

				GetOwningPlayer()->GetPlayerState()->TrapCount++;
				Inventory[i] = Traps[traptype];
				*Slots[i] = (uint8)traptype;
				break; //exit the for loop
			}
		}
	}
}

// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UInventoryComponent::ServerCollectLoot_Implementation(AActor* lootedObject)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		CollectLoot(lootedObject);
	}
}

bool UInventoryComponent::ServerCollectLoot_Validate(AActor* lootedObject)
{
	return true;
}

//increments score for grabbing the loot and does a random check which if passed gives a Trap as well
void UInventoryComponent::CollectLoot(AActor* lootedObject)
{
	ALootActor* loot = Cast<ALootActor>(lootedObject);
	if (loot)
	{
		if (!loot->isDead)
		{
			if (DidFindTrap(loot) && HasOpenSlot())
			{
				AddRandomTrap();
			}

			loot->MulticastDie();

			GetOwningPlayer()->GetPlayerState()->AddScore(FMath::RandRange(20, 60));
		}
	}
}

//Cycles through the inventory forward
void UInventoryComponent::NextInventoryItem()
{
	ServerNextInventoryItem();
}

//Cycles through the inventory backward
void UInventoryComponent::PrevInventoryItem()
{
	ServerPrevInventoryItem();
}

void UInventoryComponent::ServerPrevInventoryItem_Implementation()
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		GetOwningPlayer()->GetPlayerState()->SelectedInventorySlot--;

		if (GetOwningPlayer()->GetPlayerState()->SelectedInventorySlot <= 0)
		{
			GetOwningPlayer()->GetPlayerState()->SelectedInventorySlot = MaxInventorySlots - 1;
		}
	}
}

bool UInventoryComponent::ServerPrevInventoryItem_Validate()
{
	return true;
}

void UInventoryComponent::ServerNextInventoryItem_Implementation()
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		GetOwningPlayer()->GetPlayerState()->SelectedInventorySlot++;

		if (GetOwningPlayer()->GetPlayerState()->SelectedInventorySlot >= MaxInventorySlots)
		{
			GetOwningPlayer()->GetPlayerState()->SelectedInventorySlot = 0;
		}
	}
}

bool UInventoryComponent::ServerNextInventoryItem_Validate()
{
	return true;
}

void UInventoryComponent::ServerPlaceTrap_Implementation(FVector location)
{
	if (GetNetMode() == ENetMode::NM_DedicatedServer)
		NetMulticastPlaceTrap(location);

	if (GetNetMode() != ENetMode::NM_DedicatedServer)
		SpawnTrap(location);
}

bool UInventoryComponent::ServerPlaceTrap_Validate(FVector location)
{
	return true;
}

void UInventoryComponent::NetMulticastPlaceTrap_Implementation(FVector location)
{
	if (GetOwner()->Role == ROLE_Authority)
	{
		SpawnTrap(location);
	}
}

//actually spawns the trap in the world at the given location
void UInventoryComponent::PlaceTrap(FVector location)
{
	ServerPlaceTrap(location);
}

void UInventoryComponent::SpawnTrap(FVector location)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		//if we actually have a trap selected
		if (Inventory[GetOwningPlayer()->GetPlayerState()->SelectedInventorySlot] != Traps[0] && GetOwningPlayer()->GetPlayerState()->TrapCount > 0)
		{
			//Spawn the trap in-world
			FRotator Rotation(0.0f, 0.0f, 0.0f);
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AActor* trap = GetWorld()->SpawnActor(Inventory[GetOwningPlayer()->GetPlayerState()->SelectedInventorySlot], &location, &Rotation, SpawnInfo);

			trap->SetOwner(GetOwner());

			Inventory[GetOwningPlayer()->GetPlayerState()->SelectedInventorySlot] = Traps[0]; //set it back to empty
			*Slots[GetOwningPlayer()->GetPlayerState()->SelectedInventorySlot] = 0;
			GetOwningPlayer()->GetPlayerState()->TrapCount--;
		}
	}
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, Inventory);
	DOREPLIFETIME(UInventoryComponent, Slot1);
	DOREPLIFETIME(UInventoryComponent, Slot2);
	DOREPLIFETIME(UInventoryComponent, Slot3);
	DOREPLIFETIME(UInventoryComponent, Slot4);
	DOREPLIFETIME(UInventoryComponent, Slot5);
	DOREPLIFETIME(UInventoryComponent, Slot6);
	DOREPLIFETIME(UInventoryComponent, MaxInventorySlots);
}