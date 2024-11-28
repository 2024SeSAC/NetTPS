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
	// 화면 전환 관련
	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher * WidgetSwitcher;
	UPROPERTY(meta = (BindWidget))
	class UButton* btn_Go_Create;
	UFUNCTION()
	void GoCreate();

	UPROPERTY(meta = (BindWidget))
	class UButton* btn_Go_Find;
	UFUNCTION()
	void GoFind();

	UPROPERTY()
	class UNetGameInstance* gi;

	// 세선 생성 관련
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

	// 세션 검색 관련
	UPROPERTY(meta = (BindWidget))
	class UButton* btn_FindSession;
	UPROPERTY(meta = (BindWidget))
	class UScrollBox* scroll_SessionList;
	UPROPERTY(EditAnywhere)
	TSubclassOf<class USessionItem> sessionItemFactory;
	UFUNCTION()
	void FindSession();
	UFUNCTION()
	void OnAddSession(FString info);
};
