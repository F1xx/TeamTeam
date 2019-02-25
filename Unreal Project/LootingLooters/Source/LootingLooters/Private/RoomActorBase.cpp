// Fill out your copyright notice in the Description page of Project Settings.

#include "RoomActorBase.h"
#include "Kismet/GameplayStatics.h"
#include "LootingLootersGameModeBase.h"
#include "Net/UnrealNetwork.h"
#include "DoorActor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"

ARoomActorBase::ARoomActorBase()
{
	RoomOverlap = CreateDefaultSubobject<UBoxComponent>("OverlapBox");
	RoomOverlap->SetCollisionResponseToAllChannels(ECR_Ignore);
	RoomOverlap->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RoomOverlap->SetMobility(EComponentMobility::Static);
	RoomOverlap->SetupAttachment(RootComponent);
}

void ARoomActorBase::SetRoomMesh(AStaticMeshActor* Mesh)
{
	RoomMesh = Mesh;
	RoomMesh->SetOwner(Cast<AActor>(this));
	RoomMesh->AttachToActor(Cast<AActor>(this), FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	FVector origin;
	FVector bounds;

	RoomMesh->GetActorBounds(false, origin, bounds);
	RoomOverlap->SetBoxExtent(bounds);
}

void ARoomActorBase::PopulateEmptySockets()
{
	//Get all static meshes
	TArray<UStaticMeshComponent*> Meshes;
	RoomMesh->GetComponents<UStaticMeshComponent>(Meshes);

	//get our game mode
	ALootingLootersGameModeBase* GameMode = Cast<ALootingLootersGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));

	for (int i = 0; i < Meshes.Num(); i++)
	{
		//Get all socket names
		TArray<FName> Socket_Names = Meshes[i]->GetAllSocketNames();

		//Check each socket name and fill it appropriately
		for (int a = 0; a < Socket_Names.Num(); a++)
		{
			//string parsing
			FString name = Socket_Names[a].ToString();
			TArray<FString> name_parsed;
			name.ParseIntoArray(name_parsed, TEXT("_"), true);

			//Spawning parameters
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			FVector SpawnLocation = Meshes[i]->GetSocketLocation(Socket_Names[a]);
			FRotator SpawnRotation = Meshes[i]->GetSocketRotation(Socket_Names[a]);

			if (name_parsed[0] == "Door")
			{
				ADoorActor* door = GetWorld()->SpawnActor<ADoorActor>(ADoorActor::StaticClass(), SpawnLocation, SpawnRotation, SpawnParams);
				door->SetOwner(this);
				GeneratedDoors.Add(door);
			}
			else
				//spawn assets. for now we're just spawning using the first filter
			{
				UStaticMesh* AssetMesh = GameMode->GetMeshOfType(name_parsed[0]);
				AStaticMeshActor* Asset = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), SpawnLocation, SpawnRotation, SpawnParams);

				//temporarily adjusting mobility to set mesh
				Asset->SetMobility(EComponentMobility::Movable);
				Asset->GetStaticMeshComponent()->SetStaticMesh(AssetMesh);
				Asset->SetActorScale3D(FVector(4.5f, 4.5f, 4.5f));
				Asset->SetMobility(EComponentMobility::Static);
			}
		}
	}


}

void ARoomActorBase::GenerateRandomLoot(unsigned int loot_count)
{
	//things
}

/*
RULES FOR DOOR CONNECTION LOGIC

1: Always connect to a room only once. (2 doors cannot connect to the same room)
2: Don't connect a door to your own room.
3. Unconnected doors simply will not move you. (It's possible we'll have loose ends)

*/
void ARoomActorBase::GenerateDoorConnections()
{
	if (Role == ROLE_Authority)
	{
		ALootingLootersGameModeBase* GameMode = Cast<ALootingLootersGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));

		if (GameMode)
		{
			//our entire room list
			TArray<ARoomActorBase*> TotalRooms;
			GameMode->GetRoomArray(TotalRooms);

			//Get all doors with connections (we need this to ignore rooms we've already connected with)
			TArray<ADoorActor*> ConnectedDoors;
			GetOrganizedDoors(ConnectedDoors, true);

			//if the number of connnected doors is the same as the total doors, get out of here.
			if (ConnectedDoors.Num() == GeneratedDoors.Num())
				return;

			//Our list of rooms filtered to only contain the ones still missing connections
			TArray<ARoomActorBase*> UnconnectedRooms;
			for (int i = 0; i < TotalRooms.Num(); i++)
			{
				//ignore any rooms that we know about
				for (int a = 0; a < ConnectedDoors.Num(); a++)
					if (TotalRooms[i] == ConnectedDoors[a]->GetOwner())
						continue;

				//ignore our own room
				if (TotalRooms[i] == this)
					continue;

				//If the room has available connections add it 
				if (TotalRooms[i]->GetNumberOfDoorsWithoutAConnection() > 0)
					UnconnectedRooms.Add(TotalRooms[i]);
			}

			//Get all unconnected doors that require a connection
			TArray<ADoorActor*> UnconnectedDoors;
			GetOrganizedDoors(UnconnectedDoors, false);

			//iterate through all unconnected doors and connect them all
			for (int i = 0; i < UnconnectedDoors.Num(); i++)
			{
				if (UnconnectedRooms.Num() == 0)
					break;

				//Choose a random number clamped to our list of rooms
				int random = FMath::RandRange(0, UnconnectedRooms.Num() - 1);

				//connect to said room
				ARoomActorBase* ChosenRoom = UnconnectedRooms[random];
				UnconnectedDoors[i]->ApplyConnection(ChosenRoom->GetARandomUnconnectedDoor());

				//remove it from the list now that a connection has been made
				UnconnectedRooms.RemoveSingle(ChosenRoom);
			}
		}
	}
}



void ARoomActorBase::GetDoorArray(TArray<ADoorActor*> &Output)
{
	Output = GeneratedDoors;
}

int ARoomActorBase::GetNumberOfDoorsWithoutAConnection()
{
	int num = 0;
	for (int i = 0; i < GeneratedDoors.Num(); i++)
		num = GeneratedDoors[i]->IsConnected() == false ? num + 1 : num;

	return num;
}

ADoorActor* ARoomActorBase::GetARandomUnconnectedDoor()
{
	// get a list of unconnected doors
	TArray<ADoorActor*> UnconnectedDoors;
	GetOrganizedDoors(UnconnectedDoors, false);

	//return a random one
	if (UnconnectedDoors.Num() > 0)
		return UnconnectedDoors[FMath::Rand() % UnconnectedDoors.Num()];
	else
		return nullptr;
}

void ARoomActorBase::GetOrganizedDoors(TArray<ADoorActor*> &Output, bool connected/* = true*/)
{
	TArray<ADoorActor*> UnconnectedDoors;

	//iterate through the list of doors and add all that follow the connection rule
	for (int i = 0; i < GeneratedDoors.Num(); i++)
		if (GeneratedDoors[i]->IsConnected() == connected)
			UnconnectedDoors.Add(GeneratedDoors[i]);

	Output = UnconnectedDoors;
}

void ARoomActorBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARoomActorBase, GeneratedDoors);
}
