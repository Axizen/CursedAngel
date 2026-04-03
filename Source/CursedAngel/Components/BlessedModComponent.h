// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatTypes.h"
#include "BlessedModComponent.generated.h"

// Forward declarations
class UBlessedModDataAsset;
class UCurseWeaponComponent;
class UCurrencyComponent;

/**
 * Delegate for mod unlock events
 * Broadcasts when a mod is successfully unlocked
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnModUnlocked, UBlessedModDataAsset*, UnlockedMod);

/**
 * Delegate for mod equip events
 * Broadcasts when a mod is successfully equipped to a weapon
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnModEquipped, UBlessedModDataAsset*, EquippedMod, ECurseWeaponType, WeaponType);

/**
 * Delegate for mod unequip events
 * Broadcasts when a mod is unequipped from a weapon
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnModUnequipped, UBlessedModDataAsset*, UnequippedMod, ECurseWeaponType, WeaponType);

/**
 * Component that manages blessed mod inventory and equipping system
 * Deadlocked-style flat mod system: no tiers, currency-based unlocks
 * Integrates with CurseWeaponComponent to apply stat modifiers
 * Each weapon has 1-3 mod slots (configurable via weapon data asset)
 */
UCLASS(ClassGroup=(BlessedMod), meta=(BlueprintSpawnableComponent))
class CURSEDANGEL_API UBlessedModComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBlessedModComponent();

protected:
	virtual void BeginPlay() override;

public:	
	// ========== Properties ==========

	/** Array of mods that have been unlocked by the player */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blessed Mods")
	TArray<UBlessedModDataAsset*> UnlockedMods;

private:
	/** Map of equipped mods per weapon type (not exposed to Blueprint due to TMap<TArray> limitation) */
	TMap<ECurseWeaponType, TArray<UBlessedModDataAsset*>> EquippedMods;

	// ========== Blueprint Events ==========

	/** Called when a mod is successfully unlocked */
	UPROPERTY(BlueprintAssignable, Category = "Blessed Mods|Events")
	FOnModUnlocked OnModUnlocked;

	/** Called when a mod is equipped to a weapon */
	UPROPERTY(BlueprintAssignable, Category = "Blessed Mods|Events")
	FOnModEquipped OnModEquipped;

	/** Called when a mod is unequipped from a weapon */
	UPROPERTY(BlueprintAssignable, Category = "Blessed Mods|Events")
	FOnModUnequipped OnModUnequipped;

	// ========== Functions ==========

	/**
	 * Unlock a mod by spending currency
	 * @param Mod The mod to unlock
	 * @param CurrencyComp The currency component to spend from
	 * @return True if unlocked successfully, false if already unlocked or insufficient currency
	 */
	UFUNCTION(BlueprintCallable, Category = "Blessed Mods")
	bool UnlockMod(UBlessedModDataAsset* Mod, UCurrencyComponent* CurrencyComp);

	/**
	 * Equip a mod to a specific weapon
	 * @param Mod The mod to equip
	 * @param WeaponType The weapon to equip the mod to
	 * @return True if equipped successfully, false if not unlocked, incompatible, or no available slots
	 */
	UFUNCTION(BlueprintCallable, Category = "Blessed Mods")
	bool EquipMod(UBlessedModDataAsset* Mod, ECurseWeaponType WeaponType);

	/**
	 * Unequip a mod from a specific weapon
	 * @param Mod The mod to unequip
	 * @param WeaponType The weapon to unequip the mod from
	 * @return True if unequipped successfully, false if not equipped
	 */
	UFUNCTION(BlueprintCallable, Category = "Blessed Mods")
	bool UnequipMod(UBlessedModDataAsset* Mod, ECurseWeaponType WeaponType);

	/**
	 * Get all equipped mods for a specific weapon
	 * @param WeaponType The weapon to query
	 * @return Array of equipped mods for the weapon
	 */
	UFUNCTION(BlueprintCallable, Category = "Blessed Mods")
	TArray<UBlessedModDataAsset*> GetEquippedMods(ECurseWeaponType WeaponType) const;

	/**
	 * Check if a mod is unlocked
	 * @param Mod The mod to check
	 * @return True if the mod is unlocked
	 */
	UFUNCTION(BlueprintCallable, Category = "Blessed Mods")
	bool IsModUnlocked(UBlessedModDataAsset* Mod) const;

	/**
	 * Check if a mod is equipped to a specific weapon
	 * @param Mod The mod to check
	 * @param WeaponType The weapon to check
	 * @return True if the mod is equipped to the weapon
	 */
	UFUNCTION(BlueprintCallable, Category = "Blessed Mods")
	bool IsModEquipped(UBlessedModDataAsset* Mod, ECurseWeaponType WeaponType) const;

	/**
	 * Apply all equipped mods to a weapon
	 * Calculates cumulative stat modifiers and applies them to the weapon
	 * @param WeaponType The weapon to apply mods to
	 */
	UFUNCTION(BlueprintCallable, Category = "Blessed Mods")
	void ApplyModsToWeapon(ECurseWeaponType WeaponType);

	/**
	 * Remove all mod effects from a weapon
	 * Resets weapon stats to base values
	 * @param WeaponType The weapon to remove mods from
	 */
	UFUNCTION(BlueprintCallable, Category = "Blessed Mods")
	void RemoveModsFromWeapon(ECurseWeaponType WeaponType);

	/**
	 * Get available mod slots for a weapon
	 * @param WeaponType The weapon to query
	 * @return Number of available mod slots (max slots - equipped mods)
	 */
	UFUNCTION(BlueprintCallable, Category = "Blessed Mods")
	int32 GetAvailableModSlots(ECurseWeaponType WeaponType) const;

private:
	/** Cached reference to the owner's curse weapon component */
	UPROPERTY()
	UCurseWeaponComponent* WeaponComponent;
};
