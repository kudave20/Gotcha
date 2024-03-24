// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/CombatComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/ShooterCharacterBase.h"
#include "Components/BoxComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Player/ShooterPlayerController.h"
#include "Weapon/Weapon.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/MeleeWeapon.h"
#include "Weapon/Shotgun.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;
		
		SetHUDCrosshairs(DeltaTime);
	}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UCombatComponent, bHoldingFlag);
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::Fire()
{
	if (CanFire())
	{
		bCanFire = false;
		if (EquippedWeapon)
		{
			CrosshairShootingFactor = 0.75f;
			switch (EquippedWeapon->FireType)
			{
			case EFireType::EFT_HitScan:
				break;
			case EFireType::EFT_Projectile:
				FireProjectileWeapon();
				break;
			case EFireType::EFT_Shotgun:
				FireShotgun();
				break;
			case EFireType::EFT_Melee:
				FireMeleeWeapon();
				break;
			default:
				break;
			}
		}
		StartFireTimer();
	}
}

void UCombatComponent::FireProjectileWeapon()
{
	if (EquippedWeapon && Character)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		if (!Character->HasAuthority()) LocalFire(HitTarget);
		ServerFire(HitTarget);
	}
}

void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;
	if (Character)
	{
		Character->PlayFireMontage();
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::FireShotgun()
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if (Shotgun && Character)
	{
		TArray<FVector_NetQuantize> HitTargets;
		Shotgun->ShotgunTraceEndWithScatter(HitTarget, HitTargets);
		if (!Character->HasAuthority()) ShotgunLocalFire(HitTargets);
		ServerShotgunFire(HitTargets, EquippedWeapon->FireDelay);
	}
}

void UCombatComponent::ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if (Shotgun == nullptr || Character == nullptr) return;

	if (!Character->HasAuthority()) Character->PlayFireMontage();
	Shotgun->FireShotgun(TraceHitTargets);
}

void UCombatComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay)
{
	MulticastShotgunFire(TraceHitTargets);
}

bool UCombatComponent::ServerShotgunFire_Validate(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay)
{
	if (EquippedWeapon)
	{
		bool bNearlyEqual = FMath::IsNearlyEqual(EquippedWeapon->FireDelay, FireDelay, 0.001f);
		return bNearlyEqual;
	}
	return true;
}

void UCombatComponent::MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	ShotgunLocalFire(TraceHitTargets);
}

void UCombatComponent::FireMeleeWeapon()
{
	if (EquippedWeapon && Character)
	{
		if (!Character->HasAuthority()) MeleeLocalFire();
		ServerMeleeFire(EquippedWeapon->FireDelay);
	}
}

void UCombatComponent::MeleeLocalFire()
{
	AMeleeWeapon* MeleeWeapon = Cast<AMeleeWeapon>(EquippedWeapon);
	if (Character && MeleeWeapon)
	{
		Character->PlayFireMontage();
		MeleeWeapon->FireMeleeWeapon();
	}
}

void UCombatComponent::ServerMeleeFire_Implementation(float FireDelay)
{
	MulticastMeleeFire();
}

bool UCombatComponent::ServerMeleeFire_Validate(float FireDelay)
{
	if (EquippedWeapon)
	{
		bool bNearlyEqual = FMath::IsNearlyEqual(EquippedWeapon->FireDelay, FireDelay, 0.001f);
		return bNearlyEqual;
	}
	return true;
}

void UCombatComponent::MulticastMeleeFire_Implementation()
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	MeleeLocalFire();
}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;
	
	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireDelay
	);
}

void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr) return;
	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bAutomatic)
	{
		Fire();
	}
	if (EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

void UCombatComponent::SwapWeapons()
{
	ServerSwapWeapons();
}

void UCombatComponent::ServerSwapWeapons_Implementation()
{
	if (Character == nullptr) return;

	if (EquippedWeapon)
	{
		EquippedWeapon->GetWeaponMesh()->SetVisibility(false);
		SecondaryWeapon->GetWeaponMesh()->SetVisibility(true);
		AWeapon* Weapon = EquippedWeapon;
		EquippedWeapon = SecondaryWeapon;
		SecondaryWeapon = Weapon;
	}
}

void UCombatComponent::Reload()
{
	ServerReload();
}

void UCombatComponent::ServerReload_Implementation()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	
	HandleReload();
}

