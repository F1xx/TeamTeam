// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseTrapActor.h"
#include "StopTrapActor.generated.h"

/**
 * 
 */
UCLASS()
class LOOTINGLOOTERS_API AStopTrapActor : public ABaseTrapActor
{
	GENERATED_BODY()

public:
	AStopTrapActor();

protected:

	float TargetOriginalSpeed;

	virtual void BeginPlay() override;

	virtual void ApplyDebuff() override;
	virtual void RemoveDebuff() override;
};
