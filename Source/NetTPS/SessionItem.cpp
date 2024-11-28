// Fill out your copyright notice in the Description page of Project Settings.


#include "SessionItem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "NetGameInstance.h"

void USessionItem::NativeConstruct()
{
	Super::NativeConstruct();

	btn_Join->OnClicked.AddDynamic(this, &USessionItem::Join);
}

void USessionItem::Join()
{
	UNetGameInstance* gi = Cast<UNetGameInstance>(GetGameInstance());
	gi->JoinOtherSession(selectIdx);
}

void USessionItem::SetInfo(int32 idx, FString info)
{
	selectIdx = idx;
	text_SessionName->SetText(FText::FromString(info));
}
