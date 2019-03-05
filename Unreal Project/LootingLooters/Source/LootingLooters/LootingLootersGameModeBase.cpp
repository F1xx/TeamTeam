// Fill out your copyright notice in the Description page of Project Settings.

#include "LootingLootersGameModeBase.h"
#include "EngineUtils.h" //Needed for TActorIterator
#include "Kismet/GameplayStatics.h"
#include "RoomActorBase.h"
#include "Net/UnrealNetwork.h"
#include "Engine/StaticMeshActor.h"
#include "Components/BoxComponent.h"
#include "FileManager.h"
#include "Engine/StaticMesh.h"
#include "AssetTemplate.h"

ALootingLootersGameModeBase::ALootingLootersGameModeBase() : Super()
 {
	//uasset wildcard
 	FString wildcard("*.uasset");
 
 	//find all blueprints and load into memory
	{
		FString AssetDirectory(FPaths::Combine(*FPaths::ProjectDir(), TEXT("Content/Blueprints/BlueprintedAssets/Static/"), *wildcard));
		FString ConstructorPath("/Game/Blueprints/BlueprintedAssets/Static/");
		LoadAssetArray<AActor>(Game_Assets, AssetDirectory, ConstructorPath);

		FString GrabbableAssetDirectory(FPaths::Combine(*FPaths::ProjectDir(), TEXT("Content/Blueprints/BlueprintedAssets/Grabbable"), *wildcard));
		ConstructorPath = "/Game/Blueprints/BlueprintedAssets/Grabbable/";
		LoadAssetArray<AActor>(Game_Assets, GrabbableAssetDirectory, ConstructorPath);

		FString RoomDirectory(FPaths::Combine(*FPaths::ProjectDir(), TEXT("Content/Blueprints/Test_Rooms"), *wildcard));
		ConstructorPath = "/Game/Blueprints/Test_Rooms/";
		LoadAssetArray<AStaticMeshActor>(Room_Assets, RoomDirectory, ConstructorPath);

		FString LootDirectory(FPaths::Combine(*FPaths::ProjectDir(), TEXT("Content/Blueprints/Loot/"), *wildcard));
		ConstructorPath = "/Game/Blueprints/Loot/";
		LoadAssetArray<ALootActor>(Loot_Assets, LootDirectory, ConstructorPath);

		FString DoorDirectory(FPaths::Combine(*FPaths::ProjectDir(), TEXT("Content/Blueprints/Doors/"), *wildcard));
		ConstructorPath = "/Game/Blueprints/Doors/";
		LoadAssetArray<AStaticMeshActor>(Door_Assets, DoorDirectory, ConstructorPath);
	}
}

void ALootingLootersGameModeBase::StartPlay()
{
	Super::StartPlay();
	FString command = "show collision";
	APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (controller)
	{
		controller->ConsoleCommand(command, false);
		//controller->bShowMouseCursor = true;
		//controller->bEnableClickEvents = true;
		//controller->bEnableMouseOverEvents = true;
	}

	GenerateRandomRoomLayout();
	PopulateRoomSockets();
	GenerateRandomRoomConnections();
	GenerateLoot();
}

void ALootingLootersGameModeBase::GenerateRandomRoomLayout_Implementation()
{
	check(Room_Assets.Num() > 0);

	UWorld* world = GetWorld();
	int room_root = FMath::Sqrt(Total_Rooms_To_Generate);

	if (world)
	{
		for (int i = 0; i < Total_Rooms_To_Generate; i++)
		{
			//pull a random room 
			int random_index = FMath::RandRange(0, Room_Assets.Num() - 1);

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			//Spacing so the rooms aren't on top of one another.
			FVector SpawnOffset = FVector(2000.0f * (i / room_root), 2000.0f * (i % room_root), -500.0f);
			FRotator SpawnRotation = FRotator(0.0f, 0.0f, 0.0f);

			//Spawn the room and the Mesh for the room and attach them together.
			ARoomActorBase* AddedRoom = world->SpawnActor<ARoomActorBase>(ARoomActorBase::StaticClass(), SpawnOffset, SpawnRotation, SpawnParams);
			AStaticMeshActor* AddedRoomMesh = world->SpawnActor<AStaticMeshActor>(Room_Assets[random_index], SpawnOffset, SpawnRotation, SpawnParams);
			AddedRoom->SetRoomMesh(AddedRoomMesh);
			Rooms.Add(AddedRoom);
		}
	}
}

