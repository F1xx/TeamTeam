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

	FTimerHandle DespawnTimer;
	float TimeBeforeDespawn = 5.0f;
	bool bWasThrown = false;
	class UHealthComponent* Health;

	UFUNCTION()
		virtual void OnFracture(const FVector& HitPosition, const FVector& HitDirection);

	UFUNCTION()
		virtual void PostInitializeComponents() override;

public:
	virtual void Tick(float DeltaSeconds);

	UFUNCTION()
		void Pickup(class ABaseCharacter* acharacter);

	UFUNCTION()
		void Drop();

	UFUNCTION()
		virtual void BreakMesh(AActor* actor);

	UFUNCTION()
		void Throw();

	UFUNCTION()
		void RotateX(float Value);
		void RotateY(float Value);

	UFUNCTION()
		void Zoom(float Value);
	
	class ABaseCharacter* m_Character;
	FVector m_CamForward;
	FRotator m_Rotation;

	UPROPERTY(EditAnywhere)
	float m_Distance = 200.0f;

	virtual void Die();

private:
	UPROPERTY(Replicated)
		class UDestructibleComponent* DestructibleMesh;

	UFUNCTION()
		void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);
};
