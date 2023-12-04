// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/HitScanWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
}

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if (World)
	{
		// TArray<AActor*> ActorsToIgnore;
		// ActorsToIgnore.Add(GetOwner());
		// UKismetSystemLibrary::LineTraceSingle(
		// 	this,
		// 	TraceStart,
		// 	HitTarget,
		// 	UEngineTypes::ConvertToTraceType(ECC_Visibility),
		// 	false,
		// 	ActorsToIgnore,
		// 	EDrawDebugTrace::Persistent,
		// 	OutHit,
		// 	false
		// );
		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			HitTarget,
			ECC_Visibility
		);
		FVector BeamEnd = HitTarget;
		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}
		else
		{
			OutHit.ImpactPoint = HitTarget;
		}

		if (BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamParticles,
				TraceStart,
				FRotator::ZeroRotator,
				true
			);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}
