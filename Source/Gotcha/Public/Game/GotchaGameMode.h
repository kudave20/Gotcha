// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GotchaGameMode.generated.h"

class AShooterCharacterBase;
class AShooterPlayerController;

/**
 * 
 */
UCLASS()
class GOTCHA_API AGotchaGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void PlayerEliminated(AShooterCharacterBase* ElimmedCharacter, AShooterPlayerController* VictimController, AShooterPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);
	
};
