// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "NetTPSCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ANetTPSCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere)
	UInputAction* takeAction;

	UPROPERTY(EditAnywhere)
	UInputAction* fireAction;
	
	UPROPERTY(EditAnywhere)
	UInputAction* reloadAction;

public:
	ANetTPSCharacter();
	

protected:

	// MainUI 초기화
	void InitMainUIWidget();

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
			
	void TakePistol();
	void AttackPistol(AActor* pistol);
	void DetachPistol();

	void Fire();
	void Reload();

public:
	void ReloadFinish();

public:
	// 총이 붙어야 하는 컴포넌트
	UPROPERTY(EditAnywhere)
	USceneComponent* compGun;
	// 총 소유 여부
	bool bHasPistol = false;
	// 내가 잡고 있는 총 
	UPROPERTY()
	AActor* ownedPistol = nullptr;

	// 총을 잡을 수 있는 일정범위
	UPROPERTY(EditAnywhere)
	float distanceToGun = 200;

	// 카메라가 위치해야하는 곳
	FVector originCamPos;

	
	UPROPERTY(EditAnywhere)
	class UParticleSystem* gunEffect;

	UPROPERTY(EditAnywhere)
	class UAnimMontage* playerMontage;

	// MainUI 위젯 블루프린트 클래스 담을 변수
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UMainUI> mainUIWidget;
	// 만들어진 MainUI 를 담을 변수
	UPROPERTY()
	class UMainUI* mainUI;

	// 최대 총알 갯수
	UPROPERTY(EditAnywhere)
	float maxBulletCount = 10;
	// 현재 총알 갯수
	float currBulletCount = 0;

	// 현재 재장전 중인지 여부
	bool isReloading = false;


protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

	virtual void Tick(float DeltaSeconds) override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

