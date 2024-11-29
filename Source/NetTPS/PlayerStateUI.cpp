// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerStateUI.h"
#include "NetPlayerState.h"
#include "Components/TextBlock.h"

void UPlayerStateUI::Init(ANetPlayerState* ps)
{
	playerState = ps;
	// 점수 변경되었을 때 호출되는 함수 등록
	ps->onUpdateScore.BindUObject(this, &UPlayerStateUI::OnUpdateScore);

	UpdateName(ps->GetPlayerName());
	UpdateScore(ps->GetScore());
}

void UPlayerStateUI::UpdateName(FString name)
{
	text_Name->SetText(FText::FromString(name));
}

void UPlayerStateUI::UpdateScore(int32 score)
{
	text_Score->SetText(FText::AsNumber(score));
}

void UPlayerStateUI::OnUpdateScore()
{
	UpdateScore(playerState->GetScore());
}
