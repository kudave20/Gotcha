// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactable/InteractableBase.h"
#include "FlagContainer.generated.h"

/**
 * 
 */
UCLASS()
class GOTCHA_API AFlagContainer : public AInteractableBase
{
	GENERATED_BODY()

protected:
	virtual void OnInteract(APlayerController* Player) override;
	
};
