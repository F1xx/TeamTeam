// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Sound/SoundCue.h"
#include "LootingLootersGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class LOOTINGLOOTERS_API ALootingLootersGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

	ALootingLootersGameModeBase();


	// --------------------------------------------------------------------
	//     - Template function to load a TArray with blueprinted data. -
	//                             PARAMETERS
	// --------------------------------------------------------------------
	// 1. Array to load
	// 2. Folder directory
	// 3. Constructor directory
	// --------------------------------------------------------------------
	template<typename TemplateClass>
	void LoadAssetArray(TArray<TSubclassOf<TemplateClass>> &ArrayToLoad, FString FileDirectory, FString ConstructorDirectory)
	{
		//create filemanager and parsing array
		IFileManager& manager = IFileManager::Get();
		TArray<FString> FileResults;

		//fetch our files
		manager.FindFiles(FileResults, *FileDirectory, true, false);

		//add all objects to the asset list
		for (int i = 0; i < FileResults.Num(); i++)
		{
			//trim file ending
			FileResults[i].RemoveFromEnd(".uasset");

			//raw filepath
			FString AssetFilePath = "Class'" + ConstructorDirectory + FileResults[i] + "." + FileResults[i] + "_C'";

			//find our blueprint and add it
			ConstructorHelpers::FObjectFinder<UClass> AssetBlueprint(*AssetFilePath);

			//if the file was found add it to the list
			if (AssetBlueprint.Succeeded())
			{
				//create a blueprint template
				UClass* bpClass = AssetBlueprint.Object;
				TSubclassOf<TemplateClass> subclass = bpClass;

				//add it
				ArrayToLoad.Add(subclass);
			}
		}

		//remove changes
		FileResults.Empty();
		manager.SetSandboxEnabled(false);
	}

	virtual void StartPlay() override;

	//Helper to set PlayerCharacter logic upon login.
	void HandleNewPlayer(APlayerController* NewPlayer);

public:

	//Copies the room list.
	void GetRoomArray(TArray<class ARoomActorBase*>& RoomArray);
	
	//Copies the room templates
	void GetRoomTemplateArray(TArray<TSubclassOf<class AStaticMeshActor>>& AssetTemplateArray);

	//Will return the room that submitted Actor is in. Will return nullptr if Actor cannot be located in any room.
	class ARoomActorBase* GetRoomActorIsIn(AActor* actor);

	//DEPERECATED returns an asset from the pool of meshes using a type delimiter DEPRECATED
	TSubclassOf<AActor> GetAssetOfType(FString type);

	//returns an asset from the pool of meshes using all type specifiers. Will return nullptr if nothing fit the criteria
	TSubclassOf<AActor> GetRandomAssetOfTypes(TArray<FString> TypeSpecifiers);

	//Pulls a random Loot blueprint from the list of loaded blueprints. Will return nullptr if no Loot assets exist
	TSubclassOf <class ALootActor> GetARandomLootAsset();

	//Return the door blueprint.
	TSubclassOf<class AStaticMeshActor> GetDoorBlueprint() { return Door_Assets[0]; }

	//Called by PlayerCharacters when they are killed by the guard and will respawn the Player at the set location with their old team.
	void RespawnPlayer(APlayerController* NewPlayer, uint8 playerTeam, FTransform location);

	//Called by GameState to end the game once time has expired in the match.
	void Server_StartEndGame();

	//Override to apply PlayerCharacter specific logic upon a new controller logging in.
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	//Request to move all clients back to the main menu after match ends.
	virtual void EndMatch();

	//how many rooms we're making
	UPROPERTY(EditAnywhere, Category = "Rooms")
		int Total_Rooms_To_Generate = 10;

protected:

	//All room blueprint templates
	UPROPERTY(VisibleAnywhere, Category = "Assets")
		TArray<TSubclassOf<AStaticMeshActor>> Room_Assets;

	//All asset blueprint templates
	UPROPERTY(VisibleAnywhere, Category = "Assets")
		TArray<TSubclassOf< AActor>> Game_Assets;

	//All loot blueprint templates
	UPROPERTY(VisibleAnywhere, Category = "Assets")
		TArray<TSubclassOf< ALootActor>> Loot_Assets;

	//All door asset templates
	UPROPERTY(VisibleAnywhere, Category = "Assets")
		TArray<TSubclassOf<AStaticMeshActor>> Door_Assets;
};