// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GotchaType/Team.h"
#include "GotchaGameState.generated.h"

class AShooterPlayerState;

/**
 * 
 */
UCLASS()
class GOTCHA_API AGotchaGameState : public AGameState
{
	GENERATED_BODY()

public:
	void UpdateTopScore(AShooterPlayerState* ScoringPlayer);
	
	TArray<AShooterPlayerState*> TopPlayers;

	TArray<ETeam> TopTeams;
	
	void ScoreTeam(ETeam Team);

	TMap<ETeam, TArray<APlayerState*>> Teams;

	TMap<ETeam, float> TeamScores;

	void RemoveFromTeam(AShooterPlayerState* PlayerToRemove);

private:
	float TopScore = 0.f;
	float TopTeamScore = 0.f;
	
};
