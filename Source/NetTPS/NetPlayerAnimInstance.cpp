// Fill out your copyright notice in the Description page of Project Settings.


#include "NetPlayerAnimInstance.h"
#include "NetTPSCharacter.h"

void UNetPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// ���� �پ��ִ� Pawn ��������
	player = Cast<ANetTPSCharacter>(TryGetPawnOwner());
}

void UNetPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (player)
	{
		// ���� ������� ����
		bHasPistol = player->bHasPistol;

		// ��, �� ���⿡ ���� dirV �� ������.
		dirV = FVector::DotProduct(player->GetActorForwardVector(), player->GetVelocity());
		// ��, �� ���⿡ ���� dirH �� ������.
		dirH = FVector::DotProduct(player->GetActorRightVector(), player->GetVelocity());
	}
}
