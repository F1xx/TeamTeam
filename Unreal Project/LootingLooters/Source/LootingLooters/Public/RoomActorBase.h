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

	//Iterates through all sockets that the room mesh has and fills them with assets.
	UFUNCTION()
		void PopulateEmptySockets();

	//Iterates through all sockets that the room assets contain and fill them with loot.
	UFUNCTION()
		void GenerateRandomLoot(unsigned int loot_count);

	//Iterate through all doors without connections and attempt to connect them to other doors without connections.
	UFUNCTION()
		void GenerateDoorConnections();

	//Getter for door Tarray
	void GetDoorArray(TArray<class ADoorActor*> &Output);

	//Overlap component for finding actors in rooms
	UPROPERTY(VisibleInstanceOnly, Category = "Mesh")
		class UBoxComponent* RoomOverlap;

protected:
	//Fetches the total number of doors lacking a connection.
	int GetNumberOfDoorsWithoutAConnection();

	//Fetches a random unconnected door if there is one.
	ADoorActor* GetARandomUnconnectedDoor();

	//Get a list of all unconnected doors if there are any.
	void GetOrganizedDoors(TArray<ADoorActor*> &Output, bool connected = true);

	//All doors spawned by the room
	UPROPERTY(Replicated, VisibleInstanceOnly, Category = "Doors")
		TArray<ADoorActor*> GeneratedDoors;

	//The room mesh
	UPROPERTY(Replicated, VisibleInstanceOnly, Category = "Mesh")
		AStaticMeshActor* RoomMesh;

	//All Assets spawned by the room
	UPROPERTY(VisibleInstanceOnly, Category = "Assets")
		TArray<class AAssetTemplate*> RoomAssets;
};