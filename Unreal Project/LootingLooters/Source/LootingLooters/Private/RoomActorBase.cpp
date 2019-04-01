// Fill out your copyright notice in the Description page of Project Settings.

#include "RoomActorBase.h"
#include "Kismet/GameplayStatics.h"
#include "LootingLootersGameModeBase.h"
#include "Net/UnrealNetwork.h"
#include "DoorActor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "AssetTemplate.h"
#include "GrabbableStaticMeshActor.h"

ARoomActorBase::ARoomActorBase()
{
	SetReplicates(true);

	RoomOverlap = CreateDefaultSubobject<UBoxComponent>("OverlapBox");
	RoomOverlap->SetCollisionResponseToAllChannels(ECR_Ignore);
	RoomOverlap->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RoomOverlap->SetMobility(EComponentMobility::Static);
	RoomOverlap->SetupAttachment(RootComponent);
	RootComponent = RoomOverlap;
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

	if (HasAuthority())
	{
		//Get all static meshes
		TArray<UStaticMeshComponent*> Meshes;
		RoomMesh->GetComponents<UStaticMeshComponent>(Meshes);

		//safety check in case a room mesh wasn't actually added for some reason
		if (Meshes.Num() > 1)
		{
			//iterate through all meshes that make up the room and check their sockets for spawning objects.
			for (int i = 0; i < Meshes.Num(); i++)
			{
				//Get all socket names
				TArray<FName> Socket_Names = Meshes[i]->GetAllSocketNames();

				//Check each socket name and fill it appropriately
				for (int a = 0; a < Socket_Names.Num(); a++)
				{
					//string parsing
					FString Raw_Socket_Name = Socket_Names[a].ToString();
					TArray<FString> Type_Names;
					Raw_Socket_Name.ParseIntoArray(Type_Names, TEXT("_"), true);

					//remove the last element (this is a socket naming number and isn't needed)
					Type_Names.RemoveAt(Type_Names.Num() - 1, 1, true);

					//Spawning parameters
					FVector SpawnLocation = Meshes[i]->GetSocketLocation(Socket_Names[a]);
					FRotator SpawnRotation = Meshes[i]->GetSocketRotation(Socket_Names[a]);

					//spawn a door if the socket is a door
					if (Type_Names[0] == "Door")
					{
						Server_SpawnDoor(SpawnLocation, SpawnRotation);
					}
					//otherwise spawn assets. We can get as specific as we want when spawning items.
					else
					{
						Server_SpawnAsset(Type_Names, SpawnLocation, SpawnRotation);
					}
				}
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
void ARoomActorBase::Server_GenerateDoorConnections_Implementation()
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

			//get our lucky door
			ARoomActorBase* ChosenRoom = UnconnectedRooms[random];
			ADoorActor* OtherDoor = ChosenRoom->GetARandomUnconnectedDoor();

			//connect them together
			UnconnectedDoors[i]->ApplyConnection(OtherDoor);
			OtherDoor->ApplyConnection(UnconnectedDoors[i]);

			//remove it from the list now that a connection has been made
			UnconnectedRooms.RemoveSingle(ChosenRoom);
		}
	}
}

bool ARoomActorBase::Server_GenerateDoorConnections_Validate()
{
	return true;
}

void ARoomActorBase::Server_SpawnDoor_Implementation(FVector SpawnLocation, FRotator SpawnRotation)
{
	//get our game mode
	ALootingLootersGameModeBase* GameMode = Cast<ALootingLootersGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));

	//Spawning parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ADoorActor* door = GetWorld()->SpawnActor<ADoorActor>(GameMode->GetDoorBlueprint(), SpawnLocation, SpawnRotation, SpawnParams);
	door->SetOwner(this);
	GeneratedDoors.Add(door);
}

bool ARoomActorBase::Server_SpawnDoor_Validate(FVector SpawnLocation, FRotator SpawnRotation)
{
	return true;
}

void ARoomActorBase::Server_SpawnAsset_Implementation(const TArray<FString>& TypeNameArray, FVector SpawnLocation, FRotator SpawnRotation)
{
	//get our game mode
	ALootingLootersGameModeBase* GameMode = Cast<ALootingLootersGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));

	//fetch a random mesh that fits our type specifiers
	TSubclassOf<AActor> AssetMesh = GameMode->GetRandomAssetOfTypes(TypeNameArray);

	//Spawning parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AssetMesh != nullptr)
	{
		//create the asset
		AActor* Asset = GetWorld()->SpawnActor<AActor>(*AssetMesh, SpawnLocation, SpawnRotation, SpawnParams);
		Asset->SetOwner(this);

		//check to see which array to push the asset into
		if (Asset->ActorHasTag("Grabbable") == false)
		{
			AAssetTemplate* StaticMeshObject = Cast<AAssetTemplate>(Asset);

			//add our static object and populate loot sockets
			if (StaticMeshObject)
			{
				StaticMeshObject->PopulateLootSockets();
				StaticRoomAssets.Add(StaticMeshObject);
			}
		}
		else
		{
			AGrabbableStaticMeshActor* GrabbableMeshObject = Cast<AGrabbableStaticMeshActor>(Asset);

			//add our grabbable mesh
			if (GrabbableMeshObject)
				GrabbableRoomAssets.Add(GrabbableMeshObject);
		}

	}
}

bool ARoomActorBase::Server_SpawnAsset_Validate(const TArray<FString>& TypeNameArray, FVector SpawnLocation, FRotator SpawnRotation)
{
	return true;
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
	DOREPLIFETIME(ARoomActorBase, RoomMesh);
	DOREPLIFETIME(ARoomActorBase, StaticRoomAssets);
	DOREPLIFETIME(ARoomActorBase, GrabbableRoomAssets);
}
