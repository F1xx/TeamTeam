// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerCharacterController.generated.h"

/**
 * 
 */
UCLASS()
class LOOTINGLOOTERS_API APlayerCharacterController : public APlayerController
{
	GENERATED_BODY()

public:
	APlayerCharacterController();

	virtual void Possess(APawn* aPawn) override;
	virtual void UnPossess() override;

	//Ambient music component.
	UPROPERTY(EditDefaultsOnly)
		class UAudioComponent* m_Music;

	//Chase music component.
	UPROPERTY(EditDefaultsOnly)
		class UAudioComponent* m_ChaseMusic;

protected:
	virtual void SetupInputComponent() override;
	virtual void AcknowledgePossession(APawn* PossesedPawn) override;
	virtual void Tick(float DeltaSeconds) override;

	//Delegate to the BaseCharacter to handle
	/** Called for forwards/backward input */
	void MoveForward(float Value);
	/** Called for side to side input */
	void MoveRight(float Value);

	/* Handle horizontal mouse input */
	void Turn(float Val);
	/* Handle vertical mouse input */
	void LookUp(float Val);

	/* Handle horizontal analog stick input */
	void TurnAtRate(float Rate);

	/* Handle vertical analog stick input */
	void LookUpAtRate(float Rate);

	void Interact();
	void RotateMode();
	void ThrowObject();
	void PlaceTrap();
	void NextInventory();

	void ZoomOut();
	void ZoomIn();

	void RotateHeldObjectX(float Value);
	void RotateHeldObjectY(float Value);

	void ZoomObject(float Value);
};
