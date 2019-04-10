// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LootActor.generated.h"

UCLASS()
class LOOTINGLOOTERS_API ALootActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALootActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Our timer to respawn the loot.
	UPROPERTY(Replicated)
		FTimerHandle RespawnTimer;

	//The particles our loot emits.
	UPROPERTY(EditDefaultsOnly, Category = "Effects", Replicated)
		class UParticleSystem* m_ParticleSystem;

	//The particle component
	UPROPERTY(VisibleAnywhere, Replicated)
		class UParticleSystemComponent* m_ParticleComponent;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void PostInitializeComponents() override;

	//remove the loot without actually deleting it.
	//unless bCanRespawn is false in which case it is Destroyed
	UFUNCTION(NetMulticast, Reliable)
		virtual void NetMulticast_Die();

	virtual void Die();

	//return the loot to the field
	virtual void Respawn();

	UPROPERTY(EditAnywhere, Category = "Collision", Replicated)
		class USphereComponent* Sphere;

	//a % chance. Setting above 100 will set it to 100
	UPROPERTY(EditAnywhere)
		int ChanceToFindTrap = 50;

	//How long before the item will respawn. 
	//Setting this to or lower than zero will cause the item to never respawn
	UPROPERTY(EditAnywhere)
		float RespawnDelay = 60;

	UPROPERTY(Replicated)
		bool isDead = false;
};
