// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"

class AShooterHUD;

/**
 * 
 */
UCLASS()
class GOTCHA_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AShooterPlayerController();
	virtual void PlayerTick(float DeltaTime) override;

	void SetHUDHealth(float Health, float MaxHealth);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	AShooterHUD* ShooterHUD;

	float HUDHealth;
	float HUDMaxHealth;
	
};
