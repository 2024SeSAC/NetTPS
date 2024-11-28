// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyUI.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/Slider.h"
#include "NetGameInstance.h"

void ULobbyUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 게임인스턴스 찾아오자
	gi = Cast<UNetGameInstance>(GetGameInstance());

	btn_Create->OnClicked.AddDynamic(this, &ULobbyUI::CreateSession);
}

void ULobbyUI::CreateSession()
{
	// 세션 생성
	FString displayName = edit_DisplayName->GetText().ToString();
	int32 playerCount = slider_PlayerCount->GetValue();
	gi->CreateMySession(displayName, playerCount);
}
