// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

#define TRACE_LENGTH 80000.f

class UTexture2D;
class UAnimationAsset;

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
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = "Properties")
	float ZoomInterpSpeed = 20.f;

	UPROPERTY(EditAnywhere, Category = "Properties")
	float FireDelay = 1.f;

	UPROPERTY(EditAnywhere, Category = "Properties")
	bool bAutomatic;

	UPROPERTY(EditAnywhere, Category = "Properties")
	bool bIsMelee;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;
	
	void SpendRound();

	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimationAsset> FireAnimation;

	UPROPERTY(Replicated)
	int32 Ammo;

	UPROPERTY(EditAnywhere, Category = "Properties")
	int32 MagCapacity;
	
public:	
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	bool IsEmpty();
};
