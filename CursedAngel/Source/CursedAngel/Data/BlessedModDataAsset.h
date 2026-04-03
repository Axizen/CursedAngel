// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CombatTypes.h"
#include "BlessedModDataAsset.generated.h"

/**
 * Blessed Mod type classification
 * Inspired by Ratchet Deadlocked Omega Mods - flat system with no tiers
 */
UENUM(BlueprintType)
enum class EBlessedModType : uint8
{
	Damage UMETA(DisplayName = "Damage"),
	Utility UMETA(DisplayName = "Utility"),
	Movement UMETA(DisplayName = "Movement"),
	Special UMETA(DisplayName = "Special")
};

/**
 * Stat modifier configuration for blessed mods
 * Flat modifiers that stack multiplicatively (multipliers) or additively (bonuses)
 */
USTRUCT(BlueprintType)
struct FModStatModifiers
{
	GENERATED_BODY()

	/** Damage multiplier (1.0 = no change, 1.25 = +25% damage) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Modifiers")
	float DamageMultiplier = 1.0f;

	/** Fire rate multiplier (1.0 = no change, 1.15 = +15% fire rate) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Modifiers")
	float FireRateMultiplier = 1.0f;

	/** Projectile speed multiplier (1.0 = no change, 1.5 = +50% speed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Modifiers")
	float ProjectileSpeedMultiplier = 1.0f;

	/** Penetration bonus (additive, 0 = no change, 2 = +2 penetration) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Modifiers")
	int32 PenetrationBonus = 0;

	/** Cooldown multiplier (1.0 = no change, 0.8 = -20% cooldown) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Modifiers")
	float CooldownMultiplier = 1.0f;
};

/**
 * Data Asset for blessed mod configuration
 * Deadlocked-style Omega Mod system: flat progression, no tiers, currency-based unlocks
 * All mods are equal in power, differentiated by cost, effects, and synergies
 */
UCLASS(BlueprintType)
class CURSEDANGEL_API UBlessedModDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Display name of the mod */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mod Info")
	FText ModName;

	/** Description of what the mod does */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mod Info")
	FText ModDescription;

	/** Type classification of the mod */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mod Info")
	EBlessedModType ModType;

	/** 
	 * Compatible weapon types for this mod
	 * Empty array = universal mod (works on all weapons)
	 * Populated array = weapon-specific mod
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compatibility")
	TArray<ECurseWeaponType> CompatibleWeapons;

	/** Stat modifiers applied by this mod */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	FModStatModifiers StatModifiers;

	/** 
	 * Special effect identifiers for unique mod behaviors
	 * Examples: "ExplosiveRounds", "ChainLightning", "SlowTime"
	 * Handled by gameplay code based on effect name
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	TArray<FName> SpecialEffects;

	/** Data Fragments cost to unlock this mod */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlock Requirements")
	int32 UnlockCostDataFragments = 0;

	/** Curse Essence cost to unlock this mod */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlock Requirements")
	int32 UnlockCostCurseEssence = 0;

	/** Code Keys cost to unlock this mod (rare collectibles) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlock Requirements")
	int32 UnlockCostCodeKeys = 0;

	/** Whether this mod requires story progression to unlock */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlock Requirements")
	bool bRequiresStoryProgress = false;

	/** Icon for UI display */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	UTexture2D* ModIcon;

	/** 
	 * Synergy tags for combo effects
	 * Mods with matching tags provide bonus effects when equipped together
	 * Examples: "Explosive", "Electric", "Freeze"
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Synergy")
	TArray<FName> SynergyTags;

	/**
	 * Check if this mod is compatible with a specific weapon type
	 * @param WeaponType The weapon type to check
	 * @return True if compatible (empty array = universal, or weapon in array)
	 */
	UFUNCTION(BlueprintCallable, Category = "Mod Info")
	bool IsCompatibleWithWeapon(ECurseWeaponType WeaponType) const;

	/** Override to provide primary asset ID for asset manager integration */
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
