// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseTrapActor.h"
#include "UObject/ConstructorHelpers.h"
#include "BaseCharacter.h"

ABaseTrapActor::ABaseTrapActor() : Super()
{
	PrimaryActorTick.bCanEverTick = true;

	GetStaticMeshComponent()->SetCollisionProfileName(FName("GrabbableTrace"));
	GetStaticMeshComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	GetStaticMeshComponent()->SetGenerateOverlapEvents(true);
	GetStaticMeshComponent()->SetSimulatePhysics(true);
	
	SetMeshes("TRAP_Bear_Open", "TRAP_Bear_Closed");

	GetStaticMeshComponent()->OnComponentBeginOverlap.AddDynamic(this, &ABaseTrapActor::HandleOverlap);

	TrapDebuff = EDebuffs::DE_Nothing;
	Tags.Add("Trap");
}

void ABaseTrapActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ABaseTrapActor::HandleOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr) && (OtherActor != GetOwner()))
	{
		//do trap thing
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Emerald, FString("Someone Stepped on a TRAP!!!!!!!!!"));//do something later
	}

	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr) && (OtherActor == GetOwner()))
	{
		//Nothing actually should happen here. This is just so we know it knows it's owner
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Emerald, FString("My Owner Stepped on their own TRAP!!!!!!!!"));//do something later


		if (OtherActor->ActorHasTag("Player"))
		{
			ABaseCharacter* dummy = Cast<ABaseCharacter>(OtherActor);
			if (dummy)
			{
				GetStaticMeshComponent()->SetStaticMesh(ChangeMesh);
				SetTarget(dummy);
				ApplyDebuff();
			}
		}
	}
}

//for now just destroy itself
void ABaseTrapActor::Die()
{
	Destroy();
}

//for some reason is not properly finding the changemesh
void ABaseTrapActor::SetMeshes(FString mesh1, FString mesh2)
{
	FString starting = "/Game/Assets/TrapMeshes/" + mesh1;
	const TCHAR* startingchar = *starting;

	FString ending = "/Game/Assets/TrapMeshes/" + mesh2;
	const TCHAR* endingchar = *ending;

	ConstructorHelpers::FObjectFinder<UStaticMesh> FoundMesh(startingchar);
	if (FoundMesh.Succeeded())
	{
		GetStaticMeshComponent()->SetStaticMesh(FoundMesh.Object);
	}

	ConstructorHelpers::FObjectFinder<UStaticMesh> SecondFoundMesh(endingchar);
	if (FoundMesh.Succeeded())
	{
		ChangeMesh = FoundMesh.Object;
	}
}

void ABaseTrapActor::SetTarget(class ABaseCharacter* character)
{
	m_Target = character;
}