// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GotchaType/WeaponTypes.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

#define TRACE_LENGTH 2000.f

class UTexture2D;
class UAnimationAsset;
class AShooterCharacterBase;

UCLASS()
class GOTCHA_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Fire(const FVector& HitTarget);
	void Reload();
	
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	TObjectPtr<UTexture2D> CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	TObjectPtr<UTexture2D> CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	TObjectPtr<UTexture2D> CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	TObjectPtr<UTexture2D> CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	TObjectPtr<UTexture2D> CrosshairsBottom;

	UPROPERTY(EditAnywhere, Category = "Properties")
	float FireDelay = 1.f;

	UPROPERTY(EditAnywhere, Category = "Properties")
	bool bAutomatic;

	UPROPERTY(EditAnywhere, Category = "Properties")
	bool bIsMelee;

	UPROPERTY(EditAnywhere, Category = "Properties")
	bool bUseScatter;

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 75.f;
	
	UPROPERTY(EditAnywhere, Category = "Properties")
	float Damage = 10.f;

	UPROPERTY()
	AShooterCharacterBase* OwnerCharacter;

private:
	void SpendRound();

	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimationAsset> FireAnimation;

	UPROPERTY(ReplicatedUsing = OnRep_Ammo)
	int32 Ammo;

	UFUNCTION()
	void OnRep_Ammo();

	UPROPERTY(EditAnywhere, Category = "Properties")
	int32 MagCapacity;

	UPROPERTY(EditAnywhere, Category = "Properties")
	EWeaponType WeaponType;
	
	void SetHUDAmmo();
	
public:	
	FORCEINLINE TObjectPtr<UStaticMeshComponent> GetWeaponMesh() const { return WeaponMesh; }
	bool IsEmpty();
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
};
