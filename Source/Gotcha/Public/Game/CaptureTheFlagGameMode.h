// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/TeamGameMode.h"
#include "CaptureTheFlagGameMode.generated.h"

class AShooterPlayerState;

/**
 * 
 */
UCLASS()
class GOTCHA_API ACaptureTheFlagGameMode : public ATeamGameMode
{
	GENERATED_BODY()

public:
	virtual void PlayerEliminated(AShooterCharacterBase* ElimmedCharacter, AShooterPlayerController* VictimController, AShooterPlayerController* AttackerController);
	void FlagCaptured(AShooterPlayerState* ScoringPlayer);
	
};
