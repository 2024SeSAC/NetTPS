// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetTPSCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include <Kismet/GameplayStatics.h>
#include "Components/WidgetComponent.h"
#include "MainUI.h"
#include "Pistol.h"
#include "HealthBar.h"
#include <Kismet/KismetMathLibrary.h>
#include <Net/UnrealNetwork.h>
#include "NetTPSGameMode.h"
#include "GameFramework/PlayerState.h"
#include "NetGameState.h"
#include "GameUI.h"


DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ANetTPSCharacter


void ANetTPSCharacter::MakeCube()
{
	// 내 차례가 아니면 함수나가자
	if(canMakeCube == false) return;

	// 서버에게 큐브 만든다라고 알려주자.
	ServerRPC_MakeCube();
}

void ANetTPSCharacter::ServerRPC_MakeCube_Implementation()
{
	// 턴넘기자
	// 게임 모드 가져오자 (ANetTPSGameMode 형변환)
	ANetTPSGameMode* gm = Cast<ANetTPSGameMode>(GetWorld()->GetAuthGameMode());
	gm->ChangeTurn();

	FVector pos = GetActorLocation() + GetActorForwardVector() * 100;
	MulticastRPC_MakeCube(pos, GetActorRotation());
}

void ANetTPSCharacter::MulticastRPC_MakeCube_Implementation(FVector pos, FRotator rot)
{
	// 큐브 생성하자
	AActor* cube = GetWorld()->SpawnActor<AActor>(cubeFactory);
	// 큐브의 위치를 나의 앞방향으로 100만큼 떨어진 위치에 놓자.	
	cube->SetActorLocation(pos);
	// 큐브의 회전값을 나의 회전값으로 설정
	cube->SetActorRotation(rot);
}

ANetTPSCharacter::ANetTPSCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetRelativeLocation(FVector(0, 0, 60));
	CameraBoom->TargetArmLength = 300; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	//  Gun component 
	compGun = CreateDefaultSubobject<USceneComponent>(TEXT("GUN"));
	compGun->SetupAttachment(GetMesh(), TEXT("gunPosition"));
	compGun->SetRelativeLocation(FVector(-7.144f, 3.68f, 4.136f));
	compGun->SetRelativeRotation(FRotator(3.4f, 75.699f, 6.642f));

	// HP component
	compHP = CreateDefaultSubobject<UWidgetComponent>(TEXT("HP"));
	compHP->SetupAttachment(RootComponent);
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ANetTPSCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
	
	FString isServer = HasAuthority() ? TEXT("서버") : TEXT("클라");
	UE_LOG(LogTemp, Warning, TEXT("%s - BeginPlay : %s"), *isServer, *GetActorNameOrLabel());

	// originCamPos 를 초기의 CameraBoom 값으로 설정
	originCamPos = CameraBoom->GetRelativeLocation();

	// 서버라면
	if (HasAuthority())
	{
		// 게임 모드 가져오자 (ANetTPSGameMode 형변환)
		ANetTPSGameMode* gm = Cast<ANetTPSGameMode>(GetWorld()->GetAuthGameMode());
		// 나를 추가 시키자
		if (gm)
		{
			gm->AddPlayer(this);
		}
		// 서버이면서 내 캐릭터라면
		if (IsLocallyControlled())
		{
			canMakeCube = true;
		}
	}	
}

void ANetTPSCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// CameraBoom 이 originCamPos 를 향해서 계속 움직이자.
	FVector pos = FMath::Lerp(CameraBoom->GetRelativeLocation(), originCamPos, DeltaSeconds * 10);
	CameraBoom->SetRelativeLocation(pos);

	BillboardHP();

	//PrintNetLog();
}

void ANetTPSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetTPSCharacter, canMakeCube);
	DOREPLIFETIME(ANetTPSCharacter, ownedPistol);
}

// PlayerController 에 Possess 가 될 때 호출되는 함수
// 해당 함수는 서버에서만 호출이 된다.
void ANetTPSCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	UE_LOG(LogTemp, Warning, TEXT("PossessedBy : %s"), *GetActorNameOrLabel());

	ClientRPC_Init();
}

