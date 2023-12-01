// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HUD/ShooterHUD.h"
#include "CombatComponent.generated.h"

class AShooterCharacterBase;
class AShooterPlayerController;
class AShooterHUD;
class AWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GOTCHA_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	friend class AShooterPlayerController;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void SwapWeapons();
	void Reload();

protected:
	virtual void BeginPlay() override;

	void SetHUDCrosshairs(float DeltaTime);

private:
	UPROPERTY()
	AShooterCharacterBase* Character;
	
	UPROPERTY()
	AShooterPlayerController* Controller;

	UPROPERTY()
	AShooterHUD* HUD;

	UPROPERTY(Replicated)
	AWeapon* EquippedWeapon;

	FHUDPackage HUDPackage;

	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;
	
public:	
	

		
};
