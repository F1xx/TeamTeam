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

#include "Net/UnrealNetwork.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// set our turn rates for CONTROLLER input
	BaseTurnRate = 80.f;
	BaseLookUpRate = 80.f;

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

	HeldObject = nullptr;

	SetReplicateMovement(true);
	GetCharacterMovement()->SetIsReplicated(true);
	SetReplicates(true);
	bAlwaysRelevant = true;
	GetCharacterMovement()->SetIsReplicated(true);
	GetCharacterMovement()->SetNetAddressable();

	SphereComp = CreateDefaultSubobject<USphereComponent>("Sphere Collision");
	SphereComp->SetCollisionProfileName("OverlapOnlyPawn");
	SphereComp->SetGenerateOverlapEvents(true);
	SphereComp->SetNotifyRigidBodyCollision(true);
	SphereComp->SetSphereRadius(65.0f);
	SphereComp->SetupAttachment(RootComponent);
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

void ABaseCharacter::Die()
{
	SetActorTickEnabled(false);

	ServerDropItem();

	NetMulticastOnDeath();
}

void ABaseCharacter::NetMulticastOnDeath_Implementation()
{
	//disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetVisibility(false, false);
	SphereComp->SetVisibility(false, false);

	//we dead
	bIsDead = true;

	//ragdoll time
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetSimulatePhysics(true);
}

bool ABaseCharacter::NetMulticastOnDeath_Validate()
{
	return true;
}

//Alters the charactermovementcomponent MaxWalkSpeed value
void ABaseCharacter::SetMaxSpeed(float speed)
{
	GetCharacterMovement()->MaxWalkSpeed = speed;
}

float ABaseCharacter::GetMaxSpeed()
{
	return GetCharacterMovement()->MaxWalkSpeed;
}

//INPUT CONTROL HANDLING START
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
//INPUT CONTROL HANDLING END

//Does a LineTraceSingleByProfile against the given TraceProfile name and returns the result.
//Also fills OutHit with the FHitResult of the cast
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

	return GetWorld()->LineTraceSingleByProfile(OutHit, campos, end, TraceProfile, Params);
}

bool ABaseCharacter::ServerDropItem_Validate()
{
	return true;
}
//NET CODE 1 NET PICKUP
//TODO: DropWeapon Implementation Function
void ABaseCharacter::ServerDropItem_Implementation()
{
	//if already interacting stop
	if (HeldObject)
	{
		HeldObject->Drop();
		HeldObject = nullptr;
		bIsInteracting = false;
	}
}

//Performs a raycast to see if we're looking at something we can interact with
//if we are already interacting, stop, otherwise try to start
void ABaseCharacter::Interact_Implementation()
{
	if (HeldObject)
	{
		ServerDropItem();
		return;
	}
	else
	{
		//A struct that the trace will populate with the results of the hit
		FHitResult Hit(ForceInit);

		//if it succeeds do something with it
		if (PerformRayCast(FName("GrabbableTrace"), Hit))
		{
			Grab(Hit);
		}
	}
}

bool ABaseCharacter::Interact_Validate()
{
	return true;
}

//Called from interact if it succeeded
//Attempts to change the hit actor into one we can interact with. If it succeeds, interact
//Such as picking up GrabbableStaticMeshActors
void ABaseCharacter::Grab_Implementation(FHitResult Hit)
{
	check(Hit.GetActor() != nullptr && "Passed a null weapon!");

	//Making sure what we hit was an actor
	if (HeldObject == nullptr)
	{
		AGrabbableStaticMeshActor* grabbable = Cast<AGrabbableStaticMeshActor>(Hit.GetActor());
		if (grabbable)
		{
			if (Role == ROLE_Authority)
			{
				//SET/ASSIGN CurrentWeapon to Weapon
				HeldObject = grabbable;
				//CALL AttachCurrentWeapon
				PickupObject();
			}
		}
	}
#ifdef UE_BUILD_DEBUG
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Emerald, FString("Hit: " + Hit.Actor->GetName()));
#endif
}

bool ABaseCharacter::Grab_Validate(FHitResult Hit)
{
	return true;
}

//If we are holding an object let go of it while providing force in our forward direction
void ABaseCharacter::ServerThrowObject_Implementation()
{
	if (HeldObject)
	{
		if (Role == ROLE_Authority)
		{
			ThrowObject();
		}
	}
}

bool ABaseCharacter::ServerThrowObject_Validate()
{
	return true;
}

//place a trap. This is more for the AI and will place a trap at the character's
void ABaseCharacter::PlaceTrap_Implementation()
{
	//nothing right now though
}

bool ABaseCharacter::PlaceTrap_Validate()
{
	return true;
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

void ABaseCharacter::PickupObject()
{
	if (HeldObject)
	{
		HeldObject->Pickup(this);
		bIsInteracting = true;
	}
}

//While held down Toggles between rotate mode and normal mode. If we're holding an object allow us to enter rotate mode so we can move it around
void ABaseCharacter::RotateMode()
{
	if (HeldObject)
	{
		bIsRotating = !bIsRotating;
	}
}

//if we are holding something this zooms it in/out based on Value
//USED FOR CONTROLLER
void ABaseCharacter::ZoomObject(float Value)
{
	if (HeldObject)
	{
		HeldObject->Zoom(Value * 5.0f);
	}
}

//if we are holding something this zooms it out
//USED FOR PC
void ABaseCharacter::ZoomOut()
{
	if (HeldObject)
	{
		HeldObject->Zoom(15.0f);
	}
}

//if we are holding something this zooms it in
//USED FOR PC
void ABaseCharacter::ZoomIn()
{
	if (HeldObject)
	{
		HeldObject->Zoom(-15.0f);
	}
}

void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseCharacter, HeldObject);
	DOREPLIFETIME(ABaseCharacter, bIsInteracting);
	DOREPLIFETIME(ABaseCharacter, bIsRotating);
	DOREPLIFETIME(ABaseCharacter, LastDoorAccessed);
	DOREPLIFETIME(ABaseCharacter, bIsDead);
}