void ANetTPSCharacter::OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState)
{
	Super::OnPlayerStateChanged(NewPlayerState, OldPlayerState);

	// GameState 가져오고
	ANetGameState* gameState = Cast<ANetGameState>(GetWorld()->GetGameState());
	// GameUI 가져와서 PlayerStateUI 하나 만들어주세요
	gameState->GetGameUI()->AddPlayerStateUI(NewPlayerState);
}

void ANetTPSCharacter::PrintNetLog()
{
	// 연결상태
	FString connStr = GetNetConnection() != nullptr ? TEXT("Valid Connection") : TEXT("Invalid Connection");

	// Owner 
	FString ownerStr = GetOwner() != nullptr ? GetOwner()->GetName() : TEXT("No Owner");

	// 권한
	FString role = UEnum::GetValueAsString<ENetRole>(GetLocalRole());

	// 내것인지
	FString isMine = IsLocallyControlled() ? TEXT("내 것") : TEXT("남의 것");

	FString logStr = FString::Printf(TEXT("Connection : %s\nOwner : %s\nrole : %s\nmine : %s"), 
					*connStr, 
					*ownerStr,
					*role,
					*isMine);

	DrawDebugString(GetWorld(), GetActorLocation(), logStr, nullptr, FColor::Yellow, 0, true, 1);
}

//////////////////////////////////////////////////////////////////////////
// Input

void ANetTPSCharacter::DamageProcess(float damage)
{
	// HBbar 를 갱신
	UHealthBar* hpBar = nullptr;
	// 내것이라면 MainUI HealthBar 를 갱신
	if (IsLocallyControlled())
	{
		hpBar = mainUI->HealthBar;
	}
	// 남의것이라면 
	else
	{
		hpBar = Cast<UHealthBar>(compHP->GetWidget());		
	}	

	float currHP = hpBar->UpdateHPBar(damage);
	if (currHP <= 0)
	{
		// 죽음 처리
		isDead = true;

		// 움직이지 못하게 하자.
		GetCharacterMovement()->DisableMovement();
		
		// 충돌안되게 하자.
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);		


		// 만약에 총들고 있고 내 캐릭터라면 총을 떨구자 
		if (bHasPistol && IsLocallyControlled())
		{
			TakePistol();
		}
	}
}

void ANetTPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ANetTPSCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ANetTPSCharacter::Look);
		
		// F 키 눌렀을 때 호출되는 함수 등록
		EnhancedInputComponent->BindAction(takeAction, ETriggerEvent::Started, this, &ANetTPSCharacter::TakePistol);
		
		// 마우스 왼쪽 버튼 눌렀을 때 호출되는 함수 등록
		EnhancedInputComponent->BindAction(fireAction, ETriggerEvent::Started, this, &ANetTPSCharacter::Fire);

		// R 키 눌렀을 때 호출되는 함수 등록
		EnhancedInputComponent->BindAction(reloadAction, ETriggerEvent::Started, this, &ANetTPSCharacter::Reload);

		//EnhancedInputComponent->BindAction(makeCubeAction, ETriggerEvent::Started, this, &ANetTPSCharacter::MakeCube);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ANetTPSCharacter::InitMainUIWidget()
{
	// 만약에 내 캐릭터가 아니라면 함수를 나가자.
	if(IsLocallyControlled() == false) return;

	mainUI = Cast<UMainUI>( CreateWidget(GetWorld(), mainUIWidget) );
	mainUI->AddToViewport();

	// HPBar 컴포넌트 안보이게하자
	compHP->SetVisibility(false);

	//// 총알 UI 생성
	//currBulletCount = maxBulletCount;
	//for (int i = 0; i < currBulletCount; i++)
	//{
	//	mainUI->AddBulet();
	//}
}

void ANetTPSCharacter::ClientRPC_Init_Implementation()
{
	InitMainUIWidget();
}

void ANetTPSCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ANetTPSCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ANetTPSCharacter::ServerRPC_TakePistol_Implementation()
{
	// 총을 소유하고 있지 않다면 일정범위 안에 있는 총을 잡는다.
	// 1. 총을 잡고 있지 않다면
	if (bHasPistol == false)
	{
		// 2. 월드에 있는 총을 모두 찾는다.
		TArray<AActor*> allActors;
		TArray<APistol*> pistolActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APistol::StaticClass(), allActors);
		for (int32 i = 0; i < allActors.Num(); i++)
		{
			pistolActors.Add(Cast<APistol>(allActors[i]));
		}

		// 나와 총의 최단거리
		float closestDist = std::numeric_limits<float>::max();
		APistol* closestPistol = nullptr;

		for (APistol* pistol : pistolActors)
		{
			// 만약에 pistol 의 소유자가 없다면
			if (pistol->GetOwner() == nullptr)
			{
				// 3. 총과의 거리를 구하자.
				float dist = FVector::Distance(pistol->GetActorLocation(), GetActorLocation());
				// 4. 만약에 거리가 일정범위 안에 있다면
				if (dist < distanceToGun)
				{
					// closestDist 값보다 dist 값이 크다면 (더 가깝다는 의미)
					if (closestDist > dist)
					{
						// 최단 거리 갱신
						closestDist = dist;
						closestPistol = pistol;
					}
				}
			}
		}

		if (closestPistol)
		{
			ownedPistol = closestPistol;
			// Owner 설정
			ownedPistol->SetOwner(this);
			// 서버에서 총을 붙여라
			AttackPistol();

			// 모든 클라이언트 들에게  총을 붙혀라!!
			//MulticastRPC_AttachPistol(closestPistol);
		}
	}
	// 총을 잡고 있다면
	else
	{
		APistol* pistol = ownedPistol;
		// Onwer 설정
		ownedPistol->SetOwner(nullptr);
		ownedPistol = nullptr;
		// 모든 클라이언트에게 총을 버려라!!

		MulticastRPC_DetachPistol(pistol);
	}
}

void ANetTPSCharacter::TakePistol()
{
	ServerRPC_TakePistol();
}


void ANetTPSCharacter::AttackPistol()
{
	if(ownedPistol == nullptr) return;

	bHasPistol = true;

	// pistol 이 가지고 있는 StaticMesh 컴포넌트 가져오자
	UStaticMeshComponent* comp = ownedPistol->GetComponentByClass<UStaticMeshComponent>();
	// 가져온 컴포넌트를 이용해서 SimulatePhysisc 비활성화
	comp->SetSimulatePhysics(false);
	// compGun 에 붙히자.
	ownedPistol->AttachToComponent(compGun, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	if (IsLocallyControlled())
	{
		// 총 들었을 때 캐릭터 회전 기능 변경 (카메라에 의해서 변경되도록)		
		CameraBoom->TargetArmLength = 150;
		originCamPos = FVector(0, 40, 60);	

		// ownedPistol 의 현재 총알 갯수만큼 총알 UI 를 채우자.
		InitBulletUI();
	}
}

void ANetTPSCharacter::MulticastRPC_DetachPistol_Implementation(APistol* pistol)
{
	DetachPistol(pistol);
}

void ANetTPSCharacter::DetachPistol(APistol* pistol)
{
	// pistol 이 가지고 있는 StaticMesh 컴포넌트 가져오자
	UStaticMeshComponent* comp = pistol->GetComponentByClass<UStaticMeshComponent>();
	// 가져온 컴포넌트를 이용해서 SimulatePhysisc 활성화
	comp->SetSimulatePhysics(true);
	// 총을 gunPosition 에서 분리하자.
	pistol->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);	

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	if (IsLocallyControlled())
	{
		// 총 놨을때 캐릭터 회전 기능 변경 (카메라와 독립)		
		CameraBoom->TargetArmLength = 300;
		originCamPos = FVector(0, 0, 60);

		// crosshair UI 보이지 않게 하자.
		mainUI->ShowCrosshair(false);

		// 총알 UI 지우자.
		mainUI->PopBulletAll();
	}

	bHasPistol = false;
}

void ANetTPSCharacter::ServerRPC_Fire_Implementation(bool bHit, FHitResult hitInfo)
{
	// 만약에 맞은 Actor 가 Player 라면
	ANetTPSCharacter* player = Cast<ANetTPSCharacter>(hitInfo.GetActor());
	if (player)
	{
		APlayerState* ps = GetPlayerState();
		ps->SetScore(ps->GetScore() + 1);
	}
	
	// 모든 클라에게 파티클 나오게 해라!
	MulticastRPC_Fire(bHit, hitInfo);
}