bool ALootingLootersGameModeBase::GenerateRandomRoomLayout_Validate()
{
	return true;
}

void ALootingLootersGameModeBase::GenerateRandomRoomConnections_Implementation()
{
	//iterate through all rooms and have them connect to each other
	for (int i = 0; i < Rooms.Num(); i++)
		Rooms[i]->GenerateDoorConnections();
}

bool ALootingLootersGameModeBase::GenerateRandomRoomConnections_Validate()
{
	return true;
}

void ALootingLootersGameModeBase::PopulateRoomSockets_Implementation()
{
	//iterate through all rooms and have them generate assets to fill them
	for (int i = 0; i < Rooms.Num(); i++)
		Rooms[i]->PopulateEmptySockets();
}

bool ALootingLootersGameModeBase::PopulateRoomSockets_Validate()
{
	return true;
}

void ALootingLootersGameModeBase::GenerateLoot_Implementation()
{
	//iterate through all rooms and have them generate loot on their assets
	for (int i = 0; i < Rooms.Num(); i++)
		Rooms[i]->GenerateRandomLoot(-1);
}

bool ALootingLootersGameModeBase::GenerateLoot_Validate()
{
	return true;
}

void ALootingLootersGameModeBase::GetRoomArray(TArray<ARoomActorBase*>& RoomArray)
{
	//copy supplied TArray
	RoomArray = Rooms;
}

//returns nullptr if the actor can't be found in any room
ARoomActorBase* ALootingLootersGameModeBase::GetRoomActorIsIn(AActor* actor)
{
	//loop through all rooms and find which room the actor is in
	for (int i = 0; i < Rooms.Num(); i++)
	{
		TArray<AActor*> OverlappingActors;

		Rooms[i]->RoomOverlap->GetOverlappingActors(OverlappingActors, actor->StaticClass());

		// loop through all overlapping actors and see if this actor is in it
		for (int j = 0; j < OverlappingActors.Num(); j++)
		{
			//if the searched actor is in this room, return it
			if (OverlappingActors[j] == actor)
			{
				return Rooms[i];
			}
		}
	}

	return nullptr;
}

//DEPERECATED returns an asset from the pool of meshes using a type delimiter DEPRECATED
TSubclassOf<AActor> ALootingLootersGameModeBase::GetAssetOfType(FString type)
{
	TArray<TSubclassOf<AActor>> ViableMeshes;

	//get all meshes that fit our type
	for (int i = 0; i < Game_Assets.Num(); i++)
	{
		if (Game_Assets[i]->GetName().Contains(type))
			ViableMeshes.Add(Game_Assets[i]);
	}

	//return a random one
	return ViableMeshes[FMath::RandRange(0, ViableMeshes.Num() - 1)];
}


TSubclassOf<AActor> ALootingLootersGameModeBase::GetRandomAssetOfTypes(TArray<FString> TypeSpecifiers)
{
	TArray<TSubclassOf<AActor>> ViableMeshes;

	//get all meshes that fit our type
	for (int i = 0; i < Game_Assets.Num(); i++)
	{
		bool AssetFitsSpecifications = true;

		for (int t = 0; t < TypeSpecifiers.Num(); t++)
		{
			//if it doesn't contain the search term flag it false and break out of the loop.
			if (Game_Assets[i]->GetName().Contains(TypeSpecifiers[t]) == false)
			{
				AssetFitsSpecifications = false;
				break;
			}
		}

		//if it qualifies add it to the list of meshes
		if (AssetFitsSpecifications)
			ViableMeshes.Add(Game_Assets[i]);
	}

	//return a random one
	if (ViableMeshes.Num() > 0)
		return ViableMeshes[FMath::RandRange(0, ViableMeshes.Num() - 1)];

	//if we didn't find a mesh return nullptr
	return nullptr;
}

//Pulls a random Loot blueprint from the list of loaded blueprints. Will return nullptr if no Loot assets exist
TSubclassOf <ALootActor> ALootingLootersGameModeBase::GetARandomLootAsset()
{
	//return nullptr if empty
	if (Loot_Assets.Num() == 0)
		return nullptr;

	//return a random loot asset
	return Loot_Assets[FMath::RandRange(0, Loot_Assets.Num() - 1)];
}

void ALootingLootersGameModeBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALootingLootersGameModeBase, Rooms);
}