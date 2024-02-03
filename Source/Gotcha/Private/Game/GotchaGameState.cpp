// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/GotchaGameState.h"
#include "Character/ShooterCharacterBase.h"
#include "Game/GotchaGameMode.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Player/ShooterPlayerState.h"
#include "Player/ShooterPlayerController.h"

AGotchaGameState::AGotchaGameState()
{
	PrimaryActorTick.bCanEverTick = true;
	
	TeamScores.Add(ETeam::ET_RedTeam, 0.f);
	TeamScores.Add(ETeam::ET_BlueTeam, 0.f);
	TeamScores.Add(ETeam::ET_GreenTeam, 0.f);
	TeamScores.Add(ETeam::ET_YellowTeam, 0.f);

	Teams.Add(ETeam::ET_RedTeam, FTeam());
	Teams.Add(ETeam::ET_BlueTeam, FTeam());
	Teams.Add(ETeam::ET_GreenTeam, FTeam());
	Teams.Add(ETeam::ET_YellowTeam, FTeam());

	TeamRanks.Add(ETeam::ET_RedTeam, 1);
	TeamRanks.Add(ETeam::ET_BlueTeam, 1);
	TeamRanks.Add(ETeam::ET_GreenTeam, 1);
	TeamRanks.Add(ETeam::ET_YellowTeam, 1);

	TeamsAtRank.Add(1, FTeams());
	TeamsAtRank.Add(2, FTeams());
	TeamsAtRank.Add(3, FTeams());
	TeamsAtRank.Add(4, FTeams());

	TeamElimCounts.Add(ETeam::ET_RedTeam, 0);
	TeamElimCounts.Add(ETeam::ET_BlueTeam, 0);
	TeamElimCounts.Add(ETeam::ET_GreenTeam, 0);
	TeamElimCounts.Add(ETeam::ET_YellowTeam, 0);

	TeamControlPercentage.Add(ETeam::ET_RedTeam, 0.f);
	TeamControlPercentage.Add(ETeam::ET_BlueTeam, 0.f);
	TeamControlPercentage.Add(ETeam::ET_GreenTeam, 0.f);
	TeamControlPercentage.Add(ETeam::ET_YellowTeam, 0.f);
}

void AGotchaGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ControlPoint(DeltaSeconds);
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

void AGotchaGameState::ScoreTeam(ETeam ScoringTeam, uint32 NumberOfTeams)
{
	++TeamScores[ScoringTeam];

	TeamsAtRank.Empty();
	for (uint32 i = 1; i <= NumberOfTeams; i++)
	{
		TeamsAtRank.Add(i, FTeams());
	}
	
	TeamScores.ValueSort([](float A, float B)
	{
		return A > B;
	});
		
	uint32 Rank = 1;
	float PreviousScore = 0.f;
	for (auto TeamScore : TeamScores)
	{
		ETeam Team = TeamScore.Key;
		float Score = TeamScore.Value;
		if (Score == PreviousScore)
		{
			TeamRanks[Team] = Rank;
			TeamsAtRank[Rank].Teams.AddUnique(Team);
		}
		else if (Score < PreviousScore || PreviousScore == 0.f)
		{
			Rank += TeamsAtRank[Rank].Teams.Num();
			TeamRanks[Team] = Rank;
			TeamsAtRank[Rank].Teams.AddUnique(Team);
			PreviousScore = Score;
		}
	}

	for (auto TeamTuple : Teams)
	{
		ETeam Team = TeamTuple.Key;
		TArray<APlayerState*> TeamMembers = TeamTuple.Value.Members;
		for (auto TeamMember : TeamMembers)
		{
			AShooterPlayerState* SPState = Cast<AShooterPlayerState>(TeamMember);
			if (SPState)
			{
				SPState->SetTeamRank(TeamRanks[Team]);
			}
		}
	}

	LeaderTeam = TeamsAtRank[1].Teams[0];
}

void AGotchaGameState::RemoveFromTeam(AShooterPlayerState* PlayerToRemove)
{
	if (Teams[PlayerToRemove->GetTeam()].Members.Contains(PlayerToRemove))
	{
		Teams[PlayerToRemove->GetTeam()].Members.Remove(PlayerToRemove);
	}
}

void AGotchaGameState::CountTeamElim(ETeam Team, float TeamRespawnTime)
{
	TeamElimCounts[Team] = FMath::Clamp(TeamElimCounts[Team] + 1, 0, Teams[Team].Members.Num());
	if (TeamElimCounts[Team] >= Teams[Team].Members.Num())
	{
		for (auto Member : Teams[Team].Members)
		{
			AShooterCharacterBase* ShooterCharacter = Cast<AShooterCharacterBase>(Member->GetPawn());
			if (ShooterCharacter)
			{
				ShooterCharacter->ClientResetForTeamRespawn();
			}
		}
		
		FTimerHandle TeamRespawnTimer;
		FTimerDelegate TeamRespawnDelegate = FTimerDelegate::CreateUObject(this, &AGotchaGameState::RespawnTeam, Team);
		GetWorldTimerManager().SetTimer(
			TeamRespawnTimer,
			TeamRespawnDelegate,
			TeamRespawnTime,
			false
			);
	}
}

void AGotchaGameState::RespawnTeam(ETeam TeamToRespawn)
{
	AGotchaGameMode* GotchaGameMode = GetWorld()->GetAuthGameMode<AGotchaGameMode>();
	if (GotchaGameMode)
	{
		for (auto Member : Teams[TeamToRespawn].Members)
		{
			ACharacter* Character = Cast<ACharacter>(Member->GetPawn());
			if (Character)
			{
				GotchaGameMode->RequestRespawn(Character, Character->Controller);
			}
		}
		TeamElimCounts[TeamToRespawn] = 0;
	}
}

void AGotchaGameState::ControlPoint(float DeltaTime)
{
	if (bControlEnabled && ControllingTeams.Num() == 1)
	{
		TeamControlPercentage[ControllingTeams[0]] = FMath::Clamp(TeamControlPercentage[ControllingTeams[0]] + ControlSpeed * DeltaTime, 0.f, 1.f);
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
