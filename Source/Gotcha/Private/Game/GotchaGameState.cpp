// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/GotchaGameState.h"
#include "Net/UnrealNetwork.h"
#include "Player/ShooterPlayerState.h"
#include "Player/ShooterPlayerController.h"

AGotchaGameState::AGotchaGameState()
{
	TeamScores.Add(ETeam::ET_RedTeam, 0.f);
	TeamScores.Add(ETeam::ET_BlueTeam, 0.f);
	TeamScores.Add(ETeam::ET_GreenTeam, 0.f);
	TeamScores.Add(ETeam::ET_YellowTeam, 0.f);

	Teams.Add(ETeam::ET_RedTeam, TArray<APlayerState*>());
	Teams.Add(ETeam::ET_BlueTeam, TArray<APlayerState*>());
	Teams.Add(ETeam::ET_GreenTeam, TArray<APlayerState*>());
	Teams.Add(ETeam::ET_YellowTeam, TArray<APlayerState*>());

	TeamRanks.Add(ETeam::ET_RedTeam, 1);
	TeamRanks.Add(ETeam::ET_BlueTeam, 1);
	TeamRanks.Add(ETeam::ET_GreenTeam, 1);
	TeamRanks.Add(ETeam::ET_YellowTeam, 1);

	TeamsAtRank.Add(1, TArray<ETeam>());
	TeamsAtRank.Add(2, TArray<ETeam>());
	TeamsAtRank.Add(3, TArray<ETeam>());
	TeamsAtRank.Add(4, TArray<ETeam>());
}

void AGotchaGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGotchaGameState, LeaderTeam);
}

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

void AGotchaGameState::ScoreTeam(ETeam ScoringTeam, int32 NumberOfTeams)
{
	++TeamScores[ScoringTeam];

	TeamsAtRank.Empty();
	for (int32 Index = 1; Index <= NumberOfTeams; ++Index)
	{
		TeamsAtRank.Add(Index, TArray<ETeam>());
	}
	
	TeamScores.ValueSort([](float A, float B)
	{
		return A > B;
	});
		
	int32 Rank = 1;
	float PreviousScore = 0.f;
	for (auto TeamScore : TeamScores)
	{
		ETeam Team = TeamScore.Key;
		float Score = TeamScore.Value;
		if (Score == PreviousScore)
		{
			TeamRanks[Team] = Rank;
			TeamsAtRank[Rank].AddUnique(Team);
		}
		else if (Score < PreviousScore || PreviousScore == 0.f)
		{
			Rank += TeamsAtRank[Rank].Num();
			TeamRanks[Team] = Rank;
			TeamsAtRank[Rank].AddUnique(Team);
			PreviousScore = Score;
		}
	}

	for (auto TeamTuple : Teams)
	{
		ETeam Team = TeamTuple.Key;
		TArray<APlayerState*> TeamMembers = TeamTuple.Value;
		for (auto TeamMember : TeamMembers)
		{
			AShooterPlayerState* SPState = Cast<AShooterPlayerState>(TeamMember);
			if (SPState)
			{
				SPState->SetTeamRank(TeamRanks[Team]);
			}
		}
	}

	LeaderTeam = TeamsAtRank[1][0];
}

void AGotchaGameState::RemoveFromTeam(AShooterPlayerState* PlayerToRemove)
{
	if (Teams[PlayerToRemove->GetTeam()].Contains(PlayerToRemove))
	{
		Teams[PlayerToRemove->GetTeam()].Remove(PlayerToRemove);
	}
}

void AGotchaGameState::OnRep_LeaderTeam()
{
	AShooterPlayerController* ShooterPlayer = Cast<AShooterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (ShooterPlayer)
	{
		ShooterPlayer->SetHUDLeaderTeam(LeaderTeam);
	}
}
