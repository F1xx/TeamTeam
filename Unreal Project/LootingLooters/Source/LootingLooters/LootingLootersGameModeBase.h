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


public:

	void GetRoomArray(TArray<class ARoomActorBase*>& RoomArray);

	class ARoomActorBase* GetRoomActorIsIn(AActor* actor);

	class UStaticMesh* GetMeshOfType(FString type);

protected:

	UPROPERTY(/*Replicated, */BlueprintReadOnly, Category = "Rooms")
	TArray<ARoomActorBase*> Rooms;

	UPROPERTY(EditAnywhere, Category = "Rooms")
		int Total_Rooms_To_Generate = 10;

	UPROPERTY(EditAnywhere, Category = "Rooms")
		TArray<TSubclassOf<class AStaticMeshActor>> Room_Meshes;

	UPROPERTY(BlueprintReadOnly, Category = "Assets")
		TArray<UStaticMesh*> Game_Assets;
	
};
