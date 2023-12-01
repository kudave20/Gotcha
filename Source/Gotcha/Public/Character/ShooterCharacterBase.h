// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacterBase.generated.h"

class UCameraComponent;
class AWeapon;
class AShooterPlayerController;

UCLASS()
class GOTCHA_API AShooterCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	AShooterCharacterBase();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	AWeapon* EquipInitialWeapon();
	void SetCollisionBetweenCharacter(const ECollisionResponse NewResponse);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditAnywhere, Category = "Properties")
	TSubclassOf<AWeapon> PrimaryGunClass;

	UPROPERTY()
	AWeapon* PrimaryGun;

private:
	UPROPERTY()
	AShooterPlayerController* PlayerController;
	
	UFUNCTION(Server, Reliable)
	void ServerSetCollisionBetweenCharacter(const ECollisionResponse NewResponse);
	
public:	
	FORCEINLINE TObjectPtr<UCameraComponent> GetCamera() const { return Camera; }

};
