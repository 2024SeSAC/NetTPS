// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyUI.generated.h"

/**
 * 
 */
UCLASS()
class NETTPS_API ULobbyUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
public:
	UPROPERTY()
	class UNetGameInstance* gi;

	UPROPERTY(meta = (BindWidget))
	class UButton* btn_Create;
	UFUNCTION()
	void CreateSession();

	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox* edit_DisplayName;

	UPROPERTY(meta = (BindWidget))
	class USlider* slider_PlayerCount;
	UFUNCTION()
	void OnValueChanged(float value);

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* text_PlayerCount;
};
