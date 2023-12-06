// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ShooterAnimInstanceBase.h"
#include "Character/ShooterCharacterBase.h"
#include "Weapon/Weapon.h"

void UShooterAnimInstanceBase::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	ShooterCharacter = Cast<AShooterCharacterBase>(TryGetPawnOwner());
}

void UShooterAnimInstanceBase::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (ShooterCharacter == nullptr)
	{
		ShooterCharacter = Cast<AShooterCharacterBase>(TryGetPawnOwner());
	}
	if (ShooterCharacter == nullptr) return;

	FVector Velocity = ShooterCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	if (ShooterCharacter->GetEquippedWeapon())
	{
		WeaponType = ShooterCharacter->GetEquippedWeapon()->GetWeaponType();
	}
}