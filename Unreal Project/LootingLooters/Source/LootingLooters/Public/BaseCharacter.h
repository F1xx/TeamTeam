// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

UCLASS()
class LOOTINGLOOTERS_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//The object this player is holding.
	UPROPERTY(Replicated)
		class AGrabbableStaticMeshActor* HeldObject;

	//Bools to determine interaction with a held object when issuing commands.
	UPROPERTY(Replicated)
		bool bIsInteracting = false;
	UPROPERTY(Replicated)
		bool bIsRotating = false;

	//Used for tracking player movements by the guard.
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Last Door Used")
		class ADoorActor* LastDoorAccessed;

	//Collision sphere for overlapping pawns.
	UPROPERTY(VisibleAnywhere, Category = "Components")
		class USphereComponent* SphereComp;

public:	

	//Is this player dead? Used by the guard.
	UPROPERTY(BlueprintReadOnly, Replicated)
		bool bIsDead = false;

	// Called every frame
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//CONTROLS
	// 
	 	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
		float BaseLookUpRate;

	//How close the player needs to be to loot objects or grab objects.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RayCast, meta = (AllowPrivateAccess = "true"))
		float InteractRange = 800.0f;
	 
	//Our speed maximum
	UFUNCTION(BlueprintCallable)
		void SetMaxSpeed(float speed);

	UFUNCTION(BlueprintCallable)
		float GetMaxSpeed();

	/** Called for forwards/backward input */
	UFUNCTION(BlueprintCallable)
	virtual void MoveForward(float Value);
	/** Called for side to side input */
	UFUNCTION(BlueprintCallable)
	virtual void MoveRight(float Value);

	/* Handle horizontal mouse input */
	virtual void Turn(float Val);
	/* Handle vertical mouse input */
	virtual void LookUp(float Val);

	/* Handle horizontal analog stick input */
	virtual void TurnAtRate(float Rate);

	/* Handle vertical analog stick input */
	virtual void LookUpAtRate(float Rate);

	//returns true if it hit the given channel
	bool PerformRayCast(FName TraceProfile, FHitResult &OutHit);

	//Method for handling Player commands to attempt looting or picking up Grabbable meshes.
	UFUNCTION(Server, Reliable, WithValidation)
	virtual void Interact();	

	//Method for handling Player commands to attempt to place a trap.
	UFUNCTION(Server, Reliable, WithValidation)
		virtual void PlaceTrap();

	//Method for handling Player commands to attempt to grab a grabbable object.
	UFUNCTION(Server, Reliable, WithValidation)
	virtual void Grab(FHitResult Hit);

	//Method for handling commands to drop whatever the player is holding.
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerDropItem();

	//Method for handling Player commands to attemot throwing a grabbable object.
	UFUNCTION(Server, Reliable, WithValidation)
	virtual void ServerThrowObject();

	//Multicasted death function to affect the player across all clients.
	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void NetMulticastOnDeath();

	//Throw an object in client side.
	virtual void ThrowObject();

	//Pickup an object in client side.
	void PickupObject();

	//Called when the player is killed by a guard.
	UFUNCTION()
		virtual void Die();

	virtual void NextInventory(){};

	virtual void RotateMode();

	virtual void RotateHeldObjectX(float Value) {};
	virtual void RotateHeldObjectY(float Value) {};

	virtual void ZoomObject(float Value);
	virtual void ZoomOut();
	virtual void ZoomIn();

	virtual void SetLastDoorAccessed(ADoorActor* actor) { LastDoorAccessed = actor; }
	virtual ADoorActor* GetLastDoorAccessed() { return LastDoorAccessed; }
};
