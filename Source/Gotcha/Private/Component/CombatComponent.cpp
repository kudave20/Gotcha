// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/CombatComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/ShooterCharacterBase.h"
#include "Components/BoxComponent.h"
#include "Player/ShooterPlayerController.h"
#include "Weapon/Weapon.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Gotcha/Gotcha.h"
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
			switch (EquippedWeapon->GetWeaponType())
			{
			case EWeaponType::EWT_Shotgun:
				FireShotgun();
				break;
			case EWeaponType::EWT_Katana:
				FireMeleeWeapon();
				break;
			default:
				break;
			}
		}
		StartFireTimer();
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
	AWeapon* PreviousSecondaryWeapon = SecondaryWeapon;
	if (EquippedWeapon)
	{
		AttachWeaponToBackpack(EquippedWeapon);
	}
	if (PreviousSecondaryWeapon)
	{
		AttachWeaponToRightHand(PreviousSecondaryWeapon);
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

void UCombatComponent::ParryButtonPressed()
{
	if (bIsParrying) return;
	ServerParry();
}

void UCombatComponent::ServerParry_Implementation()
{
	if (Character == nullptr) return;
	
	bIsParrying = true;
	AWeapon* PreviousSecondaryWeapon = SecondaryWeapon;
	AttachWeaponToBackpack(EquippedWeapon);
	AttachWeaponToRightHand(PreviousSecondaryWeapon);
	
	Character->GetWorldTimerManager().SetTimer(
		ParryTimer,
		this,
		&UCombatComponent::ParryTimerFinished,
		Character->GetParryTime()
	);
}


void UCombatComponent::ParryTimerFinished()
{
	bIsParrying = false;
	AWeapon* PreviousSecondaryWeapon = SecondaryWeapon;
	AttachWeaponToBackpack(EquippedWeapon);
	AttachWeaponToRightHand(PreviousSecondaryWeapon);
}

bool UCombatComponent::Parry(AWeapon* DamageCauser)
{
	if (!bIsParrying) return false;
	
	if (DamageCauser == nullptr || DamageCauser->GetWeaponType() != EWeaponType::EWT_Shotgun) return false;
	
	UWorld* World = GetWorld();
	if (World == nullptr) return false;

	if (Character && Character->GetCamera())
	{
		FVector Direction = Character->GetCamera()->GetForwardVector();
		FVector Start = Character->GetCamera()->GetComponentLocation();
		FVector End = Start + Direction * TRACE_LENGTH;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Character);
		
		FHitResult ParryResult;
		World->LineTraceSingleByChannel(
			ParryResult,
			Start,
			End,
			ECC_Assist,
			QueryParams
		);
		AShooterCharacterBase* ShooterCharacter = Cast<AShooterCharacterBase>(ParryResult.GetActor());
		if (ShooterCharacter)
		{
			UGameplayStatics::ApplyDamage(
				ShooterCharacter,
				ShooterCharacter->GetMaxHealth(),
				Controller,
				EquippedWeapon,
				UDamageType::StaticClass()
			);

			return true;
		}
	}
	
	return false;
}

void UCombatComponent::EquipWeapons(AWeapon* PrimaryGun, AWeapon* SecondaryGun)
{
	if (PrimaryGun && SecondaryGun)
	{
		AttachWeaponToRightHand(PrimaryGun);
		AttachWeaponToBackpack(SecondaryGun);
	}
}

void UCombatComponent::AttachWeaponToRightHand(AWeapon* WeaponToAttach)
{
	if (Character == nullptr) return;
	
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket && WeaponToAttach)
	{
		HandSocket->AttachActor(WeaponToAttach, Character->GetMesh());
		WeaponToAttach->SetOwner(Character);
		EquippedWeapon = WeaponToAttach;
	}
}

void UCombatComponent::AttachWeaponToBackpack(AWeapon* WeaponToAttach)
{
	if (Character == nullptr) return;
	
	const USkeletalMeshSocket* BackpackSocket = Character->GetMesh()->GetSocketByName(FName("BackpackSocket"));
	if (BackpackSocket && WeaponToAttach)
	{
		BackpackSocket->AttachActor(WeaponToAttach, Character->GetMesh());
		WeaponToAttach->SetOwner(Character);
		SecondaryWeapon = WeaponToAttach;
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	if (Character && Character->GetCamera())
	{
		FVector Direction = Character->GetCamera()->GetForwardVector();
		FVector Start = Character->GetCamera()->GetComponentLocation();
		FVector End = Start + Direction * TRACE_LENGTH;
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

void UCombatComponent::SlashStarted()
{
	if (Character == nullptr || !Character->HasAuthority()) return;
	
	AMeleeWeapon* MeleeWeapon = Cast<AMeleeWeapon>(EquippedWeapon);
	if (MeleeWeapon && MeleeWeapon->GetMeleeBox())
	{
		MeleeWeapon->GetMeleeBox()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	}
}

void UCombatComponent::SlashFinished()
{
	if (Character == nullptr || !Character->HasAuthority()) return;
	
	AMeleeWeapon* MeleeWeapon = Cast<AMeleeWeapon>(EquippedWeapon);
	if (MeleeWeapon && MeleeWeapon->GetMeleeBox())
	{
		MeleeWeapon->GetMeleeBox()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
	}
}

bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr) return false;
	return !EquippedWeapon->IsEmpty() && bCanFire;
}

