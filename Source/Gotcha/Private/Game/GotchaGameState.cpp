// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/GotchaGameState.h"
#include "Player/ShooterPlayerState.h"

void AGotchaGameState::UpdateTopScore(AShooterPlayerState* ScoringPlayer)
{
	if (TopPlayers.Num() == 0)
	{
		TopPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopPlayers.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		TopPlayers.Empty();
		TopPlayers.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}

void AGotchaGameState::ScoreTeam(ETeam Team)
{
	++TeamScores[Team];

	if (TopTeams.Num() == 0)
	{
		TopTeams.Add(Team);
		TopTeamScore = TeamScores[Team];
	}
	else if (TeamScores[Team] == TopTeamScore)
	{
		TopTeams.AddUnique(Team);
	}
	else if (TeamScores[Team] > TopTeamScore)
	{
		TopTeams.Empty();
		TopTeams.AddUnique(Team);
		TopTeamScore = TeamScores[Team];
	}
}

void AGotchaGameState::RemoveFromTeam(AShooterPlayerState* PlayerToRemove)
{
	if (Teams[PlayerToRemove->GetTeam()].Contains(PlayerToRemove))
	{
		Teams[PlayerToRemove->GetTeam()].Remove(PlayerToRemove);
	}
}
