// Fill out your copyright notice in the Description page of Project Settings.


#include "NetActor.h"
#include <Kismet/GameplayStatics.h>

#include "NetTPSCharacter.h"
#include <Net/UnrealNetwork.h>
#include <Kismet/KismetMathLibrary.h>

// Sets default values
ANetActor::ANetActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Replicate 활성화
	bReplicates = true;

	compMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MESH"));
	SetRootComponent(compMesh);
}

// Called when the game starts or when spawned
void ANetActor::BeginPlay()
{
	Super::BeginPlay();
	
	// 매터리얼 복제
	mat = compMesh->CreateDynamicMaterialInstance(0);

	// 1초마다 ChangeScale 함수 호출하는 타이머 등록
	FTimerHandle handle;
	GetWorldTimerManager().SetTimer(handle, this, &ANetActor::ChageScale, 1.0f, true);


	if (HasAuthority())
	{
		FTimerHandle handleLocation;
		GetWorldTimerManager().SetTimer(handleLocation, this, &ANetActor::ChangeLocation, 1.5f, true);
	}
}

// Called every frame
void ANetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FindOwner();
	Rotate();
	ChangeColor();
	PrintNetLog();
}

void ANetActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate 하고 싶은 변수를 등록
	DOREPLIFETIME(ANetActor, rotYaw);
	DOREPLIFETIME(ANetActor, matColor);
}

void ANetActor::OnRep_RotYaw()
{
	FRotator rot = GetActorRotation();
	rot.Yaw = rotYaw;
	SetActorRotation(rot);
}

void ANetActor::OnRep_ChangeColor()
{
	// 해당 색을 매터리얼 설정하자.
	mat->SetVectorParameterValue(TEXT("FloorColor"), matColor);
}

void ANetActor::ChangeColor()
{
	// 만약에 서버라면
	if (HasAuthority())
	{
		// 시간을 흐르게 하자.
		currTime += GetWorld()->DeltaTimeSeconds;
		// 만약에 현재시간이 색상변경시간보다 커지면
		if (currTime > changeTime)
		{
			// 랜덤한 색상 뽑아내고
			matColor = FLinearColor::MakeRandomColor();
			// 해당 색을 매터리얼 설정하자.
			mat->SetVectorParameterValue(TEXT("FloorColor"), matColor);

			currTime = 0;
		}
	}
}

void ANetActor::ChageScale()
{
	if (GetOwner() == GetWorld()->GetFirstPlayerController()->GetPawn())
	{
		// 서버에게 크기 변경 요청
		ServerRPC_ChangeScale();
	}
}

void ANetActor::ServerRPC_ChangeScale_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("ServerRPC_ChangeScale_Implementation"));
	float rand = FMath::RandRange(0.5f, 2.0f);

	// 클라이언트 한테 rand 만큼 크기 변경되게 알려주자.
	//ClientRPC_ChangeScale(FVector(rand));
	MulitcastRPC_ChangeScale(FVector(rand));
}

void ANetActor::ClientRPC_ChangeScale_Implementation(FVector scale)
{	
	SetActorScale3D(scale);
}

void ANetActor::MulitcastRPC_ChangeScale_Implementation(FVector scale)
{
	SetActorScale3D(scale);
}

void ANetActor::ChangeLocation()
{
	// 랜덤한 위치를 뽑아내자.
	FVector rand = UKismetMathLibrary::RandomPointInBoundingBox(GetActorLocation(), FVector(20));
	// rand 모든 클라이언트에게 알려주자.
	MulitcastRPC_ChangeScale(rand);
}

void ANetActor::MulticastRPC_ChnageLocation_Implementation(FVector location)
{
	SetActorLocation(location);
}

void ANetActor::FindOwner()
{
	if(HasAuthority() == false) return;

	TArray<AActor*> allPlayers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANetTPSCharacter::StaticClass(), allPlayers);

	AActor* owner = nullptr;
	float closestDistance = searchDistance;
	for (int32 i = 0; i < allPlayers.Num(); i++)
	{
		float dist = FVector::Distance(GetActorLocation(), allPlayers[i]->GetActorLocation());
		if (dist < closestDistance)
		{
			closestDistance = dist;
			owner = allPlayers[i];
		}
	}

	// owner 값을 해당 Actor 에 Owner 설정
	if (GetOwner() != owner)
	{
		SetOwner(owner);
	}
}

void ANetActor::Rotate()
{
	// 만약에 서버라면
	if (HasAuthority())
	{
		AddActorWorldRotation(FRotator(0, 50 * GetWorld()->DeltaTimeSeconds, 0));
		rotYaw = GetActorRotation().Yaw;
	}
}

void ANetActor::PrintNetLog()
{
	// 연결상태
	FString connStr = GetNetConnection() != nullptr ? TEXT("Valid Connection") : TEXT("Invalid Connection");

	// Owner 
	FString ownerStr = GetOwner() != nullptr ? GetOwner()->GetName() : TEXT("No Owner");

	// 권한
	FString role = UEnum::GetValueAsString<ENetRole>(GetLocalRole());


	FString logStr = FString::Printf(TEXT("Connection : %s\nOwner : %s\nrole : %s"),
		*connStr,
		*ownerStr,
		*role);

	DrawDebugString(GetWorld(), GetActorLocation(), logStr, nullptr, FColor::Yellow, 0, true, 1);
}

