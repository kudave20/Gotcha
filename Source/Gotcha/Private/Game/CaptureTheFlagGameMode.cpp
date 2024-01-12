// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/CaptureTheFlagGameMode.h"
#include "Game/GotchaGameState.h"
#include "Player/GotchaPlayerState.h"

void ACaptureTheFlagGameMode::FlagCaptured(AGotchaPlayerState* ScoringPlayer)
{
	AGotchaGameState* GGameState = Cast<AGotchaGameState>(GameState);
	if (GGameState)
	{
		GGameState->ScoreTeam(ScoringPlayer->GetTeam());
	}
}
