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
	PrimaryActorTick.bCanEverTick = true;

	//LoadSoundMap();
}

void ALootingLootersGameStateBase::BeginPlay()
{
	Super::BeginPlay();

	//Generate our world.
	Server_GenerateRandomRoomLayout();
	Server_PopulateRoomSockets();
	Server_GenerateRandomRoomConnections();
	Server_GenerateLoot();

	MatchCountDown = MatchLength;

	//Set our match duration timer.
	GetWorld()->GetTimerManager().SetTimer(MatchTimer, this, &ALootingLootersGameStateBase::Server_StartEndGame, MatchLength, false);
}

void ALootingLootersGameStateBase::Tick(float DeltaSeconds)
{
	MatchCountDown -= DeltaSeconds;

	if (MatchCountDown < 0.0f)
		MatchCountDown = 0.0f;
}

void ALootingLootersGameStateBase::Server_GenerateRandomRoomLayout_Implementation()
{
	//Get our world and populate it with a random room layout.
	TArray<TSubclassOf<AStaticMeshActor>> Room_Assets;
	UWorld* world = GetWorld();
	
	if (world)
	{
		//Get our rooms from the game mode.
		Cast<ALootingLootersGameModeBase>(UGameplayStatics::GetGameMode(world))->GetRoomTemplateArray(Room_Assets);
		check(Room_Assets.Num() > 0);

		//get the total amount of rooms to spawn and our grid size.
		int Total_Rooms_To_Generate = Cast<ALootingLootersGameModeBase>(UGameplayStatics::GetGameMode(world))->Total_Rooms_To_Generate;
		int room_root = FMath::Sqrt(Total_Rooms_To_Generate);

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

void ALootingLootersGameStateBase::Server_EndGame_Implementation()
{
	ALootingLootersGameModeBase* mode = Cast<ALootingLootersGameModeBase>(GetWorld()->GetAuthGameMode());

	if (mode)
	{
		mode->EndMatch();
	}
}

bool ALootingLootersGameStateBase::Server_EndGame_Validate()
{
	return true;
}

void ALootingLootersGameStateBase::Server_StartEndGame_Implementation()
{
	ALootingLootersGameModeBase* mode = Cast<ALootingLootersGameModeBase>(GetWorld()->GetAuthGameMode());
	if (mode)
	{
		mode->Server_StartEndGame();
	}

	//Flag ending to true for HUD.
	MatchEnding = true;

	//Set our buffer timer.
	GetWorld()->GetTimerManager().SetTimer(EndBufferTime, this, &ALootingLootersGameStateBase::Server_EndGame, BufferLength, false);
}

bool ALootingLootersGameStateBase::Server_StartEndGame_Validate()
{
	return true;
}

void ALootingLootersGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALootingLootersGameStateBase, Rooms);

	DOREPLIFETIME(ALootingLootersGameStateBase, TeamOneMaterials);
	DOREPLIFETIME(ALootingLootersGameStateBase, TeamTwoMaterials);
	DOREPLIFETIME(ALootingLootersGameStateBase, TeamThreeMaterials);
	DOREPLIFETIME(ALootingLootersGameStateBase, TeamFourMaterials);

	DOREPLIFETIME(ALootingLootersGameStateBase, PlayerOneLoggedIn);
	DOREPLIFETIME(ALootingLootersGameStateBase, PlayerTwoLoggedIn);
	DOREPLIFETIME(ALootingLootersGameStateBase, PlayerThreeLoggedIn);
	DOREPLIFETIME(ALootingLootersGameStateBase, PlayerFourLoggedIn);
	DOREPLIFETIME(ALootingLootersGameStateBase, MatchTimer);
	DOREPLIFETIME(ALootingLootersGameStateBase, EndBufferTime);
	DOREPLIFETIME(ALootingLootersGameStateBase, MatchLength);
	DOREPLIFETIME(ALootingLootersGameStateBase, BufferLength);
	DOREPLIFETIME(ALootingLootersGameStateBase, MatchEnding);
	DOREPLIFETIME(ALootingLootersGameStateBase, MatchCountDown);

	//DOREPLIFETIME(ALootingLootersGameStateBase, SoundsMap);
}

//CURRENTLY UNUSED BUT CAN BE REIMPLEMENTED IF WE FIX THE SOUND ISSUE WITH LOADING FILES//

//USoundWave* ALootingLootersGameStateBase::GetSoundWave(FString name)
//{
//	return *SoundsMap.Find(name);
//}

//void ALootingLootersGameStateBase::LoadSoundMap()
//{
//	//create filemanager and parsing array
//	IFileManager& manager = IFileManager::Get();
//
//	TArray<FString> FileResults;
//	FString wildcard("*.uasset");
//
//	FString FileDirectory(FPaths::Combine(*FPaths::ProjectDir(), TEXT("Content/Assets/Sound/"), *wildcard));
//	FString RootDirectory = "/Game/Assets/Sound/";
//	//SoundWave'/Game/Assets/Sound/FootSteps.FootSteps'
//
//	//fetch our files
//	manager.FindFiles(FileResults, *FileDirectory, true, false);
//
//	//add all objects to the asset list
//	for (int i = 0; i < FileResults.Num(); i++)
//	{
//		//trim file ending
//		FileResults[i].RemoveFromEnd(".uasset");
//
//		//raw filepath
//		FString AssetFilePath = RootDirectory + FileResults[i];
//
//		//find our blueprint and add it
//		ConstructorHelpers::FObjectFinder<USoundWave> AssetBlueprint(*AssetFilePath);
//
//		//if the file was found add it to the list
//		if (AssetBlueprint.Succeeded())
//		{
//			//create a blueprint template
//			USoundWave* bpClass = AssetBlueprint.Object;
//			USoundWave* subclass = bpClass;
//
//
//			//add it
//			SoundsMap.Add(FileResults[i], subclass);
//		}
//	}
//
//	//remove changes
//	FileResults.Empty();
//	manager.SetSandboxEnabled(false);
//}


