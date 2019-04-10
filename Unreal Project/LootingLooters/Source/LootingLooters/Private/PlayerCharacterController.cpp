// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacterController.h"
#include "BaseCharacter.h"

APlayerCharacterController::APlayerCharacterController() : Super()
{
}

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

		//rotation of held item
 		InputComponent->BindAxis("RotateObjectX", this, &APlayerCharacterController::RotateHeldObjectX);
 		InputComponent->BindAxis("RotateObjectY", this, &APlayerCharacterController::RotateHeldObjectY);
		//For Gamepad
 		InputComponent->BindAxis("Zoom", this, &APlayerCharacterController::ZoomObject);
		//For PC
		InputComponent->BindAction("ZoomOut", IE_Pressed, this, &APlayerCharacterController::ZoomOut);
		InputComponent->BindAction("ZoomIn", IE_Pressed, this, &APlayerCharacterController::ZoomIn);

		//handle Actions
		InputComponent->BindAction("Interact", IE_Pressed, this, &APlayerCharacterController::Interact);
		InputComponent->BindAction("ThrowObject", IE_Pressed, this, &APlayerCharacterController::ThrowObject);
		InputComponent->BindAction("PlaceTrap", IE_Pressed, this, &APlayerCharacterController::PlaceTrap);
		InputComponent->BindAction("NextInventory", IE_Pressed, this, &APlayerCharacterController::NextInventory);
		InputComponent->BindAction("SetRotationMode", IE_Pressed, this, &APlayerCharacterController::RotateMode);
		InputComponent->BindAction("SetRotationMode", IE_Released, this, &APlayerCharacterController::RotateMode);
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
		owner->MoveForward(Value);
}

void APlayerCharacterController::MoveRight(float Value)
{
	ABaseCharacter* owner = Cast<ABaseCharacter>(GetPawn());
	if (owner)
		owner->MoveRight(Value);
}

void APlayerCharacterController::Turn(float Val)
{
	ABaseCharacter* owner = Cast<ABaseCharacter>(GetPawn());
	if (owner)
		owner->Turn(Val);
}

void APlayerCharacterController::LookUp(float Val)
{
	ABaseCharacter* owner = Cast<ABaseCharacter>(GetPawn());
	if (owner)
		owner->LookUp(Val);
}

void APlayerCharacterController::TurnAtRate(float Rate)
{
	ABaseCharacter* owner = Cast<ABaseCharacter>(GetPawn());
	if (owner)
		owner->TurnAtRate(Rate);
}

void APlayerCharacterController::LookUpAtRate(float Rate)
{
	ABaseCharacter* owner = Cast<ABaseCharacter>(GetPawn());
	if (owner)
		owner->LookUpAtRate(Rate);
}

//Will either interact if not already interacting
//if already interacting will stop
void APlayerCharacterController::Interact()
{
	ABaseCharacter* owner = Cast<ABaseCharacter>(GetPawn());
	if (owner)
		owner->Interact();
}

void APlayerCharacterController::RotateMode()
{
	ABaseCharacter* owner = Cast<ABaseCharacter>(GetPawn());
	if (owner)
		owner->RotateMode();
}

void APlayerCharacterController::ThrowObject()
{
	ABaseCharacter* owner = Cast<ABaseCharacter>(GetPawn());
	if (owner)
		owner->ServerThrowObject();
}

void APlayerCharacterController::PlaceTrap()
{
	ABaseCharacter* owner = Cast<ABaseCharacter>(GetPawn());
	if (owner)
		owner->PlaceTrap();
}

void APlayerCharacterController::NextInventory()
{
	ABaseCharacter* owner = Cast<ABaseCharacter>(GetPawn());
	if (owner)
		owner->NextInventory();
}

void APlayerCharacterController::ZoomOut()
{
	ABaseCharacter* owner = Cast<ABaseCharacter>(GetPawn());
	if (owner)
		owner->ZoomOut();
}

void APlayerCharacterController::ZoomIn()
{
	ABaseCharacter* owner = Cast<ABaseCharacter>(GetPawn());
	if (owner)
		owner->ZoomIn();
}

void APlayerCharacterController::RotateHeldObjectX(float Value)
{
	ABaseCharacter* owner = Cast<ABaseCharacter>(GetPawn());
	if (owner)
		owner->RotateHeldObjectX(Value);
}

void APlayerCharacterController::RotateHeldObjectY(float Value)
{
	ABaseCharacter* owner = Cast<ABaseCharacter>(GetPawn());
	if (owner)
		owner->RotateHeldObjectY(Value);
}

void APlayerCharacterController::ZoomObject(float Value)
{
	ABaseCharacter* owner = Cast<ABaseCharacter>(GetPawn());
	if (owner)
		owner->ZoomObject(Value);
}