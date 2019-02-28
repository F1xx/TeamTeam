// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LootingLootersGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class LOOTINGLOOTERS_API ALootingLootersGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

	ALootingLootersGameModeBase();

	virtual void StartPlay() override;

	UFUNCTION(Server, WithValidation, Reliable)
		void GenerateRandomRoomLayout();

	UFUNCTION(Server, WithValidation, Reliable)
		void GenerateRandomRoomConnections();

	UFUNCTION(Server, WithValidation, Reliable)
		void PopulateRoomSockets();

	UFUNCTION(Server, WithValidation, Reliable)
		void GenerateLoot();


public:

	void GetRoomArray(TArray<class ARoomActorBase*>& RoomArray);

	class ARoomActorBase* GetRoomActorIsIn(AActor* actor);

	TSubclassOf<class AAssetTemplate> GetAssetOfType(FString type);

	TSubclassOf<class AAssetTemplate> GetRandomAssetOfTypes(TArray<FString> TypeSpecifiers);

	class UParticleSystem* GetLootParticleSystem() { return LootParticle; }

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Rooms")
	TArray<ARoomActorBase*> Rooms;

protected:

	//how many rooms we're making
	UPROPERTY(EditAnywhere, Category = "Rooms")
		int Total_Rooms_To_Generate = 10;

	//our list of room asset blueprints
	UPROPERTY(BlueprintReadOnly, Category = "Rooms")
		TArray<TSubclassOf<class AStaticMeshActor>> Room_Meshes;

	//our list of game asset blueprints
	UPROPERTY(BlueprintReadOnly, Category = "Assets")
		TArray<TSubclassOf< AAssetTemplate>> Game_Assets;

	//sets what the loot will look like in-game
	UPROPERTY(EditAnywhere, Category = "Assets")
		UParticleSystem* LootParticle;
	
};