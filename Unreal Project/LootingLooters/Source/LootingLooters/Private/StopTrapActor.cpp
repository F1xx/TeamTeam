// Fill out your copyright notice in the Description page of Project Settings.

#include "StopTrapActor.h"
#include "BaseCharacter.h"
#include "Sound/SoundWave.h"
#include "LootingLootersGameStateBase.h"
#include "Kismet/GameplayStatics.h"

AStopTrapActor::AStopTrapActor() : Super()
{
	TrapDebuff = EDebuffs::DE_Stop;
	DebuffLength = 5.0f;

	SetReplicates(true);
}

void AStopTrapActor::BeginPlay()
{
	m_Sound->bLooping = false;
}

//completely stops the character that stepped on it (sets its speed to 0)
void AStopTrapActor::ApplyDebuff()
{
	//Server call to play the shatter sound.
	if (HasAuthority())
		Server_PlaySound();

	//Timer delegate function
	FTimerDelegate del;
	del.BindUFunction(this, FName("RemoveDebuff"));

	//Set the debuff timer
	TargetOriginalSpeed = m_Target->GetMaxSpeed();
	m_Target->SetMaxSpeed(0.0f);
	GetWorld()->GetTimerManager().SetTimer(DebuffTime, del, DebuffLength, false);
}

//removes the target's debuf and then destroys itself
void AStopTrapActor::RemoveDebuff()
{
	m_Target->SetMaxSpeed(TargetOriginalSpeed);

	Die();
}