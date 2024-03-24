#pragma once

UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan UMETA(DisplayName = "Hit Scan Weapon"),
	EFT_Projectile UMETA(DisplayName = "Projectile Weapon"),
	EFT_Shotgun UMETA(DisplayName = "Shotgun Weapon"),
	EFT_Melee UMETA(DisplayName = "Melee Weapon"),

	EFT_MAX UMETA(DisplayName = "DefaultMAX")
};
