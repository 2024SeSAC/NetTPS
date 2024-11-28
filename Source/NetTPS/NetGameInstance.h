// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "NetGameInstance.generated.h"

DECLARE_DELEGATE_OneParam(FAddSession, FString);

UCLASS()
class NETTPS_API UNetGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	// 세션 생성
	UFUNCTION(BlueprintCallable)
	void CreateMySession(FString displayName, int32 playerCount);
	void OnCreateSessionComplete(FName sessionName, bool bWasSuccessful);

	// 세션 파괴
	UFUNCTION(BlueprintCallable)
	void DestroyMySession(FString sessionName);
	void OnDestroySessionComplete(FName sessionName, bool bWasSuccessful);

	// 세션 검색
	UFUNCTION(BlueprintCallable)
	void FindOtherSession();
	void OnFindSessionsComplete(bool bWasSuccessful);


public:
	// 세션의 모든 처리를 진행
	IOnlineSessionPtr sessionInterface;

	// 세션 검색 처리
	TSharedPtr<FOnlineSessionSearch> sessionSearch;

	// 세션이 검색되었을 때 각 세션의 정보를 전달해주는 딜리게이트
	FAddSession  onAddSession;
};
