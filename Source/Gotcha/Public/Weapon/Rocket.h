// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile.h"
#include "Rocket.generated.h"

class URocketMovementComponent;

/**
 * 
 */
UCLASS()
class GOTCHA_API ARocket : public AProjectile
{
	GENERATED_BODY()
	
public:
	ARocket();
	
protected:
	virtual void BeginPlay() override;

	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<URocketMovementComponent> RocketMovementComponent;
	
	UPROPERTY(EditAnywhere, Category = "Effect")
	TObjectPtr<USoundCue> ProjectileLoop;

	UPROPERTY()
	TObjectPtr<UAudioComponent> ProjectileLoopComponent;

	UPROPERTY(EditAnywhere, Category = "Effect")
	TObjectPtr<USoundAttenuation> LoopingSoundAttenuation;

};
