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
	//fetch all static mesh files and load them up
	TArray<FString> FileResults;
	IFileManager& manager = IFileManager::Get();
	manager.SetSandboxEnabled(true);
	FString wildcard("*.uasset");

	//find all asset blueprints and load into memory
	FString AssetDirectory(FPaths::Combine(*FPaths::ProjectDir(), TEXT("Content/Blueprints/Furniture"), *wildcard));
	manager.FindFiles(FileResults, *AssetDirectory, true, false);

	//add all asset objects to the asset list
	FString ConstructorPath("/Game/Blueprints/Furniture/");
	for (int i = 0; i < FileResults.Num(); i++)
	{
		//raw filepath
		FString AssetFilePath = ConstructorPath + FileResults[i];

		//trim file ending
		AssetFilePath.RemoveFromEnd(".uasset");

		//find our blueprint and add it
		ConstructorHelpers::FObjectFinder<UBlueprint> AssetBlueprint(*AssetFilePath);

		//if the file was found add it to the list
		if (AssetBlueprint.Succeeded())
		{
			Game_Assets.Add( (UClass*) AssetBlueprint.Object->GeneratedClass);
		}
	}
	FileResults.Empty();

	//find all room blueprints and load them into memory
	FString RoomDirectory(FPaths::Combine(*FPaths::ProjectDir(), TEXT("Content/Blueprints/Test_Rooms"), *wildcard));
	manager.FindFiles(FileResults, *RoomDirectory, true, false);

	//add all room objects to the asset list
	ConstructorPath = "/Game/Blueprints/Test_Rooms/";
	for (int i = 0; i < FileResults.Num(); i++)
	{
		//raw filepath
		FString RoomFilePath = ConstructorPath + FileResults[i];

		//trim file ending
		RoomFilePath.RemoveFromEnd(".uasset");

		//find our blueprint and add it
		ConstructorHelpers::FObjectFinder<UBlueprint> RoomBlueprint(*RoomFilePath);

		//if the file was found add it to the list
		if (RoomBlueprint.Succeeded())
		{
			Room_Assets.Add((UClass*)RoomBlueprint.Object->GeneratedClass);
		}
	}
	FileResults.Empty();


	//find all loot blueprints and load them into memory
	FString LootDirectory(FPaths::Combine(*FPaths::ProjectDir(), TEXT("Content/Blueprints/Loot/"), *wildcard));
	manager.FindFiles(FileResults, *LootDirectory, true, false);

	//add all Loot objects to the asset list
	ConstructorPath = "/Game/Blueprints/Loot/";
	for (int i = 0; i < FileResults.Num(); i++)
	{
		//raw filepath
		FString LootFilePath = ConstructorPath + FileResults[i];

		//trim file ending
		LootFilePath.RemoveFromEnd(".uasset");

		//find our blueprint and add it
		ConstructorHelpers::FObjectFinder<UBlueprint> LootBlueprint(*LootFilePath);

		//if the file was found add it to the list
		if (LootBlueprint.Succeeded())
		{
			Loot_Assets.Add((UClass*)LootBlueprint.Object->GeneratedClass);
		}
	}
	FileResults.Empty();


	manager.SetSandboxEnabled(false);
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
TSubclassOf<AAssetTemplate> ALootingLootersGameModeBase::GetAssetOfType(FString type)
{
	TArray<TSubclassOf<AAssetTemplate>> ViableMeshes;

	//get all meshes that fit our type
	for (int i = 0; i < Game_Assets.Num(); i++)
	{
		if (Game_Assets[i]->GetName().Contains(type))
			ViableMeshes.Add(Game_Assets[i]);
	}

	//return a random one
	return ViableMeshes[FMath::RandRange(0, ViableMeshes.Num() - 1)];
}


TSubclassOf<class AAssetTemplate> ALootingLootersGameModeBase::GetRandomAssetOfTypes(TArray<FString> TypeSpecifiers)
{
	TArray<TSubclassOf<AAssetTemplate>> ViableMeshes;

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