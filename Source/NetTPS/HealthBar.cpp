// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthBar.h"
#include "Components/ProgressBar.h"

void UHealthBar::NativeConstruct()
{
	Super::NativeConstruct();

	currHP = maxHP;
}


float UHealthBar::UpdateHPBar(float damage)
{
	// 현재HP 를 damage 만큼 줄여주자.
	currHP -= damage;

	// 현재 HP 를 0 ~ 1 사이의 값으로 바꾸자.
	float percent = currHP / maxHP;

	// ProgressBar 생신
	HPBar->SetPercent(percent);

	// 현재 HP 값 반환하자.
	return currHP;
}
