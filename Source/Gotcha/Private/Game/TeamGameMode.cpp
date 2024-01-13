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

			if (!GGameState->Teams.Contains(ETeam::ET_RedTeam))
			{
				MinNumber = 0;
				TeamToJoin = ETeam::ET_RedTeam;
			}
			else if (GGameState->Teams[ETeam::ET_RedTeam].Num() < MinNumber)
			{
				MinNumber = GGameState->Teams[ETeam::ET_RedTeam].Num();
				TeamToJoin = ETeam::ET_RedTeam;
			}
			if (!GGameState->Teams.Contains(ETeam::ET_BlueTeam))
			{
				MinNumber = 0;
				TeamToJoin = ETeam::ET_BlueTeam;
			}
			else if (GGameState->Teams[ETeam::ET_BlueTeam].Num() < MinNumber)
			{
				MinNumber = GGameState->Teams[ETeam::ET_BlueTeam].Num();
				TeamToJoin = ETeam::ET_BlueTeam;
			}
			if (!GGameState->Teams.Contains(ETeam::ET_GreenTeam))
			{
				MinNumber = 0;
				TeamToJoin = ETeam::ET_GreenTeam;
			}
			else if (GGameState->Teams[ETeam::ET_GreenTeam].Num() < MinNumber)
			{
				MinNumber = GGameState->Teams[ETeam::ET_GreenTeam].Num();
				TeamToJoin = ETeam::ET_GreenTeam;
			}
			if (!GGameState->Teams.Contains(ETeam::ET_YellowTeam))
			{
				// MinNumber = 0;
				TeamToJoin = ETeam::ET_YellowTeam;
			}
			else if (GGameState->Teams[ETeam::ET_YellowTeam].Num() < MinNumber)
			{
				// MinNumber = GGameState->Teams[ETeam::ET_YellowTeam].Num();
				TeamToJoin = ETeam::ET_YellowTeam;
			}

			if (TeamToJoin == ETeam::ET_NoTeam)
			{
				/*
				 * Make player spectator.
				 */
			}
			else
			{
				GGameState->Teams[TeamToJoin].AddUnique(SPState);
				SPState->SetTeam(TeamToJoin);
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
	if (GGameState && AttackerPlayerState)
	{
		GGameState->ScoreTeam(AttackerPlayerState->GetTeam());
	}
}
