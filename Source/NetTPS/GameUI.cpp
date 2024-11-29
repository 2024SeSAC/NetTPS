// Fill out your copyright notice in the Description page of Project Settings.


#include "GameUI.h"
#include "GameFramework/GameState.h"
#include "GameFramework/PlayerState.h"
#include "Components/TextBlock.h"

class FPlayerStateSort
{
public:
	bool operator() (const APlayerState& a, const APlayerState& b) const
	{
		return (a.GetPlayerId() < b.GetPlayerId()); // 오름차순
	}
};

void UGameUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	// GameState 가져오자.
	AGameStateBase* gameState = GetWorld()->GetGameState();

	// 가져온 GameState 의 모든 Player 이름을 text_PlayerState 에 출력하자.
	FString allPlayerState;
	FString playerName;
	int32 playerId;
	int32 score;
	APlayerState* ps;

	// playerId 를 기준으로 정렬
	gameState->PlayerArray.Sort(FPlayerStateSort());

	for (int32 i = 0; i < gameState->PlayerArray.Num(); i++)
	{
		ps = gameState->PlayerArray[i];
		playerName = ps->GetPlayerName();
		playerId = ps->GetPlayerId();
		score = ps->GetScore();

		allPlayerState.Append(FString::Printf(TEXT("[%d] %s : %d\n"), playerId, *playerName, score));
	}
	text_PlayerState->SetText(FText::FromString(allPlayerState));
}
