// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SessionItem.generated.h"

/**
 * 
 */
UCLASS()
class NETTPS_API USessionItem : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	UPROPERTY(meta=(BindWidget))
	class UButton* btn_Join;
	UFUNCTION()
	void Join();

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* text_SessionName;

	int32 selectIdx;

	void SetInfo(int32 idx, FString info);
};
