// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoomActorBase.generated.h"

/**
 * 
 */
UCLASS()
class LOOTINGLOOTERS_API ARoomActorBase : public AActor
{
	GENERATED_BODY()
	
	
public:

	ARoomActorBase();

	void SetRoomMesh(class AStaticMeshActor* Mesh);

	UFUNCTION()
		void PopulateEmptySockets();

	UFUNCTION()
		void GenerateRandomLoot(unsigned int loot_count);

	UFUNCTION()
		void GenerateDoorConnections();

	void GetDoorArray(TArray<class ADoorActor*> &Output);

	UPROPERTY(VisibleInstanceOnly, Category = "Mesh")
		class UBoxComponent* RoomOverlap;

protected:
	//Fetches the total number of doors lacking a connection.
	int GetNumberOfDoorsWithoutAConnection();

	//Fetches a random unconnected door if there is one.
	ADoorActor* GetARandomUnconnectedDoor();

	//Get a list of all unconnected doors if there are any.
	void GetOrganizedDoors(TArray<ADoorActor*> &Output, bool connected = true);

	UPROPERTY(Replicated, VisibleInstanceOnly, Category = "Doors")
		TArray<ADoorActor*> GeneratedDoors;

	UPROPERTY(Replicated, VisibleInstanceOnly, Category = "Mesh")
		AStaticMeshActor* RoomMesh;

	UPROPERTY(VisibleInstanceOnly, Category = "Assets")
		TArray<class AAssetTemplate*> RoomAssets;
};