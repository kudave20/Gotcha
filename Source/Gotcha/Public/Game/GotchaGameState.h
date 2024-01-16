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
	AGotchaGameState();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void UpdateTopScore(AShooterPlayerState* ScoringPlayer);
	
	TArray<AShooterPlayerState*> TopPlayers;
	
	void ScoreTeam(ETeam ScoringTeam, int32 NumberOfTeams);

	TMap<ETeam, TArray<APlayerState*>> Teams;

	TMap<ETeam, float> TeamScores;

	TMap<ETeam, int32> TeamRanks;
	TMap<int32, TArray<ETeam>> TeamsAtRank;

	UPROPERTY(ReplicatedUsing = OnRep_LeaderTeam)
	ETeam LeaderTeam = ETeam::ET_RedTeam;

	void RemoveFromTeam(AShooterPlayerState* PlayerToRemove);

private:
	float TopScore = 0.f;

	UFUNCTION()
	void OnRep_LeaderTeam();
	
};
