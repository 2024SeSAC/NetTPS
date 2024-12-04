// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NetTPSGameMode.generated.h"

UCLASS(minimalapi)
class ANetTPSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANetTPSGameMode();

public:
	UPROPERTY(VisibleAnywhere)
	TArray<class ANetTPSCharacter*> allPlayers;
	// 현재 몇 번째 Player 가 만들 차례니? 
	int32 turnIdx;

	// 내가 몇 번째 위치에서 태어나야하니?
	int32 posIdx;

public:
	void AddPlayer(class ANetTPSCharacter* player);
	void ChangeTurn();


};



