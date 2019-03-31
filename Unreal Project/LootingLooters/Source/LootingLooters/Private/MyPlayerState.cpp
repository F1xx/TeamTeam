// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPlayerState.h"
#include "Net/UnrealNetwork.h"




AMyPlayerState::AMyPlayerState() :Super()
{

}

void AMyPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyPlayerState, TrapCount);
	DOREPLIFETIME(AMyPlayerState, SelectedInventorySlot);
}