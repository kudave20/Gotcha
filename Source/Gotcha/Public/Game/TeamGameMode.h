// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/GotchaGameMode.h"
#include "TeamGameMode.generated.h"

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

private:
	UPROPERTY(EditDefaultsOnly, Category = "Game Settings")
	int32 TeamMemberLimit = 4;
	
};
