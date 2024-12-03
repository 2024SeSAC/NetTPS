// Fill out your copyright notice in the Description page of Project Settings.


#include "GameUI.h"
#include "GameFramework/GameState.h"
#include "GameFramework/PlayerState.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "NetPlayerState.h"
#include "PlayerStateUI.h"

class FPlayerStateSort
{
public:
	bool operator() (const APlayerState& a, const APlayerState& b) const
	{
		return (a.GetPlayerId() < b.GetPlayerId()); // 오름차순
	}
};

class FPlayerStateUISort
{
public:
	bool operator() (const UPlayerStateUI& a, const UPlayerStateUI& b) const
	{
		return (a.playerState->GetPlayerId() < b.playerState->GetPlayerId()); // 오름차순
	}
};

void UGameUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	//// GameState 가져오자.
	//AGameStateBase* gameState = GetWorld()->GetGameState();

	//// 가져온 GameState 의 모든 Player 이름을 text_PlayerState 에 출력하자.
	//FString allPlayerState;
	//FString playerName;
	//int32 playerId;
	//int32 score;
	//APlayerState* ps;

	//// playerId 를 기준으로 정렬
	//gameState->PlayerArray.Sort(FPlayerStateSort());

	//for (int32 i = 0; i < gameState->PlayerArray.Num(); i++)
	//{
	//	ps = gameState->PlayerArray[i];
	//	playerName = ps->GetPlayerName();
	//	playerId = ps->GetPlayerId();
	//	score = ps->GetScore();

	//	allPlayerState.Append(FString::Printf(TEXT("[%d] %s : %d\n"), playerId, *playerName, score));
	//}
	//text_PlayerState->SetText(FText::FromString(allPlayerState));
}

void UGameUI::AddPlayerStateUI(APlayerState* ps)
{
	if(ps == nullptr) return;

	// 만약에 ps 가 이미 추가 되어있다면 함수를 나가자.
	bool isExist = allPlayerState.ContainsByPredicate([ps](UPlayerStateUI* p) {
		return p->playerState == ps;
	});
	if(isExist) return;

	UPlayerStateUI* psUI = CreateWidget<UPlayerStateUI>(GetWorld(), playerStateUIFactory);
	psUI->Init(Cast<ANetPlayerState>(ps));
	// 만들어지 psUI 를 나만의 Array 추가하자.
	allPlayerState.Add(psUI);
	// allPlayerState 를 PlayerId 기준으로 정렬
	allPlayerState.Sort(FPlayerStateUISort());
	
	// vBox_PlayerState 자식들을 Clear 하자
	vBox_PlayerState->ClearChildren();
	// allPlayerState 순서대로 추가하자.
	for (int32 i = 0; i < allPlayerState.Num(); i++)
	{
		vBox_PlayerState->AddChild(allPlayerState[i]);
		// Panel 에 붙히고 난 후 Slot 이 생성된다!! 
		// Vertical Box Slot 을 가져오자.
		UVerticalBoxSlot* slot = Cast<UVerticalBoxSlot>(allPlayerState[i]->Slot);
		// Horizontal Alignment 을 오른쪽을 하자.
		slot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Right);
		slot->SetPadding(FMargin(0, 0, 20, 0));
	}	
}
