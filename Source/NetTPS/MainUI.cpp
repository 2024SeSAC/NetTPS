// Fill out your copyright notice in the Description page of Project Settings.


#include "MainUI.h"
#include "Components/Image.h"
#include "Components/HorizontalBox.h"
#include "Components/Button.h"
#include "NetPlayerController.h"

void UMainUI::NativeConstruct()
{
	Super::NativeConstruct();

	// crosshair image 위젯 가져오자.
	imgCrosshair = Cast<UImage>(GetWidgetFromName(TEXT("crossHair")));
	ShowCrosshair(false);

	// bulletPanel 위젯 가져오자.
	bulletMagazine = Cast<UHorizontalBox>(GetWidgetFromName(TEXT("bulletPanel")));

	// btnRetry 안보이게
	ShowBtnRetry(false);

	btnRetry->OnClicked.AddDynamic(this, &UMainUI::OnRetry);
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

void UMainUI::ShowBtnRetry(bool isShow)
{
	if (isShow)
	{
		btnRetry->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		btnRetry->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UMainUI::OnRetry()
{
	// 마우스 커서 안보이게
	ANetPlayerController* pc = Cast<ANetPlayerController>(GetWorld()->GetFirstPlayerController());
	pc->SetShowMouseCursor(false);
	// 관찰자 모드로 전환	
	pc->ServerRPC_ChageToSpectator();

	// MainUI 를 삭제
	RemoveFromParent();
}

void UMainUI::AddBulet()
{
	UUserWidget* bullet =  CreateWidget(GetWorld(), bulletFactory);
	// 만들어진 bullet 을 bulletMagazine 에 추가하자.
	bulletMagazine->AddChild(bullet);
}

void UMainUI::PopBullet(int32 index)
{
	bulletMagazine->RemoveChildAt(index);
}

void UMainUI::PopBulletAll()
{
	int32 remainBullet = bulletMagazine->GetChildrenCount();

	for (int32 i = remainBullet - 1; i >= 0; i--)
	{
		PopBullet(i);
	}
}
