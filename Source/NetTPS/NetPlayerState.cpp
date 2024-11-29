// Fill out your copyright notice in the Description page of Project Settings.


#include "NetPlayerState.h"

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
