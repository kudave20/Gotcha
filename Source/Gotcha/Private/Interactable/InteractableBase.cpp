// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/InteractableBase.h"

AInteractableBase::AInteractableBase()
{
	PrimaryActorTick.bCanEverTick = true;

}

void AInteractableBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void AInteractableBase::OnInteract(APlayerController* Player)
{
	
}

