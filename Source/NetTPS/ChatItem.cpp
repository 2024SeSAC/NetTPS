// Fill out your copyright notice in the Description page of Project Settings.


#include "ChatItem.h"
#include "Components/TextBlock.h"

void UChatItem::SetContent(FText s)
{
	content->SetText(s);
}
