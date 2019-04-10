// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseTrapActor.h"
#include "UObject/ConstructorHelpers.h"
#include "BaseCharacter.h"

#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerCharacter.h"

ABaseTrapActor::ABaseTrapActor() : Super()
{
	Team = -1;
	m_Sound = nullptr;
	PrimaryActorTick.bCanEverTick = true;

	GetStaticMeshComponent()->SetCollisionProfileName(FName("GrabbableTrace"));
	GetStaticMeshComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	GetStaticMeshComponent()->SetGenerateOverlapEvents(true);
	GetStaticMeshComponent()->SetSimulatePhysics(true);

	GetStaticMeshComponent()->OnComponentBeginOverlap.AddDynamic(this, &ABaseTrapActor::HandleOverlap);

	TrapDebuff = EDebuffs::DE_Nothing;

	SetReplicates(true);
	Tags.Add("Trap");
}

void ABaseTrapActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ABaseTrapActor::HandleOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//only do anything if the trap hasn't been triggered
	if (!bIsTriggered)
	{
		//don't hit ourselves or our owner
		if ((OtherActor != nullptr) && (OtherActor != this))
		{
			APlayerCharacter* owner = Cast<APlayerCharacter>(GetOwner());
			if (owner)
			{
				Team = owner->Team;
			}

			//Don't hit owner/teammates
			APlayerCharacter* pc = Cast<APlayerCharacter>(OtherActor);
			if (pc)
			{
				if (pc->Team == Team)
				{
					return;
				}
			}

			//cast to the actor as a basecharacter as those are all we can hit
			ABaseCharacter* dummy = Cast<ABaseCharacter>(OtherActor);
			if (dummy)
			{
				//Not all Traps have one
				if (ActivatedMesh)
				{
					//Required or the mesh can't be changed
					GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
					GetStaticMeshComponent()->SetStaticMesh(ActivatedMesh);
					GetStaticMeshComponent()->SetMobility(EComponentMobility::Static);
				}
				SetTarget(dummy);
				ApplyDebuff();
				bIsTriggered = true;
			}
		}
	}
}

//for now just destroy itself
void ABaseTrapActor::Die()
{
	Destroy();
}

void ABaseTrapActor::SetTarget(class ABaseCharacter* character)
{
	m_Target = character;
}

void ABaseTrapActor::Server_PlaySound_Implementation()
{
	NetMulticast_PlaySound();
}

bool ABaseTrapActor::Server_PlaySound_Validate()
{
	return true;
}

void ABaseTrapActor::NetMulticast_PlaySound_Implementation()
{
	if (m_Sound)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, m_Sound, GetActorLocation());
	}
}

void ABaseTrapActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseTrapActor, m_Sound);
	DOREPLIFETIME(ABaseTrapActor, Team);
}