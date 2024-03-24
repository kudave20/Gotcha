// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Weapon.h"
#include "Character/ShooterCharacterBase.h"
#include "Engine/StaticMeshSocket.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	// SetReplicateMovement(true);

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->CastShadow = false;
	WeaponMesh->SetIsReplicated(true);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	Ammo = MagCapacity;
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, Ammo);
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (bIsMelee)
	{
		return;
	}
	
	// if (FireAnimation && WeaponMesh)
	// {
	// 	WeaponMesh->PlayAnimation(FireAnimation, false);
	// }
	
	SpendRound();
}

void AWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	if (!HasAuthority())
	{
		SetHUDAmmo();
	}
}

void AWeapon::Reload()
{
	Ammo = MagCapacity;
}

void AWeapon::OnRep_Ammo()
{
	SetHUDAmmo();
}

FVector AWeapon::TraceEndWithScatter(const FVector& HitTarget)
{
	const UStaticMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlashSocket");
	if (MuzzleFlashSocket == nullptr) return FVector();

	FTransform SocketTransform;
	MuzzleFlashSocket->GetSocketTransform(SocketTransform, GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	const FVector EndLoc = SphereCenter + RandVec;
	const FVector ToEndLoc = EndLoc - TraceStart;

	return FVector(TraceStart + ToEndLoc * TraceLength / ToEndLoc.Size());
}

void AWeapon::SetHUDAmmo()
{
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<AShooterCharacterBase>(GetOwner()) : OwnerCharacter;
	if (OwnerCharacter && OwnerCharacter->IsLocallyControlled())
	{
		OwnerCharacter->UpdateHUDAmmo();
	}
}

bool AWeapon::IsEmpty()
{
	return !bIsMelee && Ammo <= 0;
}

