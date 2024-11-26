// Fill out your copyright notice in the Description page of Project Settings.


#include "NetPlayerController.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/SpectatorPawn.h"

void ANetPlayerController::ServerRPC_ChageToSpectator_Implementation()
{
	// 현재 Possess 하고 있는 Pawn 을 가져오자.
	APawn* player = GetPawn();
	// UnPossess
	UnPossess();
	
	// Sepctator Pawn 생성
	AGameModeBase* gm = GetWorld()->GetAuthGameMode();
	ASpectatorPawn* spectator = GetWorld()->SpawnActor<ASpectatorPawn>(gm->SpectatorClass, player->GetActorTransform());
	// 생성된 Sepctator Pawn 을 Possess 하자
	Possess(spectator);

	// 현재 Possess 하고 있는 Pawn 을 파괴하자.
	player->Destroy();
}
