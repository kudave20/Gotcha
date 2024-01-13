// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ShooterPlayerState.h"

void AShooterPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
}

void AShooterPlayerState::SetTeam(ETeam TeamToSet)
{
	Team = TeamToSet;
}
