// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/TeamGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Game/GotchaGameState.h"
#include "Player/GotchaPlayerState.h"

void ATeamGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AGotchaGameState* GGameState = Cast<AGotchaGameState>(UGameplayStatics::GetGameState(this));
	if (GGameState)
	{
		AGotchaPlayerState* GPState = NewPlayer->GetPlayerState<AGotchaPlayerState>();
		if (GPState && GPState->GetTeam() == ETeam::ET_NoTeam)
		{
			int32 MinNumber = TeamMemberLimit;
			ETeam TeamToJoin = ETeam::ET_NoTeam;
			
			if (GGameState->Teams[ETeam::ET_RedTeam].Num() < MinNumber)
			{
				MinNumber = GGameState->Teams[ETeam::ET_RedTeam].Num();
				TeamToJoin = ETeam::ET_RedTeam;
			}
			if (GGameState->Teams[ETeam::ET_BlueTeam].Num() < MinNumber)
			{
				MinNumber = GGameState->Teams[ETeam::ET_BlueTeam].Num();
				TeamToJoin = ETeam::ET_BlueTeam;
			}
			if (GGameState->Teams[ETeam::ET_GreenTeam].Num() < MinNumber)
			{
				MinNumber = GGameState->Teams[ETeam::ET_GreenTeam].Num();
				TeamToJoin = ETeam::ET_GreenTeam;
			}
			if (GGameState->Teams[ETeam::ET_YellowTeam].Num() < MinNumber)
			{
				TeamToJoin = ETeam::ET_YellowTeam;
			}

			GGameState->Teams[TeamToJoin].AddUnique(GPState);
			GPState->SetTeam(TeamToJoin);
		}
	}
}

void ATeamGameMode::Logout(AController* Exiting)
{
	AGotchaGameState* GGameState = Cast<AGotchaGameState>(UGameplayStatics::GetGameState(this));
	AGotchaPlayerState* GPState = Exiting->GetPlayerState<AGotchaPlayerState>();
	if (GGameState && GPState)
	{
		if (GGameState->Teams[ETeam::ET_RedTeam].Contains(GPState))
		{
			GGameState->Teams[ETeam::ET_RedTeam].Remove(GPState);
		}
		if (GGameState->Teams[ETeam::ET_BlueTeam].Contains(GPState))
		{
			GGameState->Teams[ETeam::ET_BlueTeam].Remove(GPState);
		}
		if (GGameState->Teams[ETeam::ET_GreenTeam].Contains(GPState))
		{
			GGameState->Teams[ETeam::ET_GreenTeam].Remove(GPState);
		}
		if (GGameState->Teams[ETeam::ET_YellowTeam].Contains(GPState))
		{
			GGameState->Teams[ETeam::ET_YellowTeam].Remove(GPState);
		}
	}
}