// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/GotchaGameMode.h"
#include "Character/ShooterCharacter.h"
#include "Game/GotchaGameState.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Player/ShooterPlayerController.h"
#include "Player/ShooterPlayerState.h"

namespace MatchState
{
	const FName Ending = FName("Ending");
}

void AGotchaGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void AGotchaGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (MatchState == MatchState::InProgress)
	{
		CountdownTime = MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Ending);
		}
	}
	else if (MatchState == MatchState::Ending)
	{
		CountdownTime = EndingTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

void AGotchaGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AShooterPlayerController* ShooterPlayer = Cast<AShooterPlayerController>(*It);
		if (ShooterPlayer)
		{
			ShooterPlayer->OnMatchStateSet(MatchState);
		}
	}
}

void AGotchaGameMode::PlayerEliminated(AShooterCharacterBase* ElimmedCharacter, AShooterPlayerController* VictimController, AShooterPlayerController* AttackerController)
{
	AShooterPlayerState* AttackerPlayerState = AttackerController ? Cast<AShooterPlayerState>(AttackerController->PlayerState) : nullptr;
	AGotchaGameState* GotchaGameState = GetGameState<AGotchaGameState>();
	if (GotchaGameState && AttackerPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);
		GotchaGameState->UpdateTopScore(AttackerPlayerState);
	}
	
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim(false);
	}
}

void AGotchaGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}