void ANetTPSCharacter::MulticastRPC_Fire_Implementation(bool bHit, FHitResult hitInfo)
{
	if (bHit)
	{
		// 맞은 위치에 파티클로 표시 하자.
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), gunEffect, hitInfo.Location, FRotator(), true);

		// 만약에 맞은 Actor 가 Player 라면
		ANetTPSCharacter* player = Cast<ANetTPSCharacter>(hitInfo.GetActor());
		if (player)
		{
			// 해당 Player 가 가지고 있는 DamageProcess 함수 실행
			player->DamageProcess(ownedPistol->weaponDamage);
		}
	}
	
	// 총알 제거
	ownedPistol->currBulletCount--;
	if (IsLocallyControlled())
	{
		mainUI->PopBullet(ownedPistol->currBulletCount);

	}

	// 총쏘는 애니메이션 실행하자 (Montage 실행)
	PlayAnimMontage(playerMontage, 2, TEXT("Fire"));
}

void ANetTPSCharacter::Fire()
{
	// 만약에 총을 들고 있지 않다면 함수를 나가자
	if(bHasPistol == false) return;

	// 현재 총알 갯수가 0보다 작거나 같으면 함수를 나가자
	if(ownedPistol->currBulletCount <= 0) return;
	
	// 재장전 중이면 함수를 나가자.
	if(isReloading) return;
	
	// LineTrace 로 부딪힌 곳 찾아내자.
	FVector startPos = FollowCamera->GetComponentLocation();
	FVector endPos = startPos + FollowCamera->GetForwardVector() * 100000;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);

	FHitResult hitInfo;
	bool bHit = GetWorld()->LineTraceSingleByChannel(hitInfo, startPos, endPos, ECollisionChannel::ECC_Visibility, params);


	// 서버에게 파티클 나오게 요청!
	ServerRPC_Fire(bHit, hitInfo);		
}

void ANetTPSCharacter::ServerRPC_Reload_Implementation()
{
	// 총을 가지고 있지 않고
	if (!bHasPistol) return;
	// 현재 총알 갯수가 최대 총알 갯수와 같으면 함수를 나가자.
	if (ownedPistol->IsMaxBulletCount()) return;
	// 현재 재장전 중이면 함수를 나가자.
	if (isReloading) return;

	MulticastRPC_Reload();
}

void ANetTPSCharacter::MulticastRPC_Reload_Implementation()
{
	isReloading = true;

	// 장전 애니메이션 실행
	PlayAnimMontage(playerMontage, 1, TEXT("Reload"));
}

void ANetTPSCharacter::Reload() 
{
	ServerRPC_Reload();
}

void ANetTPSCharacter::ReloadFinish()
{	
	isReloading = false;

	// 현재 총알 갯수를 최대 총알 갯수로 설정
	ownedPistol->Reload();

	InitBulletUI();
}

void ANetTPSCharacter::InitBulletUI()
{
	// 만약에 내 캐릭터가 아니라면 함수 나가자.
	if(IsLocallyControlled() == false) return;

	// crosshair UI 보이게 하자.
	mainUI->ShowCrosshair(true);

	// 총알 UI 다 지우자.
	mainUI->PopBulletAll();

	// addBulletCount 만큼 총알 UI 채우자.
	for (int i = 0; i < ownedPistol->currBulletCount; i++)
	{
		mainUI->AddBulet();
	}
}

void ANetTPSCharacter::BillboardHP()
{
	// 카메라 찾자
	AActor* cam = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	// -카메라 앞방향 값을 담자
	FVector forward = -cam->GetActorForwardVector();
	// 카메라 윗방향
	FVector up = cam->GetActorUpVector();

	// 위 두방향을 이용해서 HP 바의 회전 값을 구하자.
	FRotator rot = UKismetMathLibrary::MakeRotFromXZ(forward, up);

	compHP->SetWorldRotation(rot);
}

void ANetTPSCharacter::DieProcess()
{
	if(IsLocallyControlled() == false) return;

	// 화면 회색 처리
	FollowCamera->PostProcessSettings.ColorSaturation = FVector4(0, 0, 0, 1);

	// 마우스 커서 보이게 하자.
	APlayerController* pc = Cast<APlayerController>(Controller);
	pc->SetShowMouseCursor(true);	

	// 다시하기 버튼 보이게
	mainUI->ShowBtnRetry(true);
}
