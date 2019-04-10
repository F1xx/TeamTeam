// Fill out your copyright notice in the Description page of Project Settings.

#include "LootActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

#include "UObject/ConstructorHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "LootingLootersGameModeBase.h"

#include "Net/UnrealNetwork.h"

// Sets default values
ALootActor::ALootActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Sphere = CreateDefaultSubobject<USphereComponent>("Collision");
	Sphere->SetSimulatePhysics(false);
	Sphere->SetEnableGravity(false);
	Sphere->SetCollisionProfileName("GrabbableTrace");
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	Sphere->SetCanEverAffectNavigation(false); //Loot should be completely ignorable by AI

	RootComponent = Sphere;

	m_ParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("MyPSC"));
	m_ParticleComponent->SetupAttachment(RootComponent);

	SetReplicates(true);

	Tags.Add("Loot");
}

// Called when the game starts or when spawned
void ALootActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ALootActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

//use this to use the right mesh or whatever
//100% chance to find a trap = different appearance
// also forces the chance to remain between 0 and 100.
void ALootActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (ChanceToFindTrap >= 100)
	{
		ChanceToFindTrap = 100;
		//Change appearance
	}
	else if (ChanceToFindTrap < 0)
	{
		ChanceToFindTrap = 0;
	}
}

void ALootActor::NetMulticast_Die_Implementation()
{
	Die();
}

//Either actually destroys the actor or "turns it off" depending on if it can respawn
void ALootActor::Die()
{
	if (Role == ROLE_Authority)
	{
		if (RespawnDelay > 0)
		{
			SetActorHiddenInGame(true);
			SetActorEnableCollision(false);
			SetActorTickEnabled(false);

			GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &ALootActor::Respawn, RespawnDelay, false);
			isDead = true;
		}
		else
			Destroy();
	}
}

//set its variables back to what they should be to exist in the world
void ALootActor::Respawn()
{
	if (Role == ROLE_Authority)
	{
		SetActorHiddenInGame(false);
		SetActorEnableCollision(true);
		SetActorTickEnabled(true);
		isDead = false;
	}
}

void ALootActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALootActor, RespawnTimer);
	DOREPLIFETIME(ALootActor, m_ParticleSystem);
	DOREPLIFETIME(ALootActor, m_ParticleComponent);
	DOREPLIFETIME(ALootActor, Sphere);
	DOREPLIFETIME(ALootActor, isDead);
}