// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GotchaType/Team.h"
#include "ShooterPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class GOTCHA_API AShooterPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	
	void AddToScore(float ScoreAmount);
	
private:
	UPROPERTY(Replicated)
	ETeam Team = ETeam::ET_NoTeam;

	UPROPERTY(ReplicatedUsing = OnRep_TeamRank)
	int32 TeamRank = 1;

	UFUNCTION()
	void OnRep_TeamRank();

	bool bControl = false;

public:
	FORCEINLINE ETeam GetTeam() const { return Team; }
	void SetTeam(ETeam TeamToSet);
	FORCEINLINE int32 GetTeamRank() const { return TeamRank; }
	FORCEINLINE void SetTeamRank(int32 Rank) { TeamRank = Rank; }
	FORCEINLINE bool IsControlling() const { return bControl; }
	FORCEINLINE void Control(bool bControlling) { bControl = bControlling; }
	
};