void UCombatComponent::HandleReload()
{
	EquippedWeapon->Reload();
	//Character->PlayReloadMontage();
}

void UCombatComponent::EquipWeapon(AWeapon* PrimaryGun, AWeapon* SecondaryGun)
{
	if (Character == nullptr) return;

	const USkeletalMeshSocket* HandSocket = Character->GetArm()->GetSocketByName(FName("RightHandSocket"));
	if (PrimaryGun)
	{
		HandSocket->AttachActor(PrimaryGun, Character->GetArm());
		PrimaryGun->SetOwner(Character);
		EquippedWeapon = PrimaryGun;
	}
	if (SecondaryGun)
	{
		HandSocket->AttachActor(SecondaryGun, Character->GetArm());
		SecondaryGun->SetOwner(Character);
		SecondaryGun->GetWeaponMesh()->SetVisibility(false);
		SecondaryWeapon = SecondaryGun;
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	if (Character && Character->GetCamera())
	{
		FVector Direction = Character->GetBaseAimRotation().Vector();
		FVector Start = Character->GetCamera()->GetComponentLocation();
		FVector End = Start + Direction * Character->GetTraceLength();
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Character);

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECC_Visibility,
			QueryParams
		);
		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
		}
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr || Character->GetCharacterMovement() == nullptr) return;

	Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : Controller;
	if (Controller == nullptr) return;
	
	HUD = HUD == nullptr ? Cast<AShooterHUD>(Controller->GetHUD()) : HUD;
	if (HUD)
	{
		if (EquippedWeapon)
		{
			HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
			HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
			HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
			HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
			HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
		}
		else
		{
			HUDPackage.CrosshairsCenter = nullptr;
			HUDPackage.CrosshairsLeft = nullptr;
			HUDPackage.CrosshairsRight = nullptr;
			HUDPackage.CrosshairsBottom = nullptr;
			HUDPackage.CrosshairsTop = nullptr;
		}
		
		FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
		FVector2D VelocityMultiplierRange(0.f, 1.f);
		FVector Velocity = Character->GetVelocity();
		Velocity.Z = 0.f;

		CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

		if (Character->GetCharacterMovement()->IsFalling())
		{
			CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
		}
		else
		{
			CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
		}
			
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);

		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);

		HUDPackage.CrosshairSpread =
			0.5f +
			CrosshairVelocityFactor +
			CrosshairInAirFactor -
			CrosshairAimFactor +
			CrosshairShootingFactor;

		HUDPackage.CrosshairsColor = FLinearColor::White;

		HUD->SetHUDPackage(HUDPackage);
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (Character)
	{
		if (Character->IsLocallyControlled())
		{
			Character->UpdateHUDAmmo();
			AttachToArm(EquippedWeapon);
		}
		else
		{
			AttachToMesh(EquippedWeapon);
		}
	}
}

void UCombatComponent::OnRep_SecondaryWeapon()
{
	if (Character)
	{
		if (Character->IsLocallyControlled())
		{
			AttachToArm(SecondaryWeapon);
		}
		else
		{
			AttachToMesh(SecondaryWeapon);
		}
	}
}

void UCombatComponent::AttachToArm(AWeapon* WeaponToAttach)
{
	const USkeletalMeshSocket* HandSocket = Character->GetArm()->GetSocketByName(FName("RightHandSocket"));
	HandSocket->AttachActor(WeaponToAttach, Character->GetArm());
}

void UCombatComponent::AttachToMesh(AWeapon* WeaponToAttach)
{
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	HandSocket->AttachActor(WeaponToAttach, Character->GetMesh());
}

bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr) return false;
	return !EquippedWeapon->IsEmpty() && bCanFire;
}

