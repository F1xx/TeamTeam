
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "DoorActor.generated.h"

/**
 * 
 */
UCLASS()
class LOOTINGLOOTERS_API ADoorActor : public AStaticMeshActor
{
	GENERATED_BODY()

protected:

	virtual void PostInitializeComponents() override;

public:
	ADoorActor();

	//Collision to trigger the teleport event from 1 door to another.
	UPROPERTY(EditAnywhere)
		class USphereComponent* Sphere;
	
	//The door that this door connects to. If unset will cause nothing to happen.
	UPROPERTY(EditAnywhere)
		ADoorActor* Connector;

	//Our arrow. Points in the direction the player will be ejected to by the Connector.
	UPROPERTY(EditAnywhere)
		class UArrowComponent* ArrowComponent;

	//Does this door have a connection?
	bool IsConnected();

	//Connect a door to this door.
	void ApplyConnection(ADoorActor* OtherDoor);

	//Event trigger to teleport pawns to connected doors.
	UFUNCTION()
		void TeleportPawnToOtherDoor(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

private:

	//a small offset to make sure teleporting isnt an infinite loop.
	const float TELEPORT_DISTANCE_FROM_DOOR = 100.0f;
};
