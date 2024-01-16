// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ShooterPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Player/ShooterPlayerController.h"

void AShooterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterPlayerState, Team);
	DOREPLIFETIME_CONDITION(AShooterPlayerState, TeamRank, COND_OwnerOnly);
}

void AShooterPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
}

void AShooterPlayerState::OnRep_TeamRank()
{
	AShooterPlayerController* ShooterPlayer = Cast<AShooterPlayerController>(GetPlayerController());
	if (ShooterPlayer)
	{
		ShooterPlayer->SetHUDOwnerRank(TeamRank);
	}
}

void AShooterPlayerState::SetTeam(ETeam TeamToSet)
{
	Team = TeamToSet;
}
