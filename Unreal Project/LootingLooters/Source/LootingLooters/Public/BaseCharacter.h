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

public:	
	// Called every frame
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	//CONTROLS
	// 
	 	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lock On Camera")
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lock On Camera")
		float BaseLookUpRate;
	 
	/** Called for forwards/backward input */
	UFUNCTION(BlueprintCallable, Category = "Errors")
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

	
	
};
