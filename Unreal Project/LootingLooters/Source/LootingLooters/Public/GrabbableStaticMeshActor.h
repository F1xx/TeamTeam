// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DestructibleActor.h"
#include "DestructibleComponent.h"
#include "GrabbableStaticMeshActor.generated.h"

/**
 * 
 */
UCLASS()
class LOOTINGLOOTERS_API AGrabbableStaticMeshActor : public ADestructibleActor
{
	GENERATED_BODY()

		AGrabbableStaticMeshActor();

protected:
	virtual void BeginPlay();

public:
	virtual void Tick(float DeltaSeconds);

	UFUNCTION()
		void Pickup(class ABaseCharacter* acharacter);

	UFUNCTION()
		void Throw();

	UFUNCTION()
		void RotateX(float Value);
		void RotateY(float Value);

	UFUNCTION()
		void Zoom(float Value);

	bool bIsHeld = false;
	bool bIsGravityOn = true;
	
	class ABaseCharacter* m_Character;
	FVector m_CamForward;
	FRotator m_Rotation;

	UPROPERTY(EditAnywhere)
	float m_Distance = 200.0f;
};
