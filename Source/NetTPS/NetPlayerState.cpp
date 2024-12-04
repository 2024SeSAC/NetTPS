// Fill out your copyright notice in the Description page of Project Settings.


#include "NetPlayerState.h"
#include "NetGameState.h"
#include "GameUI.h"

ANetPlayerState::ANetPlayerState(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	NetUpdateFrequency = 100;
}

void ANetPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	// 점수 갱신을  GameUI 가 가지고 있는 PlayerStateUI 에게 알려주자. 
	onUpdateScore.ExecuteIfBound();
}


void ANetPlayerState::ServerRPC_SendChat_Implementation(const FString& chat)
{
	MulticastRPC_SendChat(chat);
}

void ANetPlayerState::MulticastRPC_SendChat_Implementation (const FString& chat)
{
	ANetGameState* gameState = GetWorld()->GetGameState<ANetGameState>();
	gameState->gameUI->AddChat(chat);
}