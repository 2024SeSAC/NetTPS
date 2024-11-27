// Fill out your copyright notice in the Description page of Project Settings.


#include "NetGameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include <Online/OnlineSessionNames.h>

void UNetGameInstance::Init()
{
	Super::Init();

	// 현재 사용하는 서브시스템을 가져오자
	IOnlineSubsystem* subsys = IOnlineSubsystem::Get();
	if (subsys)
	{
		sessionInterface = subsys->GetSessionInterface();
		// 세션 생성 성공시 호출되는 함수 등록
		sessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UNetGameInstance::OnCreateSessionComplete);
		// 세션 파괴 성공시 호출되는 함수 등록
		sessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UNetGameInstance::OnDestroySessionComplete);
		// 세션 검색 성공시 호출되는 함수 등록
		sessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UNetGameInstance::OnFindSessionsComplete);
	}
}

void UNetGameInstance::CreateMySession(FString displayName, int32 playerCount)
{
	// 세션을 만들기 위한 옵션 설정
	FOnlineSessionSettings sessionSettings;

	// Lan 을 이용할지 여부
	FName subsysName = IOnlineSubsystem::Get()->GetSubsystemName();
	UE_LOG(LogTemp, Warning, TEXT("서브시스템 이름 : %s"), *subsysName.ToString());
	sessionSettings.bIsLANMatch = subsysName.IsEqual(FName(TEXT("NULL")));

	// Lobby 를 사용할지 여부
	sessionSettings.bUseLobbiesIfAvailable = true;

	// 세션 검색을 허용할지 여부
	sessionSettings.bShouldAdvertise = true;

	// 세션이 진행 중일때 참여 가능 여부
	sessionSettings.bAllowJoinInProgress = true;

	// 세션의 최대 인원 설정
	sessionSettings.NumPublicConnections = playerCount;

	// 커스텀 정보
	sessionSettings.Set(TEXT("DP_NAME"), displayName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	// 세션 생성
	FUniqueNetIdPtr netID = GetWorld()->GetFirstLocalPlayerFromController()->GetUniqueNetIdForPlatformUser().GetUniqueNetId();
	sessionInterface->CreateSession(*netID, FName(displayName), sessionSettings);
}

void UNetGameInstance::OnCreateSessionComplete(FName sessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] 세션 생성 성공"), *sessionName.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] 세션 생성 실패"), *sessionName.ToString());
	}
}

void UNetGameInstance::DestroyMySession(FString sessionName)
{
	sessionInterface->DestroySession(FName(sessionName));
}

void UNetGameInstance::OnDestroySessionComplete(FName sessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] 세션 파괴 성공"), *sessionName.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] 세션 파괴 실패"), *sessionName.ToString());
	}
}

void UNetGameInstance::FindOtherSession()
{
	UE_LOG(LogTemp, Warning, TEXT("세션 검색 시작"));

	// 세션 검색 설정
	sessionSearch = MakeShared<FOnlineSessionSearch>();
	
	// 랜을 사용할지 여부
	FName subsysName = IOnlineSubsystem::Get()->GetSubsystemName();
	UE_LOG(LogTemp, Warning, TEXT("서브시스템 이름 : %s"), *subsysName.ToString());
	sessionSearch->bIsLanQuery = subsysName.IsEqual(FName(TEXT("NULL")));

	// 활성화 되어있는 세션만 검색하자.
	sessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	// 세션을 몇개까지 검색할지
	sessionSearch->MaxSearchResults = 100;
	
	// 세션 검색하자
	sessionInterface->FindSessions(0, sessionSearch.ToSharedRef());
}

void UNetGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		auto results = sessionSearch->SearchResults;
		
		for (int i = 0; i < results.Num(); i++)
		{
			FOnlineSessionSearchResult sr = results[i];

			// DP_NAME 가져오자
			FString displayName;
			sr.Session.SessionSettings.Get(TEXT("DP_NAME"), displayName);
			
			// 세션을 만들 사람 이름
			FString sessionCreator = sr.Session.OwningUserName;

			UE_LOG(LogTemp, Warning, TEXT("세션 : %s, 만든이 : %s"), *displayName, *sessionCreator);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("세션 검색 완료"));
}
