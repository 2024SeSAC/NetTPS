// Fill out your copyright notice in the Description page of Project Settings.


#include "MainUI.h"
#include "Components/Image.h"
#include "Components/HorizontalBox.h"

void UMainUI::NativeConstruct()
{
	Super::NativeConstruct();

	// crosshair image ���� ��������.
	imgCrosshair = Cast<UImage>(GetWidgetFromName(TEXT("crossHair")));
	ShowCrosshair(false);

	// bulletPanel ���� ��������.
	bulletMagazine = Cast<UHorizontalBox>(GetWidgetFromName(TEXT("bulletPanel")));
}

void UMainUI::ShowCrosshair(bool isShow)
{
	if (isShow)
	{
		imgCrosshair->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		imgCrosshair->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UMainUI::AddBulet()
{
	UUserWidget* bullet =  CreateWidget(GetWorld(), bulletFactory);
	// ������� bullet �� bulletMagazine �� �߰�����.
	bulletMagazine->AddChild(bullet);
}

void UMainUI::PopBullet(int32 index)
{
	bulletMagazine->RemoveChildAt(index);
}
