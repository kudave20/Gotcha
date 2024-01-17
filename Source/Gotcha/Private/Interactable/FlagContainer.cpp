// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/FlagContainer.h"
#include "Character/ShooterCharacterBase.h"
#include "Game/CaptureTheFlagGameMode.h"
#include "Player/ShooterPlayerState.h"

void AFlagContainer::OnInteract(APlayerController* Player)
{	
	Super::OnInteract(Player);

	if (Player == nullptr) return;

	AShooterCharacterBase* ShooterCharacter = Cast<AShooterCharacterBase>(Player->GetPawn());
	if (ShooterCharacter && ShooterCharacter->IsHoldingFlag())
	{
		ACaptureTheFlagGameMode* GameMode = GetWorld()->GetAuthGameMode<ACaptureTheFlagGameMode>();
		if (GameMode)
		{
			GameMode->FlagCaptured(Player->GetPlayerState<AShooterPlayerState>());
		}
	}
}
