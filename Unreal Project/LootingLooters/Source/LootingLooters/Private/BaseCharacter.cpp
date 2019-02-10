// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PlayerCharacterController.h"
#include "GrabbableStaticMeshActor.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	//Character Movement Defaults
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.0f;
	GetCharacterMovement()->AirControl = 0.2f;

	GetCapsuleComponent()->SetSimulatePhysics(false);

	PickupLoc = CreateDefaultSubobject<USceneComponent>("Puckup holding Location");
	PickupLoc->SetupAttachment(RootComponent);

	HeldObject = nullptr;
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ABaseCharacter::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ABaseCharacter::SetMaxSpeed(float speed)
{
	GetCharacterMovement()->MaxWalkSpeed = speed;
}

float ABaseCharacter::GetMaxSpeed()
{
	return GetCharacterMovement()->MaxWalkSpeed;
}

void ABaseCharacter::MoveForward(float Value)
{
	if (Controller && Value)
	{
		FRotator Rotation = Controller->GetControlRotation();
		FRotator YawRotation = FRotator::ZeroRotator;
		YawRotation.Yaw = Rotation.Yaw;
		FVector Direction = FRotationMatrix(YawRotation).GetScaledAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ABaseCharacter::MoveRight(float Value)
{
	if (Controller && Value)
	{
		FRotator Rotation = Controller->GetControlRotation();
		FRotator YawRotation = FRotator::ZeroRotator;
		YawRotation.Yaw = Rotation.Yaw;
		FVector Direction = FRotationMatrix(YawRotation).GetScaledAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void ABaseCharacter::Turn(float Val)
{
	if (!bIsRotating)
	{
		AddControllerYawInput(Val);
	}
}

void ABaseCharacter::LookUp(float Val)
{
	if (!bIsRotating)
	{
		AddControllerPitchInput(Val);
	}
}

void ABaseCharacter::TurnAtRate(float Rate)
{
	if (!bIsRotating)
	{
		AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	}
}

void ABaseCharacter::LookUpAtRate(float Rate)
{
	if (!bIsRotating)
	{
		AddControllerPitchInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	}
}

bool ABaseCharacter::PerformRayCast(FName TraceProfile, FHitResult &OutHit)
{
	FVector campos;
	FRotator camrot;
	GetController()->GetPlayerViewPoint(campos, camrot); //Fills with info from the camera
	//The starting position of the trace, Camera for player, eyes for bot
	FVector end = campos + (camrot.Vector() * InteractRange);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this); //required or they'll only see themself
	Params.bTraceComplex = true;

	bool result = GetWorld()->LineTraceSingleByProfile(OutHit, campos, end, TraceProfile, Params);

#ifdef UE_BUILD_DEBUG
	if (result)
	{
		if (OutHit.GetActor())
		{
			DrawDebugLine(GetWorld(), campos, OutHit.ImpactPoint, FColor::Red, false, 3.0f, 0, 3.0f);
		}
	}
	else
		DrawDebugLine(GetWorld(), campos, end, FColor::Blue, false, 3.0f, 0, 5.0f);
#endif

	return result;
}

//to interact perform a raycast to see if they can interact with what they're looking at
void ABaseCharacter::Interact()
{
	//if already interacting stop
	if (bIsInteracting)
	{
		//Pickup will drop the object if its already held
		HeldObject->Pickup(this);
		HeldObject = nullptr;
		bIsInteracting = false;
	}
	else //otherwise try to interact with what we're looking at
	{
		//A struct that the trace will populate with the results of the hit
		FHitResult Hit(ForceInit);

		//raycast to see
		bool result = PerformRayCast(FName("GrabbableTrace"), Hit);

		//if it succeeds do something with it
		if (result)
		{
			Grab(Hit);
		}
	}
}

void ABaseCharacter::Grab(FHitResult Hit)
{
	//Making sure what we hit was an actor
	if (Hit.GetActor())
	{
		if (Hit.GetActor()->ActorHasTag("Grabbable")) //pick it up if it can be grabbed
		{
			HeldObject = Cast<AGrabbableStaticMeshActor>(Hit.GetActor());
			if (HeldObject)
			{
				bIsInteracting = true;
				HeldObject->Pickup(this);
			}
		}
		else if (Hit.GetActor()->ActorHasTag("Trap")) //Disarm traps?
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Emerald, FString("ITS A TRAP"));//do something later
		}
	}
#ifdef UE_BUILD_DEBUG
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Emerald, FString("Hit: " + Hit.Actor->GetName()));
#endif
}

void ABaseCharacter::ThrowObject()
{
	if (HeldObject)
	{
		HeldObject->Throw();
		HeldObject = nullptr;
		bIsInteracting = false;
	}
}

//place a trap. This is more for the AI and will place a trap at the character's feet
void ABaseCharacter::PlaceTrap()
{

}

void ABaseCharacter::RotateMode()
{
	bIsRotating = !bIsRotating;
}

void ABaseCharacter::ZoomObject(float Value)
{
	if (HeldObject)
	{
		HeldObject->Zoom(Value);
	}
}

