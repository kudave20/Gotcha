// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ShooterPlayerController.h"
#include "HUD/ShooterHUD.h"
#include "HUD/CharacterOverlay.h"
#include "Components/TextBlock.h"

AShooterPlayerController::AShooterPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	// bReplicates = true;
}

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ShooterHUD = Cast<AShooterHUD>(GetHUD());
}

void AShooterPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
}

void AShooterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHUDValid = ShooterHUD &&
		ShooterHUD->CharacterOverlay &&
		ShooterHUD->CharacterOverlay->HealthText;
	if (bHUDValid)
	{
		FString HealthText = FString::Printf(TEXT("%d"), FMath::CeilToInt(Health));
		ShooterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}