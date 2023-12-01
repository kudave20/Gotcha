// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class UTexture2D;

UCLASS()
class GOTCHA_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	
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

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	// UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo, Category = "Properties")
	// int32 Ammo;
	//
	// UFUNCTION()
	// void OnRep_Ammo();
	//
	// void SpendRound();

	UPROPERTY(EditAnywhere, Category = "Properties")
	int32 MagCapacity;
	
public:	
	

};
