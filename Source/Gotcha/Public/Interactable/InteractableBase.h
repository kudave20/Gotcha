// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractableInterface.h"
#include "InteractableBase.generated.h"

UCLASS()
class GOTCHA_API AInteractableBase : public AActor, public IInteractableInterface
{
	GENERATED_BODY()
	
public:
	AInteractableBase();

protected:
	virtual void OnInteract(APlayerController* Player) override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

};
