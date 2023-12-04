// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Weapon.h"
#include "HitScanWeapon.generated.h"

class UParticleSystem;
class USoundCue;

/**
 * 
 */
UCLASS()
class GOTCHA_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	virtual void Fire(const FVector& HitTarget) override;

protected:
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);

	UPROPERTY(EditAnywhere, Category = "Effect")
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere, Category = "Effect")
	USoundCue* HitSound;

private:
	UPROPERTY(EditAnywhere, Category = "Effect")
	UParticleSystem* BeamParticles;

	UPROPERTY(EditAnywhere, Category = "Effect")
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, Category = "Effect")
	USoundCue* FireSound;
};
