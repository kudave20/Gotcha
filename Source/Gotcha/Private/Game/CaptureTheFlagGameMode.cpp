// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/CaptureTheFlagGameMode.h"
#include "Game/GotchaGameState.h"
#include "Player/ShooterPlayerState.h"

void ACaptureTheFlagGameMode::PlayerEliminated(AShooterCharacterBase* ElimmedCharacter, AShooterPlayerController* VictimController, AShooterPlayerController* AttackerController)
{
	AGotchaGameMode::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);
}

void ACaptureTheFlagGameMode::FlagCaptured(AShooterPlayerState* ScoringPlayer)
{
	AGotchaGameState* GGameState = Cast<AGotchaGameState>(GameState);
	if (GGameState)
	{
		GGameState->ScoreTeam(ScoringPlayer->GetTeam());
	}
}
