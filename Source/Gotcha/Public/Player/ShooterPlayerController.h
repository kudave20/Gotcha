// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GotchaType/Team.h"
#include "ShooterPlayerController.generated.h"

class AShooterHUD;
class UCharacterOverlay;

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
	void SetHUDAmmo(int32 Ammo);
	void SetHUDOwnerTeam(ETeam OwnerTeam);
	void SetHUDLeaderTeam(ETeam LeaderTeam);
	void SetHUDOwnerRank(int32 OwnerRank);

	void OnMatchStateSet(FName State, bool bTeamsMatch = false);
	void HandleMatchHasStarted(bool bTeamsMatch = false);
	void HandleEnding();

protected:
	virtual void BeginPlay() override;

	void PollInit();

private:
	UPROPERTY()
	AShooterHUD* ShooterHUD;

	UPROPERTY()
	UCharacterOverlay* CharacterOverlay;

	float HUDHealth;
	float HUDMaxHealth;
	float HUDAmmo;
	ETeam HUDOwnerTeam;
	ETeam HUDLeaderTeam;
	int32 HUDOwnerRank;

	bool bInitializeHealth = false;
	bool bInitializeAmmo = false;
	bool bInitializeOwnerTeam = false;
	bool bInitializeLeaderTeam = false;
	bool bInitializeOwnerRank = false;

	FName MatchState;

	void InitTeamScores();
	void HideTeamScores();

	UPROPERTY(EditAnywhere, Category = "Properties")
	TMap<ETeam, FColor> TeamColors;
	
};
