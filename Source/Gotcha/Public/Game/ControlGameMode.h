// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/TeamGameMode.h"
#include "ControlGameMode.generated.h"

class AShooterPlayerState;

/**
 * 
 */
UCLASS()
class GOTCHA_API AControlGameMode : public ATeamGameMode
{
	GENERATED_BODY()
	
public:
	virtual void PlayerEliminated(AShooterCharacterBase* ElimmedCharacter, AShooterPlayerController* VictimController, AShooterPlayerController* AttackerController);
	void ControlPoint(AShooterPlayerState* ControllingPlayer, AShooterPlayerState* ExitingPlayer);

private:
	UPROPERTY(EditAnywhere, Category = "Game Settings")
	float TimeToFullyControl = 10.f;
	
};
