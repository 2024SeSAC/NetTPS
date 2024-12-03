// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyUI.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Components/ScrollBox.h"
#include "NetGameInstance.h"
#include "SessionItem.h"

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

	// 세션 검색되면 호출되는 함수 등록
	gi->onAddSession.BindUObject(this, &ULobbyUI::OnAddSession);
	// 세션 완전 검색 완료되면 호출되는 함수 등록
	gi->onFindComplete.BindUObject(this, &ULobbyUI::OnFIndComplete);
}

void ULobbyUI::GoCreate()
{
	WidgetSwitcher->SetActiveWidgetIndex(1);
}

void ULobbyUI::GoFind()
{
	WidgetSwitcher->SetActiveWidgetIndex(2);

	// 강제로 세션 검색 하자
	FindSession();
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
	// 세션 목록 다 지우자
	scroll_SessionList->ClearChildren();

	gi->FindOtherSession();
}

void ULobbyUI::OnAddSession(int32 idx, FString info)
{
	USessionItem* item = CreateWidget<USessionItem>(GetWorld(), sessionItemFactory);
	scroll_SessionList->AddChild(item);
	item->SetInfo(idx, info);
}

void ULobbyUI::OnFIndComplete(bool isComplete)
{	
	// 검색 버튼 활성/비활성
	btn_FindSession->SetIsEnabled(isComplete);
}
