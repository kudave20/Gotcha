// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ControlGameMode.h"
#include "Game/GotchaGameState.h"
#include "Player/ShooterPlayerState.h"

void AControlGameMode::PlayerEliminated(AShooterCharacterBase* ElimmedCharacter, AShooterPlayerController* VictimController, AShooterPlayerController* AttackerController)
{
	AGotchaGameMode::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);
}

void AControlGameMode::ControlPoint(AShooterPlayerState* ControllingPlayer, AShooterPlayerState* ExitingPlayer)
{	
	AGotchaGameState* GGameState = Cast<AGotchaGameState>(GameState);
	if (GGameState)
	{
		if (ControllingPlayer)
		{
			ControllingPlayer->Control(true);
			GGameState->ControllingTeams.AddUnique(ControllingPlayer->GetTeam());
			GGameState->ControlSpeed = 1.f / TimeToFullyControl;
			if (GGameState->ControllingTeams.Num() <= 1)
			{
				GGameState->bControlEnabled = true;
			}
			else
			{
				GGameState->bControlEnabled = false;
			}
		}
		else if (ExitingPlayer)
		{
			ExitingPlayer->Control(false);
			
			ETeam ExitingTeam = ExitingPlayer->GetTeam();
			for (APlayerState* Member : GGameState->Teams[ExitingTeam].Members)
			{
				AShooterPlayerState* SPState = Cast<AShooterPlayerState>(Member);
				if (SPState && SPState->IsControlling())
				{
					return;
				}
			}
			
			GGameState->ControllingTeams.Remove(ExitingTeam);
			if (GGameState->ControllingTeams.Num() == 1)
			{
				GGameState->bControlEnabled = true;
			}
		}
	}
}
