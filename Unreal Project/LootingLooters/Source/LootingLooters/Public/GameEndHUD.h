// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MenuSystem/MenuWidget.h"
#include "GameEndHUD.generated.h"

/**
 * 
 */
UCLASS()
class LOOTINGLOOTERS_API UGameEndHUD : public UMenuWidget
{
	GENERATED_BODY()
	
public:
	UGameEndHUD(const FObjectInitializer & ObjectInitializer);
	
	UFUNCTION(BlueprintCallable)
		void ShowScores();

private:
	UPROPERTY(meta = (BindWidget))
		class UPanelWidget* ScoreList;

	TSubclassOf<class UUserWidget> ServerRowClass;
	
};
