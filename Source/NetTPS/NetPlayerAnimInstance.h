// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "NetPlayerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class NETTPS_API UNetPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:
	// 총을 들고 있는지 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasPistol = false;	

	// dirH, dirV
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float dirH = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float dirV = 0;

	// 죽었는지 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool isDead = false;

	UPROPERTY()
	class ANetTPSCharacter* player;

public:
	// Reload 애니메이션 끝나는 부분에 호출되는 함수
	UFUNCTION()
	void AnimNotify_OnReloadFinish();
};
