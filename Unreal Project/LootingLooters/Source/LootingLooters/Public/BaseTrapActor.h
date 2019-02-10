// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "BaseTrapActor.generated.h"

/**
 * 
 */
UCLASS()
class LOOTINGLOOTERS_API ABaseTrapActor : public AStaticMeshActor
{
	GENERATED_BODY()

public:
		ABaseTrapActor();

		virtual void Tick(float DeltaSeconds) override;

		UFUNCTION()
			virtual void HandleOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
