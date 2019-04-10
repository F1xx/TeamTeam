// Fill out your copyright notice in the Description page of Project Settings.

#include "SlowTrapActor.h"
#include "BaseCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Sound/SoundWave.h"
#include "LootingLootersGameStateBase.h"
#include "Kismet/GameplayStatics.h"

ASlowTrapActor::ASlowTrapActor() : Super()
{
	TrapDebuff = EDebuffs::DE_Slow;
	DebuffLength = 10.0f;

	SetReplicates(true);
}

void ASlowTrapActor::BeginPlay()
{
	m_Sound = Cast<ALootingLootersGameStateBase>(GetWorld()->GetGameState())->GetSoundWave("Slowed");
	m_Sound->bLooping = false;
}

//applies the slow debuff (lowering max walk speed) of the character that stepped on it
void ASlowTrapActor::ApplyDebuff()
{
	if (HasAuthority())
		Server_PlaySound();

	FTimerDelegate del;
	del.BindUFunction(this, FName("RemoveDebuff"));

	TargetOriginalSpeed = m_Target->GetMaxSpeed();
	m_Target->SetMaxSpeed(TargetOriginalSpeed * 0.2f);
	GetWorld()->GetTimerManager().SetTimer(DebuffTime, del, DebuffLength, false);
}

//removes the target's debuf and then destroys itself
void ASlowTrapActor::RemoveDebuff()
{
	m_Target->SetMaxSpeed(TargetOriginalSpeed);

	Die();
}