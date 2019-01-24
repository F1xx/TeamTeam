// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacterController.h"
#include "BaseCharacter.h"

void APlayerCharacterController::Possess(APawn* aPawn)
{
	Super::Possess(aPawn);
}

void APlayerCharacterController::UnPossess()
{
	Super::UnPossess();
}

void APlayerCharacterController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent != nullptr)
	{

 		InputComponent->BindAxis("MoveForward", this, &APlayerCharacterController::MoveForward);
 		InputComponent->BindAxis("MoveRight", this, &APlayerCharacterController::MoveRight);
 
 		// Handle mouse aiming input
 		InputComponent->BindAxis("Turn", this, &APlayerCharacterController::Turn);
 		InputComponent->BindAxis("LookUp", this, &APlayerCharacterController::LookUp);
 
 		// Handle analog stick aiming input
 		InputComponent->BindAxis("TurnRate", this, &APlayerCharacterController::TurnAtRate);
 		InputComponent->BindAxis("LookUpRate", this, &APlayerCharacterController::LookUpAtRate);
	}
}

void APlayerCharacterController::AcknowledgePossession(APawn * PossesedPawn)
{
	Super::AcknowledgePossession(PossesedPawn);
}

void APlayerCharacterController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void APlayerCharacterController::MoveForward(float Value)
{
	ABaseCharacter* owner = Cast<ABaseCharacter>(GetPawn());
	if (owner)
	{
		owner->MoveForward(Value);
	}
}

void APlayerCharacterController::MoveRight(float Value)
{
	ABaseCharacter* owner = Cast<ABaseCharacter>(GetPawn());
	if (owner)
	{
		owner->MoveRight(Value);
	}
}

void APlayerCharacterController::Turn(float Val)
{
	ABaseCharacter* owner = Cast<ABaseCharacter>(GetPawn());
	if (owner)
	{
		owner->Turn(Val);
	}
}

void APlayerCharacterController::LookUp(float Val)
{
	ABaseCharacter* owner = Cast<ABaseCharacter>(GetPawn());
	if (owner)
	{
		owner->LookUp(Val);
	}
}

void APlayerCharacterController::TurnAtRate(float Rate)
{
	ABaseCharacter* owner = Cast<ABaseCharacter>(GetPawn());
	if (owner)
	{
		owner->TurnAtRate(Rate);
	}
}

void APlayerCharacterController::LookUpAtRate(float Rate)
{
	ABaseCharacter* owner = Cast<ABaseCharacter>(GetPawn());
	if (owner)
	{
		owner->LookUpAtRate(Rate);
	}
}
