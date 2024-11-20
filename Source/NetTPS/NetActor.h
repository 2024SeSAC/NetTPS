// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetActor.generated.h"

UCLASS()
class NETTPS_API ANetActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANetActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* compMesh;

	// 발견 범위
	float searchDistance = 200;

	// yaw 회전값
	//UPROPERTY(Replicated)
	UPROPERTY(ReplicatedUsing = OnRep_RotYaw)
	float rotYaw = 0;
	UFUNCTION()
	void OnRep_RotYaw();
	void Rotate();

	// 매터리얼 색상 변경
	UPROPERTY()
	class UMaterialInstanceDynamic* mat;

	UPROPERTY(ReplicatedUsing = OnRep_ChangeColor)
	FLinearColor matColor;
	UFUNCTION()
	void OnRep_ChangeColor();
	void ChangeColor();
	float changeTime = 2.0f;
	float currTime = 0;

	// RPC 이용해서 크기변경
	void ChageScale();
	// 서버 RPC
	UFUNCTION(Server, Reliable)
	void ServerRPC_ChangeScale();
	// 클라이언트 RPC
	UFUNCTION(Client, Reliable)
	void ClientRPC_ChangeScale(FVector scale);
	// Multicast RPC
	UFUNCTION(NetMulticast, Reliable)
	void MulitcastRPC_ChangeScale(FVector scale);



	void FindOwner();
	void PrintNetLog();
};
