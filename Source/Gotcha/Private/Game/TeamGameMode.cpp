// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/TeamGameMode.h"
#include "Game/GotchaGameState.h"
#include "Player/ShooterPlayerState.h"
#include "Player/ShooterPlayerController.h"

ATeamGameMode::ATeamGameMode()
{
	bTeamsMatch = true;
}

void ATeamGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AGotchaGameState* GGameState = Cast<AGotchaGameState>(GameState);
	if (GGameState)
	{
		AShooterPlayerState* SPState = NewPlayer->GetPlayerState<AShooterPlayerState>();
		if (SPState && SPState->GetTeam() == ETeam::ET_NoTeam)
		{
			int32 MinNumber = TeamMemberLimit;
			ETeam TeamToJoin = ETeam::ET_NoTeam;

			if (!GGameState->Teams.Contains(ETeam::ET_YellowTeam))
			{
				MinNumber = 0;
				TeamToJoin = ETeam::ET_YellowTeam;
			}
			else if (GGameState->Teams[ETeam::ET_YellowTeam].Members.Num() < MinNumber)
			{
				MinNumber = GGameState->Teams[ETeam::ET_YellowTeam].Members.Num();
				TeamToJoin = ETeam::ET_YellowTeam;
			}
			if (!GGameState->Teams.Contains(ETeam::ET_GreenTeam))
			{
				MinNumber = 0;
				TeamToJoin = ETeam::ET_GreenTeam;
			}
			else if (GGameState->Teams[ETeam::ET_GreenTeam].Members.Num() < MinNumber)
			{
				MinNumber = GGameState->Teams[ETeam::ET_GreenTeam].Members.Num();
				TeamToJoin = ETeam::ET_GreenTeam;
			}
			if (!GGameState->Teams.Contains(ETeam::ET_BlueTeam))
			{
				MinNumber = 0;
				TeamToJoin = ETeam::ET_BlueTeam;
			}
			else if (GGameState->Teams[ETeam::ET_BlueTeam].Members.Num() < MinNumber)
			{
				MinNumber = GGameState->Teams[ETeam::ET_BlueTeam].Members.Num();
				TeamToJoin = ETeam::ET_BlueTeam;
			}
			if (!GGameState->Teams.Contains(ETeam::ET_RedTeam))
			{
				// MinNumber = 0;
				TeamToJoin = ETeam::ET_RedTeam;
			}
			else if (GGameState->Teams[ETeam::ET_RedTeam].Members.Num() < MinNumber)
			{
				// MinNumber = GGameState->Teams[ETeam::ET_RedTeam].Members.Num();
				TeamToJoin = ETeam::ET_RedTeam;
			}

			if (TeamToJoin == ETeam::ET_NoTeam)
			{
				/*
				 * Make player spectator.
				 */
			}
			else
			{
				GGameState->Teams[TeamToJoin].Members.AddUnique(SPState);
				GGameState->TeamsAtRank[GGameState->TeamRanks[TeamToJoin]].Teams.AddUnique(TeamToJoin);
				SPState->SetTeam(TeamToJoin);
				SPState->SetTeamRank(GGameState->TeamRanks[TeamToJoin]);
			}
		}
	}
}

void ATeamGameMode::Logout(AController* Exiting)
{
	AGotchaGameState* GGameState = Cast<AGotchaGameState>(GameState);
	AShooterPlayerState* SPState = Exiting->GetPlayerState<AShooterPlayerState>();
	if (GGameState && SPState)
	{
		GGameState->RemoveFromTeam(SPState);
	}
}

void ATeamGameMode::PlayerEliminated(AShooterCharacterBase* ElimmedCharacter, AShooterPlayerController* VictimController, AShooterPlayerController* AttackerController)
{
	Super::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);

	AGotchaGameState* GGameState = Cast<AGotchaGameState>(GameState);
	AShooterPlayerState* AttackerPlayerState = AttackerController ? Cast<AShooterPlayerState>(AttackerController->PlayerState) : nullptr;
	AShooterPlayerState* VictimPlayerState = VictimController ? Cast<AShooterPlayerState>(VictimController->PlayerState) : nullptr;
	if (GGameState)
	{
		if (AttackerPlayerState)
		{
			GGameState->ScoreTeam(AttackerPlayerState->GetTeam(), NumberOfTeams);
		}
		if (VictimPlayerState)
		{
			GGameState->CountTeamElim(VictimPlayerState->GetTeam(), TeamRespawnTime);
		}
	}
}
