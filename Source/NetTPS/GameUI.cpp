// Fill out your copyright notice in the Description page of Project Settings.


#include "GameUI.h"
#include "GameFramework/GameState.h"
#include "GameFramework/PlayerState.h"
#include "Components/TextBlock.h"

void UGameUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	// GameState 가져오자.
	AGameStateBase* gameState = GetWorld()->GetGameState();

	// 가져온 GameState 의 모든 Player 이름을 text_PlayerState 에 출력하자.
	FString allUserName;
	for (int32 i = 0; i < gameState->PlayerArray.Num(); i++)
	{
		allUserName.Append(gameState->PlayerArray[i]->GetPlayerName() + TEXT("\n"));
	}
	text_PlayerState->SetText(FText::FromString(allUserName));
}
