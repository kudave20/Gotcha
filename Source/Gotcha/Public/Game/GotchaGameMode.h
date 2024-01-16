// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GotchaGameMode.generated.h"

namespace MatchState
{
	extern GOTCHA_API const FName Ending; // Match duration has been reached. Display winner and begin ending timer.
}

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
	virtual void Tick(float DeltaTime) override;
	virtual void PlayerEliminated(AShooterCharacterBase* ElimmedCharacter, AShooterPlayerController* VictimController, AShooterPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);
	
	UPROPERTY(EditDefaultsOnly, Category = "Game Settings")
	float MatchTime = 600.f;

	UPROPERTY(EditDefaultsOnly, Category = "Game Settings")
	float EndingTime = 10.f;

	float LevelStartingTime = 0.f;

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
	
	bool bTeamsMatch = false;
	
private:
	float CountdownTime = 0.f;
	
};
