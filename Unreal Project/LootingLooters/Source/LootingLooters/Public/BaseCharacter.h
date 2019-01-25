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

	class AGrabbableStaticMeshActor* HeldObject;
	bool bIsInteracting = false;
	bool bIsRotating = false;

public:	
	// Called every frame
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//This component should only be here until we have a socket or something
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
		class USceneComponent* PickupLoc;

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
	 
	/** Called for forwards/backward input */
	virtual void MoveForward(float Value);
	/** Called for side to side input */
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

	virtual void Interact();	
	virtual void ThrowObject();

	virtual void RotateMode();

	virtual void RotateHeldObjectX(float Value) {};
	virtual void RotateHeldObjectY(float Value) {};

	virtual void ZoomObject(float Value);
};
