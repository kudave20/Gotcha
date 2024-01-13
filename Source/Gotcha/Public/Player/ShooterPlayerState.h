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
	void AddToScore(float ScoreAmount);
	
private:
	ETeam Team = ETeam::ET_NoTeam;

public:
	FORCEINLINE ETeam GetTeam() const { return Team; }
	void SetTeam(ETeam TeamToSet);
	
};
