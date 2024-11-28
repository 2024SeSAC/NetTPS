// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyUI.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "NetGameInstance.h"

void ULobbyUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 게임인스턴스 찾아오자
	gi = Cast<UNetGameInstance>(GetGameInstance());

	btn_Go_Create->OnClicked.AddDynamic(this, &ULobbyUI::GoCreate);
	btn_Go_Find->OnClicked.AddDynamic(this, &ULobbyUI::GoFind);
	btn_Create->OnClicked.AddDynamic(this, &ULobbyUI::CreateSession);
	btn_FindSession->OnClicked.AddDynamic(this, &ULobbyUI::FindSession);

	// slider 값이 변경되면 호출되는 함수 등록
	slider_PlayerCount->OnValueChanged.AddDynamic(this, &ULobbyUI::OnValueChanged);
}

void ULobbyUI::GoCreate()
{
	WidgetSwitcher->SetActiveWidgetIndex(1);
}

void ULobbyUI::GoFind()
{
	WidgetSwitcher->SetActiveWidgetIndex(2);
}

void ULobbyUI::CreateSession()
{
	// edit_DisplayName 에 값이 없으면 함수 나가자.
	if(edit_DisplayName->GetText().IsEmpty()) return;

	// 세션 생성
	FString displayName = edit_DisplayName->GetText().ToString();
	int32 playerCount = slider_PlayerCount->GetValue();
	gi->CreateMySession(displayName, playerCount);
}

void ULobbyUI::OnValueChanged(float value)
{
	// 최대 인원 Text 수정
	text_PlayerCount->SetText(FText::AsNumber(value));
}

void ULobbyUI::FindSession()
{
	gi->FindOtherSession();
}
