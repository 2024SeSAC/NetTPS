// Fill out your copyright notice in the Description page of Project Settings.


#include "NetGameState.h"
#include "GameUI.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

void ANetGameState::BeginPlay()
{
	Super::BeginPlay();
}

UGameUI* ANetGameState::GetGameUI()
{
	if (gameUI == nullptr)
	{
		// GameUI 생성
		gameUI = CreateWidget<UGameUI>(GetWorld(), gameUIFactory);
		gameUI->AddToViewport();
	}

	return gameUI;
}

void ANetGameState::ShowCursor(bool isShow)
{
	APlayerController* pc = GetWorld()->GetFirstPlayerController();
	pc->SetShowMouseCursor(isShow);

	if (isShow)
	{
		UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(pc);
	}
	else
	{
		UWidgetBlueprintLibrary::SetInputMode_GameOnly(pc);
	}
}



