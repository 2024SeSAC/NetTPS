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
#include "MainUI.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ANetTPSCharacter

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
	
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ANetTPSCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
	
	InitMainUIWidget();

	// originCamPos �� �ʱ��� CameraBoom ������ ����
	originCamPos = CameraBoom->GetRelativeLocation();
}

void ANetTPSCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// CameraBoom �� originCamPos �� ���ؼ� ��� ��������.
	FVector pos = FMath::Lerp(CameraBoom->GetRelativeLocation(), originCamPos, DeltaSeconds * 10);
	CameraBoom->SetRelativeLocation(pos);
}

//////////////////////////////////////////////////////////////////////////
// Input

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
		
		// F Ű ������ �� ȣ��Ǵ� �Լ� ���
		EnhancedInputComponent->BindAction(takeAction, ETriggerEvent::Started, this, &ANetTPSCharacter::TakePistol);
		
		// ���콺 ���� ��ư ������ �� ȣ��Ǵ� �Լ� ���
		EnhancedInputComponent->BindAction(fireAction, ETriggerEvent::Started, this, &ANetTPSCharacter::Fire);

		// R Ű ������ �� ȣ��Ǵ� �Լ� ���
		EnhancedInputComponent->BindAction(reloadAction, ETriggerEvent::Started, this, &ANetTPSCharacter::Reload);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ANetTPSCharacter::InitMainUIWidget()
{
	mainUI = Cast<UMainUI>( CreateWidget(GetWorld(), mainUIWidget) );
	mainUI->AddToViewport();

	// �Ѿ� UI ����
	currBulletCount = maxBulletCount;
	for (int i = 0; i < currBulletCount; i++)
	{
		mainUI->AddBulet();
	}
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

void ANetTPSCharacter::TakePistol()
{
	// ���� �����ϰ� ���� �ʴٸ� �������� �ȿ� �ִ� ���� ��´�.
	// 1. ���� ��� ���� �ʴٸ�
	if (bHasPistol == false)
	{
		// 2. ���忡 �ִ� ���� ��� ã�´�.
		TArray<AActor*> allActors;
		TArray<AActor*> pistolActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), allActors);
		for (int32 i = 0; i < allActors.Num(); i++)
		{
			if (allActors[i]->GetActorLabel().Contains(TEXT("BP_Pistol")))
			{
				pistolActors.Add(allActors[i]);
			}
		}

		// ���� ���� �ִܰŸ�
		float closestDist = std::numeric_limits<float>::max();
		AActor* closestPistol = nullptr;

		for (AActor* pistol : pistolActors)
		{
			// ���࿡ pistol �� �����ڰ� ���ٸ�
			if (pistol->GetOwner() == nullptr)
			{
				// 3. �Ѱ��� �Ÿ��� ������.
				float dist = FVector::Distance(pistol->GetActorLocation(), GetActorLocation());
				// 4. ���࿡ �Ÿ��� �������� �ȿ� �ִٸ�
				if (dist < distanceToGun)
				{
					// closestDist ������ dist ���� ũ�ٸ� (�� �����ٴ� �ǹ�)
					if (closestDist > dist)
					{
						// �ִ� �Ÿ� ����
						closestDist = dist;
						closestPistol = pistol;						
					}
				}
			}
		}

		// 5. ���� Mesh �� �տ� ������.
		AttackPistol(closestPistol);
	}
	// ���� ��� �ִٸ�
	else
	{
		// ���� ����.
		DetachPistol();		
	}	
}

