// Fill out your copyright notice in the Description page of Project Settings.

#include "../public/LootingLootersGameStateBase.h"
#include "Net/Unrealnetwork.h"
#include "Engine/World.h"
#include "RoomActorBase.h"
#include "Kismet/GameplayStatics.h"
#include "LootingLootersGameModeBase.h"
#include "Engine/StaticMeshActor.h"




ALootingLootersGameStateBase::ALootingLootersGameStateBase()
{
	SetReplicates(true);
}

void ALootingLootersGameStateBase::BeginPlay()
{
	Super::BeginPlay();

	Server_GenerateRandomRoomLayout();
	Server_PopulateRoomSockets();
	Server_GenerateRandomRoomConnections();
	Server_GenerateLoot();
}

void ALootingLootersGameStateBase::Server_GenerateRandomRoomLayout_Implementation()
{
	TArray<TSubclassOf<AStaticMeshActor>> Room_Assets;
	Cast<ALootingLootersGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()))->GetRoomTemplateArray(Room_Assets);

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

bool ALootingLootersGameStateBase::Server_GenerateRandomRoomLayout_Validate()
{
	return true;
}

void ALootingLootersGameStateBase::Server_PopulateRoomSockets_Implementation()
{
	//iterate through all rooms and have them generate assets to fill them
	for (int i = 0; i < Rooms.Num(); i++)
		Rooms[i]->PopulateEmptySockets();
}

bool ALootingLootersGameStateBase::Server_PopulateRoomSockets_Validate()
{
	return true;
}

void ALootingLootersGameStateBase::Server_GenerateRandomRoomConnections_Implementation()
{
	//iterate through all rooms and have them connect to each other
	for (int i = 0; i < Rooms.Num(); i++)
		Rooms[i]->Server_GenerateDoorConnections();
}

bool ALootingLootersGameStateBase::Server_GenerateRandomRoomConnections_Validate()
{
	return true;
}

void ALootingLootersGameStateBase::Server_GenerateLoot_Implementation()
{
	//iterate through all rooms and have them generate loot on their assets
	for (int i = 0; i < Rooms.Num(); i++)
		Rooms[i]->GenerateRandomLoot(-1);
}

bool ALootingLootersGameStateBase::Server_GenerateLoot_Validate()
{
	return true;
}

void ALootingLootersGameStateBase::GetRoomArray(TArray<class ARoomActorBase*>& RoomArray)
{
	RoomArray = Rooms;
}

void ALootingLootersGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALootingLootersGameStateBase, Rooms);
}


