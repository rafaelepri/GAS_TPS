#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8 {
	EWT_Melee UMETA(DisplayName = "Melee"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	EWT_Rifle UMETA(DisplayName = "Rifle"),
	EWT_SMG  UMETA(DisplayName = "SMG"),
	
	EWT_MAX UMETA(DisplayName = "Max")
};

UENUM(BlueprintType)
enum class EWeaponState : uint8 {
	EWS_Initial UMETA(DisplayName = "InitialState"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "Max")
};