// Fill out your copyright notice in the Description page of Project Settings.


#include "SessionItem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void USessionItem::NativeConstruct()
{
	Super::NativeConstruct();

	btn_Join->OnClicked.AddDynamic(this, &USessionItem::Join);
}

void USessionItem::Join()
{
	
}

void USessionItem::SetInfo(FString info)
{
	text_SessionName->SetText(FText::FromString(info));
}
