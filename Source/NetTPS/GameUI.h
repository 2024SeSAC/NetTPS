// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameUI.generated.h"

/**
 * 
 */
UCLASS()
class NETTPS_API UGameUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* text_PlayerState;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UPlayerStateUI> playerStateUIFactory;

	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* vBox_PlayerState;

	UPROPERTY()
	TArray<class UPlayerStateUI*> allPlayerState;

	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox* edit_chat;
	UPROPERTY(meta = (BindWidget))
	class UScrollBox* scroll_chat;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UChatItem> chatItemFactory;

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void AddPlayerStateUI(class APlayerState* ps);

	UFUNCTION()
	void OnTextBoxCommitted(const FText& Text, ETextCommit::Type CommitMethod);
};
