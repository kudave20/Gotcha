// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/MeleeWeapon.h"
#include "Flag.generated.h"

class USphereComponent;

/**
 * 
 */
UCLASS()
class GOTCHA_API AFlag : public AMeleeWeapon
{
	GENERATED_BODY()

public:
	AFlag();

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
		);
	
private:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> PickupArea;
	
};
