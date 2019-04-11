// Fill out your copyright notice in the Description page of Project Settings.

#include "GameEndHUD.h"
#include "Kismet/GameplayStatics.h"
#include "MenuSystem/ServerRow.h"
#include "Components/TextBlock.h"
#include "PanelWidget.h"

//NOT ACTUALLY USED

UGameEndHUD::UGameEndHUD(const FObjectInitializer & ObjectInitializer)
{
	ConstructorHelpers::FClassFinder<UUserWidget> ServerRowBPClass(TEXT("/GameFrwkSessionsPlugin/MenuSystem/WBP_ServerRow"));
	if (!ensure(ServerRowBPClass.Class != nullptr)) return;

	ServerRowClass = ServerRowBPClass.Class;
}

void UGameEndHUD::ShowScores()
{
	UWorld* World = this->GetWorld();
	if (!ensure(World != nullptr)) return;

	TArray<AActor*> controllers;
	UGameplayStatics::GetAllActorsOfClass(World, APlayerController::StaticClass(), controllers);

	uint32 i = 1;
	for (auto a : controllers)
	{
		APlayerController* pc = Cast<APlayerController>(a);

		UServerRow* Row = CreateWidget<UServerRow>(World, ServerRowClass);
		if (!ensure(Row != nullptr)) return;

		Row->ServerName->SetText(FText::FromString("Team " + FString::SanitizeFloat(i) + ": "));
		Row->HostUser->SetText(FText::FromString(FString::SanitizeFloat(pc->PlayerState->Score)));
		Row->ConnectionFraction->SetVisibility(ESlateVisibility::Hidden);
		++i;

		ScoreList->AddChild(Row);
	}
}
