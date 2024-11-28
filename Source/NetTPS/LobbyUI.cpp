// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyUI.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "NetGameInstance.h"

void ULobbyUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 게임인스턴스 찾아오자
	gi = Cast<UNetGameInstance>(GetGameInstance());

	btn_Create->OnClicked.AddDynamic(this, &ULobbyUI::CreateSession);

	// slider 값이 변경되면 호출되는 함수 등록
	slider_PlayerCount->OnValueChanged.AddDynamic(this, &ULobbyUI::OnValueChanged);
}

void ULobbyUI::CreateSession()
{
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
