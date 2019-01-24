// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"

APlayerCharacter::APlayerCharacter() : Super()
{
	PrimaryActorTick.bCanEverTick = true;

	//CAMERA
	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(RootComponent); //Root is Capsule in ACharacter.cpp
	Camera->bUsePawnControlRotation = true;

	Camera->SetRelativeLocation(FVector(0.0f, 0.0f, BaseEyeHeight));

	//Make it so we can't see our own mesh but others can
	GetMesh()->SetOwnerNoSee(true);

	CameraManager = CreateDefaultSubobject<APlayerCameraManager>("CameraManager");

}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

bool APlayerCharacter::PerformRayCast(ECollisionChannel TraceChannel)
{
	//Since the camera is positioned on the player's eyes it should be the same position
	//FVector campos;
	//FRotator camrot;
	//GetActorEyesViewPoint(campos, camrot);

	//A struct that the trace will populate with the results of the hit
	FHitResult OutHit;
	//The starting position of the trace, in our case the camera
	FVector start = Camera->GetRelativeTransform().

	//GetWord()->LineTraceSingleByChannel();
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent * PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void APlayerCharacter::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
}