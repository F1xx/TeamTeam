// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPlayerState.h"
#include "Net/UnrealNetwork.h"




AMyPlayerState::AMyPlayerState() :Super()
{

}

void AMyPlayerState::AddScore(int value)
{
	if (value < 0)
	{
		value = 0;
	}

	Score += value;
}

int AMyPlayerState::GetSlot(int index)
{
	if (index < 0 || index > 6)
		return -1;
	else
		return Slots[index];
}

void AMyPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyPlayerState, TrapCount);
	DOREPLIFETIME(AMyPlayerState, SelectedInventorySlot);
	DOREPLIFETIME(AMyPlayerState, Slots);
	DOREPLIFETIME(AMyPlayerState, Team);
}