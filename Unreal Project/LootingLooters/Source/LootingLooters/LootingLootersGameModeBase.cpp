// Fill out your copyright notice in the Description page of Project Settings.

#include "LootingLootersGameModeBase.h"
#include "EngineUtils.h" //Needed for TActorIterator
#include "Kismet/GameplayStatics.h"
#include "RoomActorBase.h"
#include "Net/UnrealNetwork.h"
#include "Engine/StaticMeshActor.h"
#include "Components/BoxComponent.h"

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

	if (Role == ROLE_Authority)
	{
		GenerateRandomRoomLayout();
		PopulateRoomSockets();
		GenerateRandomRoomConnections();
	}
}

void ALootingLootersGameModeBase::GenerateRandomRoomLayout_Implementation()
{
	check(Room_Meshes.Num() > 0);

	UWorld* world = GetWorld();

	if (world)
	{
		for (int i = 0; i < Total_Rooms_To_Generate; i++)
		{

			int random_index = FMath::RandRange(0, Room_Meshes.Num() - 1);

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			//Spacing so the rooms aren't on top of one another.
			FVector SpawnOffset = FVector(2000.0f * (i), 0.0f, -500.0f);
			FRotator SpawnRotation = FRotator(0.0f, 0.0f, 0.0f);

			ARoomActorBase* AddedRoom = world->SpawnActor<ARoomActorBase>(ARoomActorBase::StaticClass(), SpawnOffset, SpawnRotation, SpawnParams);
			AStaticMeshActor* AddedRoomMesh = world->SpawnActor<AStaticMeshActor>(Room_Meshes[random_index], SpawnOffset, SpawnRotation, SpawnParams);
			AddedRoom->SetRoomMesh(AddedRoomMesh);
			Rooms.Add(AddedRoom);
		}
	}
}

bool ALootingLootersGameModeBase::GenerateRandomRoomLayout_Validate()
{
	return HasAuthority();
}

void ALootingLootersGameModeBase::GenerateRandomRoomConnections_Implementation()
{
	for (int i = 0; i < Rooms.Num(); i++)
		Rooms[i]->GenerateDoorConnections();
}

bool ALootingLootersGameModeBase::GenerateRandomRoomConnections_Validate()
{
	return HasAuthority();
}

void ALootingLootersGameModeBase::PopulateRoomSockets_Implementation()
{
	for (int i = 0; i < Rooms.Num(); i++)
		Rooms[i]->PopulateEmptySockets();
}

bool ALootingLootersGameModeBase::PopulateRoomSockets_Validate()
{
	return HasAuthority();
}

void ALootingLootersGameModeBase::GetRoomArray(TArray<ARoomActorBase*>& RoomArray)
{
	RoomArray = Rooms;
}

//returns nullptr if the actor can't be found in any room
class ARoomActorBase* ALootingLootersGameModeBase::GetRoomActorIsIn(AActor* actor)
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

//void ARoomActorBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
//{
//	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//
//	DOREPLIFETIME(ALootingLootersGameModeBase, Rooms);
//}