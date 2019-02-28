// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "AssetTemplate.generated.h"

/**
 * 
 */
UCLASS()
class LOOTINGLOOTERS_API AAssetTemplate : public AStaticMeshActor
{
	GENERATED_BODY()

	AAssetTemplate();

public:
	
	//Asset iterates through all sockets and spawns LootActors on every "loot" socket
	void PopulateLootSockets();

	//All loot spawned by the Asset
	UPROPERTY(VisibleInstanceOnly, Category = "Loot")
		TArray<class ALootActor*> Loot;

};