void ANetTPSCharacter::AttackPistol(AActor* pistol)
{
	if(pistol == nullptr) return;

	pistol->SetOwner(this);
	bHasPistol = true;
	ownedPistol = pistol;

	// pistol �� ������ �ִ� StaticMesh ������Ʈ ��������
	UStaticMeshComponent* comp = pistol->GetComponentByClass<UStaticMeshComponent>();
	// ������ ������Ʈ�� �̿��ؼ� SimulatePhysisc ��Ȱ��ȭ
	comp->SetSimulatePhysics(false);
	// compGun �� ������.
	pistol->AttachToComponent(compGun, FAttachmentTransformRules::SnapToTargetNotIncludingScale);	

	// �� ����� �� ĳ���� ȸ�� ��� ���� (ī�޶� ���ؼ� ����ǵ���)
	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	CameraBoom->TargetArmLength = 150;
	originCamPos = FVector(0, 40, 60);

	// crosshair UI ���̰� ����.
	mainUI->ShowCrosshair(true);
}

void ANetTPSCharacter::DetachPistol()
{
	// pistol �� ������ �ִ� StaticMesh ������Ʈ ��������
	UStaticMeshComponent* comp = ownedPistol->GetComponentByClass<UStaticMeshComponent>();
	// ������ ������Ʈ�� �̿��ؼ� SimulatePhysisc Ȱ��ȭ
	comp->SetSimulatePhysics(true);
	// ���� gunPosition ���� �и�����.
	ownedPistol->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	// �� ������ ĳ���� ȸ�� ��� ���� (ī�޶�� ����)
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	CameraBoom->TargetArmLength = 300;
	originCamPos = FVector(0, 0, 60);

	// crosshair UI ������ �ʰ� ����.
	mainUI->ShowCrosshair(false);

	bHasPistol = false;
	ownedPistol->SetOwner(nullptr);
	ownedPistol = nullptr;
}

void ANetTPSCharacter::Fire()
{
	// ���࿡ ���� ��� ���� �ʴٸ� �Լ��� ������
	if(bHasPistol == false) return;

	// ���� �Ѿ� ������ 0���� �۰ų� ������ �Լ��� ������
	if(currBulletCount <= 0) return;
	
	// ������ ���̸� �Լ��� ������.
	if(isReloading) return;

	// LineTrace �� �ε��� �� ã�Ƴ���.

	FVector startPos = FollowCamera->GetComponentLocation();
	FVector endPos = startPos + FollowCamera->GetForwardVector() * 100000;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);

	FHitResult hitInfo;
	bool bHit = GetWorld()->LineTraceSingleByChannel(hitInfo, startPos, endPos, ECollisionChannel::ECC_Visibility, params);

	if (bHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s, %s"), 
			*hitInfo.GetActor()->GetActorLabel(), 
			*hitInfo.GetActor()->GetName());

		// ���� ��ġ�� ��ƼŬ�� ǥ�� ����.
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), gunEffect, hitInfo.Location, FRotator(), true);
	}

	// �ѽ�� �ִϸ��̼� �������� (Montage ����)
	PlayAnimMontage(playerMontage, 2, TEXT("Fire"));

	// �Ѿ� ����
	currBulletCount--;
	mainUI->PopBullet(currBulletCount);
}

void ANetTPSCharacter::Reload()
{
	// ���� ������ ���� �ʰ�
	if(!bHasPistol) return;
	// ���� �Ѿ� ������ �ִ� �Ѿ� ������ ������ �Լ��� ������.
	if(currBulletCount == maxBulletCount) return;
	// ���� ������ ���̸� �Լ��� ������.
	if(isReloading) return;

	isReloading = true;

	// ���� �ִϸ��̼� ����
	PlayAnimMontage(playerMontage, 1, TEXT("Reload"));
}

void ANetTPSCharacter::ReloadFinish()
{
	isReloading = false;

	// ä���� �ϴ� �Ѿ� ���� ���
	int32 addBulletCount = maxBulletCount - currBulletCount;

	// ���� �Ѿ� ������ �ִ� �Ѿ� ������ ����
	currBulletCount = maxBulletCount;

	// addBulletCount ��ŭ �Ѿ� UI ä����.
	for (int i = 0; i < addBulletCount; i++)
	{
		mainUI->AddBulet();
	}
}
