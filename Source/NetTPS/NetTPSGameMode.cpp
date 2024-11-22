// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetTPSGameMode.h"
#include "NetTPSCharacter.h"
#include "UObject/ConstructorHelpers.h"

ANetTPSGameMode::ANetTPSGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void ANetTPSGameMode::AddPlayer(ANetTPSCharacter* player)
{
	allPlayers.Add(player);
}

void ANetTPSGameMode::ChangeTurn()
{
	// 현재 turnIdx 의 Player 에게 큐브생성 생성 못하게 하자.
	allPlayers[turnIdx]->canMakeCube = false;

	//턴을 증가시키자
	turnIdx++;

	//만약에 turnIdx 가 allPlayers 의 갯수와 같거나 크다면
	if (turnIdx >= allPlayers.Num())
	{
		// 턴을 처음으로 돌리자
		turnIdx = 0;
	}

	//turnIdx = (turnIdx + 1) % allPlayers.Num();

	// turnIdx 에 있는 Player가 큐브를 만들 수 있게 하자.
	allPlayers[turnIdx]->canMakeCube = true;
}
