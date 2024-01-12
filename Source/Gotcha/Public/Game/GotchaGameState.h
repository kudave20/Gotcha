// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GotchaType/Team.h"
#include "GotchaGameState.generated.h"

class AGotchaPlayerState;

/**
 * 
 */
UCLASS()
class GOTCHA_API AGotchaGameState : public AGameState
{
	GENERATED_BODY()

public:
	TArray<AGotchaPlayerState*> TopPlayers;

	ETeam TopTeam;
	
	void ScoreTeam(ETeam Team);

	TMap<ETeam, TArray<APlayerState*>> Teams;

	TMap<ETeam, float> TeamScores;
};
