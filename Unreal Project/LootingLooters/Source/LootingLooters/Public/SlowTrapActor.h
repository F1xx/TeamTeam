// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseTrapActor.h"
#include "SlowTrapActor.generated.h"

/**
 * 
 */
UCLASS()
class LOOTINGLOOTERS_API ASlowTrapActor : public ABaseTrapActor
{
	GENERATED_BODY()
	
public:
	ASlowTrapActor();
	
	
protected:

	float TargetOriginalSpeed;

	virtual void ApplyDebuff() override;
	virtual void RemoveDebuff() override;

	virtual void BeginPlay() override;
};
