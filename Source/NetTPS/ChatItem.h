// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatItem.generated.h"

/**
 * 
 */
UCLASS()
class NETTPS_API UChatItem : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* content;

	void SetContent(FText s);
};
