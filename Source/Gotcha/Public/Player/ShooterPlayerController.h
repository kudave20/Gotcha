// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GotchaType/Team.h"
#include "ShooterPlayerController.generated.h"

class AShooterHUD;
class UCharacterOverlay;
class AGotchaGameMode;

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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDAmmo(int32 Ammo);
	void SetHUDOwnerTeam(ETeam OwnerTeam);
	void SetHUDLeaderTeam(ETeam LeaderTeam);
	void SetHUDOwnerRank(int32 OwnerRank);
	void SetHUDMatchTimer(float CountdownTime);

	void OnMatchStateSet(FName State);
	void HandleMatchHasStarted();
	void HandleEnding();

	virtual float GetServerTime();
	virtual void ReceivedPlayer() override;
	
protected:
	virtual void BeginPlay() override;

	void PollInit();
	void SetHUDTime();

	/*
	* Sync time between client and server
	*/

	// Requests the current server time, passing in the client's time when the request was sent
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	// Reports the current server time to the client in response to ServerRequestServerTime
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	float ClientServerDelta = 0.f; // difference between client and server time

	UPROPERTY(EditAnywhere, Category = "Properties")
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;
	void CheckTimeSync(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float Match, float Cooldown, float StartingTime, bool bTeams);
	
private:
	UPROPERTY()
	AShooterHUD* ShooterHUD;

	UPROPERTY()
	UCharacterOverlay* CharacterOverlay;

	UPROPERTY()
	AGotchaGameMode* GotchaGameMode;

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

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();
	
	bool bTeamsMatch = false;

	void InitTeamScores();
	void HideTeamScores();

	UPROPERTY(EditAnywhere, Category = "Properties")
	TMap<ETeam, FColor> TeamColors;

	float LevelStartingTime = 0.f;
	float MatchTime = 0.f;
	float EndingTime = 0.f;
	uint32 CountdownInt = 0;
	
};
