#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_Shotgun UMETA(DisplayName = "Shotgun"),
	EWT_Katana UMETA(DisplayName = "Katana"),
	EWT_Flag UMETA(DisplayName = "Flag"),

	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};
