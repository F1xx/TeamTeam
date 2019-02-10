// Fill out your copyright notice in the Description page of Project Settings.

#include "StopTrapActor.h"
#include "BaseCharacter.h"

AStopTrapActor::AStopTrapActor() : Super()
{
	TrapDebuff = EDebuffs::DE_Stop;
	DebuffLength = 5.0f;
}

void AStopTrapActor::ApplyDebuff()
{
	FTimerDelegate del;
	del.BindUFunction(this, FName("RemoveDebuff"));

	TargetOriginalSpeed = m_Target->GetMaxSpeed();
	m_Target->SetMaxSpeed(0.0f);
	GetWorld()->GetTimerManager().SetTimer(DebuffTime, del, DebuffLength, false);
}

void AStopTrapActor::RemoveDebuff()
{
	m_Target->SetMaxSpeed(TargetOriginalSpeed);

	Die();
}
