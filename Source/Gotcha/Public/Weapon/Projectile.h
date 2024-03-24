// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UBoxComponent;
class UParticleSystem;
class USoundCue;

UCLASS()
class GOTCHA_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:
	AProjectile();
	virtual void Destroyed() override;

	UPROPERTY(EditAnywhere, Category = "Properties")
	float InitialSpeed = 15000.f;

	float Damage = 20.f;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	void StartDestroyTimer();
	void DestroyTimerFinished();
	void ExplodeDamage();

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UBoxComponent> CollisionBox;
	
	UPROPERTY(EditAnywhere, Category = "Effect")
	TObjectPtr<UParticleSystem> ImpactParticle;

	UPROPERTY(EditAnywhere, Category = "Effect")
	TObjectPtr<USoundCue> ImpactSound;

	UPROPERTY(EditAnywhere, Category = "Properties")
	float DamageInnerRadius = 200.f;

	UPROPERTY(EditAnywhere, Category = "Properties")
	float DamageOuterRadius = 500.f;

	UPROPERTY(EditAnywhere, Category = "Effect")
	TObjectPtr<UParticleSystem> TrailSystem;

	UPROPERTY()
	TObjectPtr<UParticleSystemComponent> TrailSystemComponent;
	
private:
	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere, Category = "Properties")
	float DestroyTime = 3.f;

public:

};
