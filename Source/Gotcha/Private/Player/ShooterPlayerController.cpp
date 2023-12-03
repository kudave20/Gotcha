// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ShooterPlayerController.h"

AShooterPlayerController::AShooterPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	// bReplicates = true;
}

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
}

void AShooterPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
}