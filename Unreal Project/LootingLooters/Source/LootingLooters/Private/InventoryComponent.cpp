// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryComponent.h"
#include "BaseTrapActor.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	for (int x = 0; x < MaxTrapAmount; x++)
	{
		Traps.Add("");
	}
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Emerald, FString("Inventory Slot: " + FString::FromInt(SelectedInventorySlot + 1)));

	FString name;
	if (Traps[SelectedInventorySlot] == "")
		name = "Nothing";
	else
		name = Traps[SelectedInventorySlot];
	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Emerald, FString("Equipped in slot: " + name));
	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Emerald, FString("Trap Count: " + FString::FromInt(TrapCount)));
}

float UInventoryComponent::CollectLoot()
{
	//guaranteed for now
	int result = FMath::RandRange(0, 20);

	//if we rolled lucky and have space set the first available spot to be a trap
	if (result < 20)
	{
		for (int i = 0; i < MaxTrapAmount; i++)
		{
			if (Traps[i] == "")
			{
				TrapCount++;
				Traps[i] = "Basic Trap";
				break; //exit the for loop
			}
		}
	}

	//no matter what return a score for the loot
	return (float)FMath::RandRange(20, 60);
}

void UInventoryComponent::NextInventoryItem()
{
	SelectedInventorySlot++;

	if (SelectedInventorySlot >= Traps.Num())
	{
		SelectedInventorySlot = 0;
	}
}

//actually spawns the trap in the world at the given location
void UInventoryComponent::PlaceTrap(FVector location)
{
	//if we actually have a trap selected
	if (Traps[SelectedInventorySlot] != "" && TrapCount > 0)
	{
		TrapCount--;

		//Spawn the trap in-world
		FRotator Rotation(0.0f, 0.0f, 0.0f);
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ABaseTrapActor* trap = GetWorld()->SpawnActor<ABaseTrapActor>(location, Rotation, SpawnInfo);
		trap->SetOwner(GetOwner());

		Traps[SelectedInventorySlot] = "";
	}
}