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

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ANetTPSCharacter

ANetTPSCharacter::ANetTPSCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input...	
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
	CameraBoom->SetRelativeLocation(FVector(0, 40, 60));
	CameraBoom->TargetArmLength = 150; // The camera follows at this distance behind the character	
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
		
		// F 키 눌렀을 때 호출되는 함수 등록
		EnhancedInputComponent->BindAction(takeAction, ETriggerEvent::Started, this, &ANetTPSCharacter::TakePistol);
		
		// 마우스 왼쪽 버튼 눌렀을 때 호출되는 함수 등록
		EnhancedInputComponent->BindAction(fireAction, ETriggerEvent::Started, this, &ANetTPSCharacter::Fire);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
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
	// 총을 소유하고 있지 않다면 일정범위 안에 있는 총을 잡는다.
	// 1. 총을 잡고 있지 않다면
	if (bHasPistol == false)
	{
		// 2. 월드에 있는 총을 모두 찾는다.
		TArray<AActor*> allActors;
		TArray<AActor*> pistolActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), allActors);
		for (int i = 0; i < allActors.Num(); i++)
		{
			if (allActors[i]->GetActorLabel().Contains(TEXT("BP_Pistol")))
			{
				pistolActors.Add(allActors[i]);
			}
		}

		for (AActor* pistol : pistolActors)
		{
			// 만약에 pistol 의 소유자가 없다면
			if (pistol->GetOwner() == nullptr)
			{
				// 3. 총과의 거리를 구하자.
				float dist = FVector::Distance(pistol->GetActorLocation(), GetActorLocation());
				// 4. 만약에 거리가 일정범위 안에 있다면
				if (dist < distanceToGun)
				{
					// 5. 총을 Mesh 의 손에 붙히자.
					pistol->SetOwner(this);
					bHasPistol = true;
					ownedPistol = pistol;

					AttackPistol(pistol);
					break;
				}
			}
		}
	}
	// 총을 잡고 있다면
	else
	{
		// 총을 놓자.
		DetachPistol();

		bHasPistol = false;
		ownedPistol->SetOwner(nullptr);
		ownedPistol = nullptr;
	}	
}

void ANetTPSCharacter::AttackPistol(AActor* pistol)
{
	// pistol 이 가지고 있는 StaticMesh 컴포넌트 가져오자
	UStaticMeshComponent* comp = pistol->GetComponentByClass<UStaticMeshComponent>();
	// 가져온 컴포넌트를 이용해서 SimulatePhysisc 비활성화
	comp->SetSimulatePhysics(false);
	// Mesh - gunPositon 소켓에 붙히자.
	pistol->AttachToComponent(compGun, FAttachmentTransformRules::SnapToTargetNotIncludingScale);	
}

void ANetTPSCharacter::DetachPistol()
{
	// pistol 이 가지고 있는 StaticMesh 컴포넌트 가져오자
	UStaticMeshComponent* comp = ownedPistol->GetComponentByClass<UStaticMeshComponent>();
	// 가져온 컴포넌트를 이용해서 SimulatePhysisc 활성화
	comp->SetSimulatePhysics(true);
	// 총을 gunPosition 에서 분리하자.
	ownedPistol->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
}

void ANetTPSCharacter::Fire()
{
	// 만약에 총을 들고 있지 않다면 함수를 나가자
	if(bHasPistol == false) return;
	// LineTrace 로 부딪힌 곳 찾아내자.

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

		// 맞은 위치에 파티클로 표시 하자.
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), gunEffect, hitInfo.Location, FRotator(), true);
	}

	// 총쏘는 애니메이션 실행하자 (Montage 실행)
	PlayAnimMontage(playerMontage, 2, TEXT("Fire"));
}
