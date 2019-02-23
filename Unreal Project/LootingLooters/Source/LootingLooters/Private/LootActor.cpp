// Fill out your copyright notice in the Description page of Project Settings.

#include "LootActor.h"
#include "Components/SphereComponent.h"

#include "UObject/ConstructorHelpers.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
ALootActor::ALootActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Sphere = CreateDefaultSubobject<USphereComponent>("Collision");
	Sphere->SetSimulatePhysics(false);
	Sphere->SetEnableGravity(false);
	Sphere->SetCollisionProfileName("GrabbableTrace");

	Sphere->SetCanEverAffectNavigation(false); //Loot should be completely ignorable by AI
	
	RootComponent = Sphere;

	//this is hardcoding the particle system
	//leave commented for blueprint use
// 	static ConstructorHelpers::FObjectFinder<UParticleSystem> PS(TEXT("ParticleSystem'/Game/StarterContent/Particles/P_Fire'"));
// 	if (PS.Succeeded())
// 	{
// 		m_ParticleSystem = PS.Object;
// 	}

	m_ParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("MyPSC"));
	m_ParticleComponent->SetupAttachment(RootComponent);

	Tags.Add("Loot");
}

// Called when the game starts or when spawned
void ALootActor::BeginPlay()
{
	Super::BeginPlay();
	
	m_ParticleComponent->SetTemplate(m_ParticleSystem);
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

void ALootActor::Die()
{
	if (RespawnDelay > 0)
	{
		m_ParticleComponent->SetActive(false);
		GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &ALootActor::Respawn, RespawnDelay, false);

		Sphere->SetHiddenInGame(true, true);
		Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		PrimaryActorTick.bCanEverTick = false;
	}
	else
		Destroy();
}

void ALootActor::Respawn()
{
	m_ParticleComponent->SetActive(true);

	Sphere->SetHiddenInGame(false, true);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	PrimaryActorTick.bCanEverTick = true;
}