// Fill out your copyright notice in the Description page of Project Settings.


#include "NetActor.h"
#include <Kismet/GameplayStatics.h>

#include "NetTPSCharacter.h"
#include <Net/UnrealNetwork.h>

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
	
}

// Called every frame
void ANetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FindOwner();
	Rotate();
	PrintNetLog();
}

void ANetActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate 하고 싶은 변수를 등록
	DOREPLIFETIME(ANetActor, rotYaw);
}

void ANetActor::OnRep_RotYaw()
{
	FRotator rot = GetActorRotation();
	rot.Yaw = rotYaw;
	SetActorRotation(rot);
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

