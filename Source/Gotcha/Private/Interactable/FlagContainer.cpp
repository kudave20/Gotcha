// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/FlagContainer.h"
#include "Game/CaptureTheFlagGameMode.h"
#include "Player/ShooterPlayerState.h"

void AFlagContainer::OnInteract(APlayerController* Player)
{
	Super::OnInteract(Player);

	ACaptureTheFlagGameMode* GameMode = GetWorld()->GetAuthGameMode<ACaptureTheFlagGameMode>();
	if (GameMode && Player)
	{
		GameMode->FlagCaptured(Player->GetPlayerState<AShooterPlayerState>());
	}
}
