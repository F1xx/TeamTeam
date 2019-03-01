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

	GetStaticMeshComponent()->OnComponentBeginOverlap.AddDynamic(this, &ABaseTrapActor::HandleOverlap);

// 	FString LootFilePath;
// #ifdef UE_BUILD_RELEASE
// 	LootFilePath += "Blueprint'";
// #endif
// 	FString NameTag = "TRAP_Bear_Open";
// 	FString NameTagTwo = "TRAP_Bear_Closed";
// 	LootFilePath += "/Game/Assets/TrapMeshes/";
// 
// 	FString BearFile = LootFilePath + NameTag;
// 	FString OtherBearFile = LootFilePath + NameTagTwo;
// 
// #ifdef UE_BUILD_RELEASE
// 	BearFile += "." + NameTag + "'";
// 	OtherBearFile += "." + NameTagTwo + "'";
// #endif

	//DefaultMesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(*BearFile).Object;
	//ActivatedMesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(*OtherBearFile).Object;

	//GetStaticMeshComponent()->SetStaticMesh(DefaultMesh);

	TrapDebuff = EDebuffs::DE_Nothing;
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
		if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr) && (OtherActor != GetOwner()))
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Emerald, FString("Someone Stepped on a TRAP!!!!!!!!!"));//do something later

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

		//REMOVE this entire if so we can't hit our own trap
		//if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr) && (OtherActor == GetOwner()))
		//{
		//	//Nothing actually should happen here. This is just so we know it knows it's owner
		//	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Emerald, FString("My Owner Stepped on their own TRAP!!!!!!!!"));//do something later
		//	ABaseCharacter* dummy = Cast<ABaseCharacter>(OtherActor);
		//	if (dummy)
		//	{
		//		//Not all Traps have one
		//		if (ActivatedMesh)
		//		{
		//			GetStaticMeshComponent()->SetStaticMesh(ActivatedMesh);
		//		}
		//		SetTarget(dummy);
		//		ApplyDebuff();
		//		bIsTriggered = true;
		//	}
		//}
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