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
	friend class AShooterCharacterBase;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void FireButtonPressed(bool bPressed);
	void SwapWeapons();
	void Reload();

	void EquipWeapon(AWeapon* PrimaryGun, AWeapon* SecondaryGun);

protected:
	virtual void BeginPlay() override;
	
	void Fire();
	void FireProjectileWeapon();
	void FireShotgun();
	void FireMeleeWeapon();
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	void ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTargets);
	void MeleeLocalFire();

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerMeleeFire(float FireDelay);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastMeleeFire();
	
	UFUNCTION(Server, Reliable)
	void ServerSwapWeapons();

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);
	
	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();

private:
	UPROPERTY()
	AShooterCharacterBase* Character;
	
	UPROPERTY()
	AShooterPlayerController* Controller;

	UPROPERTY()
	AShooterHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	AWeapon* SecondaryWeapon;

	UFUNCTION()
	void OnRep_EquippedWeapon();
	UFUNCTION()
	void OnRep_SecondaryWeapon();
	
	void AttachToArm(AWeapon* WeaponToAttach);
	void AttachToMesh(AWeapon* WeaponToAttach);

	FHUDPackage HUDPackage;

	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;
	
	FVector HitTarget;

	FTimerHandle FireTimer;
	bool bCanFire = true;
	bool bFireButtonPressed;

	void StartFireTimer();
	void FireTimerFinished();

	bool CanFire();

	UPROPERTY(Replicated)
	bool bHoldingFlag = false;
	
};
