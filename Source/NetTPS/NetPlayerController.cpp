// Fill out your copyright notice in the Description page of Project Settings.


#include "NetPlayerController.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/SpectatorPawn.h"
#include "NetGameState.h"
#include "NetGameInstance.h"

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

	// 5초 뒤에 다시 살아나자.
	FTimerHandle handle;
	GetWorldTimerManager().SetTimer(handle, this, &ANetPlayerController::RespawnPlayer, 5, false);
}

void ANetPlayerController::RespawnPlayer()
{
	// 현재 Possess 하고 있는 Pawn 을 가져오자.
	APawn* player = GetPawn();
	// UnPossess
	UnPossess();
	player->Destroy();

	AGameModeBase* gm = GetWorld()->GetAuthGameMode();
	gm->RestartPlayer(this);
}





void ANetPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (IsLocalController())
	{
		if (WasInputKeyJustPressed(EKeys::LeftControl))
		{
			Cast<ANetGameState>(GetWorld()->GetGameState())->ShowCursor(true);			
		}

		if (WasInputKeyJustPressed(EKeys::Zero))
		{
			if (HasAuthority())
			{
				Cast<ANetGameState>(GetWorld()->GetGameState())->MulticastRPC_DestroySession();
			}
			else
			{
				UNetGameInstance* gi = GetGameInstance<UNetGameInstance>();
				gi->DestroyMySession();
			}
		}
	}
}
