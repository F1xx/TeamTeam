// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "PlayerCharacter.generated.h"

/**
 * 
 */
UCLASS()
class LOOTINGLOOTERS_API APlayerCharacter : public ABaseCharacter
{
	GENERATED_BODY()
	
public:
	APlayerCharacter();
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Camera
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* Camera;

	//Player inventory. Keeps track of traps and updates the player state as needed.
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
		class UInventoryComponent* m_Inventory;

public:
	//Server function call to have the server call a sound change from ambient to chase (or vice versa)
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_BeingChased(bool chased);

	UFUNCTION(Client, Reliable)
		void Client_StartMusic();

	//Client method call to change music from ambient to chase (or vice versa).
	//Server_BeingChased calls this method to ensure each client hears their own music.
	UFUNCTION(Client, Reliable)
		void Client_BeingChased(bool chased);

	//PlayerState helper.
	UFUNCTION()
		class AMyPlayerState* GetPlayerState();

	//GameState helper.
	UFUNCTION()
		class ALootingLootersGameStateBase* GetGameState();

	UFUNCTION()
		class APlayerCharacterController* GetPlayerCharacterController();

	//InventoryComponent helper.
	UFUNCTION(BlueprintCallable)
		UInventoryComponent* GetInventoryComponent() { return m_Inventory; }

	//Method to set our Respawn Timer as well as our RespawnLocation transform so we respawn in place.
	//also disables input while waiting for respawn.
	virtual void Die() override;

	//Method to disable Sound and for the Server Authority to begin Respawning the player.
	UFUNCTION()
		virtual void Respawn();

	//Multicasted method to reset old data and commit to ragdoll while waiting for respawn.
	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void NetMulticastOnRespawn();

	//Setup of Sound components after initialization.
	virtual void PostInitializeComponents() override;

	//Method for handling Player commands to attempt looting.
	virtual void Interact() override;

	//Method for handling Player commands to attempt to place a trap.
	virtual void PlaceTrap() override;

	//Method for handling Player commands to cycle active inventory item.
	virtual void NextInventory() override;

	// Called every frame
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//Rotation methods while holding a Grabbable Object.
	virtual void RotateHeldObjectX(float Value) override;
	virtual void RotateHeldObjectY(float Value) override;

	//Called by GameMode to set our Team (whch is then replicated to all clients).
	void AssignTeam();

	//Called by GameMode on StartPlay and RestartPlayer to set the DefaultMaterial 
	UFUNCTION(NetMulticast, Reliable)
		void Multicast_AssignDefaultMaterial();

	//Called once on PostBeginPlay by Server Authority to apply the Material to the mesh.
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
		void NetMulticast_SetColor();

	//Server call to play a loot sound. 
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_PlayLootSound(FVector location);

	//Called by Server_PlayLootSound to cause all clients to hear the loot sound played.
	UFUNCTION(NetMulticast, Reliable)
		void NetMultiCast_PlayLootSound(FVector location);

	//Delayed run-once function to apply Color to the PlayerCharacter during runtime.
	UFUNCTION(BlueprintCallable)
		void PostBeginPlay();

	//Called by GameMode upon EndGame to disable controller input for all clients.
	UFUNCTION(NetMulticast, Reliable)
		void NetMultiCast_DisableControllerInputs();

	//PostBeginPlay timer handle
	UPROPERTY(Replicated)
		FTimerHandle PostBeginPlayDelay;

	//Our Respawn timer handle.
	UPROPERTY(Replicated)
		FTimerHandle RespawnTimer;

	//Material used to set the Skeletal Mesh color during runtime.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Materials, Replicated)
		class UMaterialInterface* DefaultMaterial;

	UFUNCTION(BlueprintCallable, Category = "Camera")
		FORCEINLINE class UCameraComponent* GetFollowCamera() const { return Camera; }

	//The team we belong to. Used for keeping track of Score as well as determining Trap collision to prevent friendly fire.
	UPROPERTY(Replicated, BlueprintReadOnly)
		uint8 Team;

	//Location to be set where the player will respawn if they die to the guard.
	UPROPERTY(Replicated, BlueprintReadOnly)
		FTransform RespawnLoc;

	//Our loot sound.
	UPROPERTY(Replicated, EditDefaultsOnly)
		class USoundWave* m_LootSound;
};
