// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerStateUI.generated.h"

/**
 * 
 */
UCLASS()
class NETTPS_API UPlayerStateUI : public UUserWidget
{
	GENERATED_BODY()

public:
	void Init(class ANetPlayerState* ps);	
	void UpdateName(FString name);
	void UpdateScore(int32 score);
	UFUNCTION()
	void OnUpdateScore();

public:
	UPROPERTY()
	class ANetPlayerState* playerState;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* text_Name;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* text_Score;
};
