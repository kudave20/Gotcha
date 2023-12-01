// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacterBase.generated.h"

class UCameraComponent;

UCLASS()
class GOTCHA_API AShooterCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	AShooterCharacterBase();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void SetCollisionBetweenCharacter(const ECollisionResponse NewResponse);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* Camera;

private:
	UFUNCTION(Server, Reliable)
	void ServerSetCollisionBetweenCharacter(const ECollisionResponse NewResponse);
	
public:	
	FORCEINLINE UCameraComponent* GetCamera() const { return Camera; }

};
