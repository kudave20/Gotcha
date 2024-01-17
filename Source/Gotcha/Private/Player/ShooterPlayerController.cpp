// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ShooterPlayerController.h"
#include "HUD/ShooterHUD.h"
#include "HUD/CharacterOverlay.h"
#include "Components/TextBlock.h"
#include "Game/GotchaGameMode.h"
#include "Character/ShooterCharacterBase.h"
#include "Component/CombatComponent.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

AShooterPlayerController::AShooterPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ShooterHUD = Cast<AShooterHUD>(GetHUD());
	ServerCheckMatchState();
}

void AShooterPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	PollInit();
	SetHUDTime();
	CheckTimeSync(DeltaTime);
}

void AShooterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AShooterPlayerController, MatchState, COND_OwnerOnly);
}

void AShooterPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (ShooterHUD && ShooterHUD->CharacterOverlay)
		{
			CharacterOverlay = ShooterHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				if (bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth);
				if (bInitializeAmmo) SetHUDAmmo(HUDAmmo);
				if (bInitializeOwnerTeam) SetHUDOwnerTeam(HUDOwnerTeam);
				if (bInitializeLeaderTeam) SetHUDLeaderTeam(HUDLeaderTeam);
				if (bInitializeOwnerRank) SetHUDOwnerRank(HUDOwnerRank);
			}
		}
	}
}

void AShooterPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void AShooterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void AShooterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimeServerReceivedClientRequest + (0.5f * RoundTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

void AShooterPlayerController::ServerCheckMatchState_Implementation()
{
	AGotchaGameMode* GameMode = Cast<AGotchaGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		MatchTime = GameMode->MatchTime;
		EndingTime = GameMode->EndingTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		bTeamsMatch = GameMode->bTeamsMatch;
		ClientJoinMidgame(MatchState, MatchTime, EndingTime, LevelStartingTime, bTeamsMatch);
	}
}

void AShooterPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Match, float Cooldown, float StartingTime, bool bTeams)
{
	MatchTime = Match;
	EndingTime = Cooldown;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	bTeamsMatch = bTeams;
	OnRep_MatchState();
}

void AShooterPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::InProgress) TimeLeft = MatchTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::Ending) TimeLeft = EndingTime + MatchTime - GetServerTime() + LevelStartingTime;
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchTimer(TimeLeft);
		}
		if (MatchState == MatchState::Ending)
		{
			// Set HUD announcement countdown.
		}
	}

	CountdownInt = SecondsLeft;
}

void AShooterPlayerController::SetHUDMatchTimer(float CountdownTime)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHUDValid = ShooterHUD &&
		ShooterHUD->CharacterOverlay &&
		ShooterHUD->CharacterOverlay->MatchTimeText;
	if (bHUDValid)
	{
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		ShooterHUD->CharacterOverlay->MatchTimeText->SetText(FText::FromString(CountdownText));
	}
}

float AShooterPlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void AShooterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void AShooterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHUDValid = ShooterHUD &&
		ShooterHUD->CharacterOverlay &&
		ShooterHUD->CharacterOverlay->HealthText;
	if (bHUDValid)
	{
		FString HealthText = FString::Printf(TEXT("%d"), FMath::CeilToInt(Health));
		ShooterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void AShooterPlayerController::SetHUDAmmo(int32 Ammo)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHUDValid = ShooterHUD &&
		ShooterHUD->CharacterOverlay &&
		ShooterHUD->CharacterOverlay->AmmoText;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		ShooterHUD->CharacterOverlay->AmmoText->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeAmmo = true;
		HUDAmmo = Ammo;
	}
}

void AShooterPlayerController::SetHUDOwnerTeam(ETeam OwnerTeam)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHUDValid = ShooterHUD &&
		ShooterHUD->CharacterOverlay &&
		ShooterHUD->CharacterOverlay->OwnerTeamColor;
	if (bHUDValid)
	{
		ShooterHUD->CharacterOverlay->OwnerTeamColor->SetBrushTintColor(TeamColors[OwnerTeam]);
	}
	else
	{
		bInitializeOwnerTeam = true;
		HUDOwnerTeam = OwnerTeam;
	}
}

void AShooterPlayerController::SetHUDLeaderTeam(ETeam LeaderTeam)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHUDValid = ShooterHUD &&
		ShooterHUD->CharacterOverlay &&
		ShooterHUD->CharacterOverlay->LeaderTeamColor;
	if (bHUDValid)
	{
		ShooterHUD->CharacterOverlay->LeaderTeamColor->SetBrushTintColor(TeamColors[LeaderTeam]);
	}
	else
	{
		bInitializeLeaderTeam = true;
		HUDLeaderTeam = LeaderTeam;
	}
}

void AShooterPlayerController::SetHUDOwnerRank(int32 OwnerRank)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHUDValid = ShooterHUD &&
		ShooterHUD->CharacterOverlay &&
		ShooterHUD->CharacterOverlay->OwnerRankText;
	if (bHUDValid)
	{
		switch (OwnerRank)
		{
		case 1:
			ShooterHUD->CharacterOverlay->OwnerRankText->SetText(FText::FromString("1st"));
			break;
		case 2:
			ShooterHUD->CharacterOverlay->OwnerRankText->SetText(FText::FromString("2nd"));
			break;
		case 3:
			ShooterHUD->CharacterOverlay->OwnerRankText->SetText(FText::FromString("3rd"));
			break;
		case 4:
			ShooterHUD->CharacterOverlay->OwnerRankText->SetText(FText::FromString("4th"));
			break;
		}
	}
	else
	{
		bInitializeOwnerRank = true;
		HUDOwnerRank = OwnerRank;
	}
}

void AShooterPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;
}

void AShooterPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Ending)
	{
		HandleEnding();
	}
}

void AShooterPlayerController::HandleMatchHasStarted()
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	if (ShooterHUD)
	{
		if (ShooterHUD->CharacterOverlay == nullptr) ShooterHUD->AddCharacterOverlay();
		
		if (bTeamsMatch)
		{
			InitTeamScores();
		}
		else
		{
			HideTeamScores();
		}
	}
}

void AShooterPlayerController::InitTeamScores()
{
	/*
	 * Show team scores.
	 */
}

void AShooterPlayerController::HideTeamScores()
{
	/*
	 * Hide team scores.
	 */
}

void AShooterPlayerController::HandleEnding()
{
	/* 
	 * Should show the game result.
	*/
	
	AShooterCharacterBase* ShooterCharacter = Cast<AShooterCharacterBase>(GetPawn());
	if (ShooterCharacter && ShooterCharacter->GetCombat())
	{
		ShooterCharacter->bDisableGameplay = true;
		ShooterCharacter->GetCombat()->FireButtonPressed(false);
	}
}
