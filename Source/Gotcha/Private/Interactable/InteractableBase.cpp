// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/InteractableBase.h"

AInteractableBase::AInteractableBase()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
}

void AInteractableBase::OnInteract(APlayerController* Player)
{
	
}

