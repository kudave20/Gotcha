// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/GotchaGameMode.h"
#include "TeamGameMode.generated.h"

class AShooterCharacterBase;
class AShooterPlayerController;

/**
 * 
 */
UCLASS()
class GOTCHA_API ATeamGameMode : public AGotchaGameMode
{
	GENERATED_BODY()

public:
	ATeamGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void PlayerEliminated(AShooterCharacterBase* ElimmedCharacter, AShooterPlayerController* VictimController, AShooterPlayerController* AttackerController) override;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Game Settings")
	uint32 NumberOfTeams = 4;
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Game Settings")
	uint32 TeamMemberLimit = 3;

	UPROPERTY(EditAnywhere, Category = "Game Settings")
	float TeamRespawnTime = 15.f;
	
};
