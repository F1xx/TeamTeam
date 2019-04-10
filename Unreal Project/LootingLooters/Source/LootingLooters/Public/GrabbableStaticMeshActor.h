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

	//Timer handle for despawning the object after it is shattered.
	UPROPERTY(Replicated)
		FTimerHandle DespawnTimer;

	float TimeBeforeDespawn = 5.0f;

	//Bool for calculating damage to the mesh.
	UPROPERTY(Replicated)
		bool bWasThrown = false;

	//Health component. Used to track when to shatter the mesh.
	UPROPERTY(Replicated)
		class UHealthComponent* Health;

	//when the object is shattered (due to health)
	UFUNCTION()
		virtual void OnFracture(const FVector& HitPosition, const FVector& HitDirection);

	UFUNCTION()
		virtual void PostInitializeComponents() override;

	//Shatter sound effect.
	UPROPERTY(Replicated, EditDefaultsOnly)
		USoundWave* m_Sound;

public:
	virtual void Tick(float DeltaSeconds);

	UFUNCTION(Server, Reliable, WithValidation)
		virtual void BreakMesh(AActor* actor);

	//Multicast to break the mesh. Also spawns a sound for each client of the shatter.
	UFUNCTION(NetMulticast, Reliable)
		virtual void NetMulticast_BreakMesh(AActor* actor);

	virtual void Die();

//CONTROLLER METHOD START

	//Rotates the mesh in front of the player.
	UFUNCTION()
		void RotateX(float Value);
		void RotateY(float Value);

	//Zooms in/out the mesh from the player
	UFUNCTION()
		void Zoom(float Value);

	//Called by players when they interact.
	UFUNCTION()
		void Pickup(class ABaseCharacter* acharacter);

	//Called by players when they drop the mesh.
	UFUNCTION()
		void Drop();

	//Called by players when they throw the mesh.
	UFUNCTION()
		void Throw();

//CONTROLLER METHOD END
	
	//The character that owns us.
	UPROPERTY(Replicated)
		class ABaseCharacter* m_Character;

	FVector m_CamForward;
	FRotator m_Rotation;

	//Default distance the object hovers in front of the player.
	UPROPERTY(EditAnywhere)
	float m_Distance = 200.0f;

	

private:
	//Our mesh.
	UPROPERTY(Replicated)
		class UDestructibleComponent* DestructibleMesh;

	//Collision function for when this object is smashed by stuff.
	UFUNCTION()
		void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);
};
