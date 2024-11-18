// Fill out your copyright notice in the Description page of Project Settings.


#include "NetPlayerAnimInstance.h"
#include "NetTPSCharacter.h"

void UNetPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// 내가 붙어있는 Pawn 가져오자
	player = Cast<ANetTPSCharacter>(TryGetPawnOwner());
}

void UNetPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (player)
	{
		// 총을 들었는지 여부
		bHasPistol = player->bHasPistol;

		// 죽었는 여부
		isDead = player->isDead;

		// 앞, 뒤 방향에 따른 dirV 값 구하자.
		dirV = FVector::DotProduct(player->GetActorForwardVector(), player->GetVelocity());
		// 좌, 우 방향에 따른 dirH 값 구하자.
		dirH = FVector::DotProduct(player->GetActorRightVector(), player->GetVelocity());
	}
}

void UNetPlayerAnimInstance::AnimNotify_OnReloadFinish()
{
	player->ReloadFinish();
}
