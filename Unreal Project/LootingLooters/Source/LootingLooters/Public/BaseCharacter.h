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

	UPROPERTY(Replicated)
		class AGrabbableStaticMeshActor* HeldObject;

	UPROPERTY(Replicated)
		bool bIsInteracting = false;
	UPROPERTY(Replicated)
		bool bIsRotating = false;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Last Door Used")
		class ADoorActor* LastDoorAccessed;

	UPROPERTY(VisibleAnywhere, Category = "Components")
		class USphereComponent* SphereComp;

public:	
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RayCast, meta = (AllowPrivateAccess = "true"))
		float InteractRange = 800.0f;
	 
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

	UFUNCTION(Server, Reliable, WithValidation)
	virtual void Interact();	

	UFUNCTION(Server, Reliable, WithValidation)
	virtual void Grab(FHitResult Hit);

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerDropItem();

	UFUNCTION(Server, Reliable, WithValidation)
	virtual void ServerThrowObject();

	virtual void ThrowObject();

	UFUNCTION()
		virtual void Die();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void NetMulticastOnDeath();

	UFUNCTION(Server, Reliable, WithValidation)
	virtual void PlaceTrap();

	void PickupObject();

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
