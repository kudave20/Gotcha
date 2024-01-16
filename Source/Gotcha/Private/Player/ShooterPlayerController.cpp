// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ShooterPlayerController.h"
#include "HUD/ShooterHUD.h"
#include "HUD/CharacterOverlay.h"
#include "Components/TextBlock.h"
#include "Game/GotchaGameMode.h"
#include "Character/ShooterCharacterBase.h"
#include "Component/CombatComponent.h"
#include "Components/Image.h"

AShooterPlayerController::AShooterPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ShooterHUD = Cast<AShooterHUD>(GetHUD());
}

void AShooterPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	PollInit();
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

void AShooterPlayerController::OnMatchStateSet(FName State, bool bTeamsMatch)
{
	MatchState = State;

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted(bTeamsMatch);
	}
	else if (MatchState == MatchState::Ending)
	{
		HandleEnding();
	}
}

void AShooterPlayerController::HandleMatchHasStarted(bool bTeamsMatch)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	if (ShooterHUD)
	{
		if (ShooterHUD->CharacterOverlay == nullptr) ShooterHUD->AddCharacterOverlay();
		if (!HasAuthority()) return;
		
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
