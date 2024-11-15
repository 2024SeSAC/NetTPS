// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainUI.generated.h"

/**
 * 
 */
UCLASS()
class NETTPS_API UMainUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	class UImage* imgCrosshair;

	UPROPERTY()
	class UHorizontalBox* bulletMagazine;

	// �Ѿ� ���� Blueprint
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> bulletFactory;

public:
	virtual void NativeConstruct() override;

	void ShowCrosshair(bool isShow);

	// �Ѿ� UI �߰� �Լ�
	void AddBulet();

	// �Ѿ� UI ���� �Լ�
	void PopBullet(int32 index);
};
