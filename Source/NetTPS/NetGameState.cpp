// Fill out your copyright notice in the Description page of Project Settings.


#include "NetGameState.h"
#include "GameUI.h"

void ANetGameState::BeginPlay()
{
	Super::BeginPlay();

	// GameUI 생성
	gameUI = CreateWidget<UGameUI>(GetWorld(), gameUIFactory);
	gameUI->AddToViewport();
}
