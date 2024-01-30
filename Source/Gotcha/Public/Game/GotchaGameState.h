// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GotchaType/Team.h"
#include "GotchaGameState.generated.h"

class AShooterPlayerState;

USTRUCT(BlueprintType)
struct FTeam
{	
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<APlayerState*> Members;
};

USTRUCT(BlueprintType)
struct FTeams
{	
	GENERATED_BODY()

	UPROPERTY()
	TArray<ETeam> Teams;
};

/**
 * 
 */
UCLASS()
class GOTCHA_API AGotchaGameState : public AGameState
{
	GENERATED_BODY()

public:
	AGotchaGameState();
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void UpdateTopScore(AShooterPlayerState* ScoringPlayer);

	UPROPERTY()
	TArray<AShooterPlayerState*> TopPlayers;
	
	void ScoreTeam(ETeam ScoringTeam, int32 NumberOfTeams);

	UPROPERTY()
	TMap<ETeam, FTeam> Teams;

	UPROPERTY()
	TMap<ETeam, float> TeamScores;

	UPROPERTY()
	TMap<ETeam, int32> TeamRanks;
	UPROPERTY()
	TMap<int32, FTeams> TeamsAtRank;

	UPROPERTY(ReplicatedUsing = OnRep_LeaderTeam)
	ETeam LeaderTeam = ETeam::ET_RedTeam;

	void RemoveFromTeam(AShooterPlayerState* PlayerToRemove);

	void CountTeamElim(ETeam Team, float TeamRespawnTime);

	UPROPERTY()
	TMap<ETeam, int32> TeamElimCounts;

	UPROPERTY()
	TMap<ETeam, float> TeamControlPercentage;

	void ControlPoint(float DeltaTime);

	bool bControlEnabled = true;

	UPROPERTY()
	TArray<ETeam> ControllingTeams;

	float ControlSpeed = 0.f;

private:
	float TopScore = 0.f;

	UFUNCTION()
	void OnRep_LeaderTeam();

	UFUNCTION()
	void RespawnTeam(ETeam TeamToRespawn);
	
};
