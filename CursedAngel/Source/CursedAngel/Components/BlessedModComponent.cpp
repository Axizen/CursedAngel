// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/BlessedModComponent.h"
#include "BlessedModDataAsset.h"
#include "Components/CurrencyComponent.h"
#include "Components/CurseWeaponComponent.h"
#include "CombatTypes.h"
#include "GameFramework/Actor.h"

UBlessedModComponent::UBlessedModComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);
	
	// Initialize empty collections
	UnlockedMods.Empty();
	EquippedMods.Empty();
	WeaponComponent = nullptr;
}

void UBlessedModComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Cache reference to owner's CurseWeaponComponent
	AActor* Owner = GetOwner();
	if (Owner)
	{
		WeaponComponent = Owner->FindComponentByClass<UCurseWeaponComponent>();
		if (!WeaponComponent)
		{
			UE_LOG(LogTemp, Warning, TEXT("BlessedModComponent: Owner %s does not have a CurseWeaponComponent!"), *Owner->GetName());
		}
	}
}

bool UBlessedModComponent::UnlockMod(UBlessedModDataAsset* Mod, UCurrencyComponent* CurrencyComp)
{
	// Validate inputs
	if (!Mod)
	{
		UE_LOG(LogTemp, Warning, TEXT("BlessedModComponent: UnlockMod called with null Mod!"));
		return false;
	}
	
	if (!CurrencyComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("BlessedModComponent: UnlockMod called with null CurrencyComponent!"));
		return false;
	}
	
	// Check if already unlocked
	if (IsModUnlocked(Mod))
	{
		UE_LOG(LogTemp, Warning, TEXT("BlessedModComponent: Mod %s is already unlocked!"), *Mod->ModName.ToString());
		return false;
	}
	
	// Check currency requirements
	const bool bHasDataFragments = CurrencyComp->GetDataFragments() >= Mod->UnlockCostDataFragments;
	const bool bHasCurseEssence = CurrencyComp->GetCurseEssence() >= Mod->UnlockCostCurseEssence;
	const bool bHasCodeKeys = CurrencyComp->GetCodeKeys() >= Mod->UnlockCostCodeKeys;
	
	if (!bHasDataFragments || !bHasCurseEssence || !bHasCodeKeys)
	{
		UE_LOG(LogTemp, Warning, TEXT("BlessedModComponent: Insufficient currency to unlock %s. Need: %d DF, %d CE, %d CK. Have: %d DF, %d CE, %d CK"),
			*Mod->ModName.ToString(),
			Mod->UnlockCostDataFragments, Mod->UnlockCostCurseEssence, Mod->UnlockCostCodeKeys,
			CurrencyComp->GetDataFragments(), CurrencyComp->GetCurseEssence(), CurrencyComp->GetCodeKeys());
		return false;
	}
	
	// Spend currency
	bool bSpendSuccess = true;
	if (Mod->UnlockCostDataFragments > 0)
	{
		bSpendSuccess &= CurrencyComp->SpendDataFragments(Mod->UnlockCostDataFragments);
	}
	if (Mod->UnlockCostCurseEssence > 0)
	{
		bSpendSuccess &= CurrencyComp->SpendCurseEssence(Mod->UnlockCostCurseEssence);
	}
	if (Mod->UnlockCostCodeKeys > 0)
	{
		bSpendSuccess &= CurrencyComp->SpendCodeKeys(Mod->UnlockCostCodeKeys);
	}
	
	if (!bSpendSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("BlessedModComponent: Failed to spend currency for mod %s!"), *Mod->ModName.ToString());
		return false;
	}
	
	// Add to unlocked mods
	UnlockedMods.Add(Mod);
	UE_LOG(LogTemp, Log, TEXT("BlessedModComponent: Successfully unlocked mod %s"), *Mod->ModName.ToString());
	
	// Broadcast unlock event
	OnModUnlocked.Broadcast(Mod);
	
	return true;
}

bool UBlessedModComponent::EquipMod(UBlessedModDataAsset* Mod, ECurseWeaponType WeaponType)
{
	// Validate inputs
	if (!Mod)
	{
		UE_LOG(LogTemp, Warning, TEXT("BlessedModComponent: EquipMod called with null Mod!"));
		return false;
	}
	
	// Check if mod is unlocked
	if (!IsModUnlocked(Mod))
	{
		UE_LOG(LogTemp, Warning, TEXT("BlessedModComponent: Cannot equip locked mod %s!"), *Mod->ModName.ToString());
		return false;
	}
	
	// Check if already equipped to this weapon
	if (IsModEquipped(Mod, WeaponType))
	{
		UE_LOG(LogTemp, Warning, TEXT("BlessedModComponent: Mod %s is already equipped to weapon!"), *Mod->ModName.ToString());
		return false;
	}
	
	// Check compatibility
	if (!Mod->IsCompatibleWithWeapon(WeaponType))
	{
		UE_LOG(LogTemp, Warning, TEXT("BlessedModComponent: Mod %s is not compatible with weapon type %d!"), 
			*Mod->ModName.ToString(), static_cast<int32>(WeaponType));
		return false;
	}
	
	// Check available mod slots
	const int32 AvailableSlots = GetAvailableModSlots(WeaponType);
	if (AvailableSlots <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("BlessedModComponent: No available mod slots for weapon type %d!"), static_cast<int32>(WeaponType));
		return false;
	}
	
	// Add to equipped mods for this weapon
	TArray<UBlessedModDataAsset*>& WeaponMods = EquippedMods.FindOrAdd(WeaponType);
	WeaponMods.Add(Mod);
	
	// Apply all mods to weapon (recalculate cumulative stats)
	ApplyModsToWeapon(WeaponType);
	
	UE_LOG(LogTemp, Log, TEXT("BlessedModComponent: Successfully equipped mod %s to weapon type %d"), 
		*Mod->ModName.ToString(), static_cast<int32>(WeaponType));
	
	// Broadcast equip event
	OnModEquipped.Broadcast(Mod, WeaponType);
	
	return true;
}

bool UBlessedModComponent::UnequipMod(UBlessedModDataAsset* Mod, ECurseWeaponType WeaponType)
{
	// Validate inputs
	if (!Mod)
	{
		UE_LOG(LogTemp, Warning, TEXT("BlessedModComponent: UnequipMod called with null Mod!"));
		return false;
	}
	
	// Check if equipped
	if (!IsModEquipped(Mod, WeaponType))
	{
		UE_LOG(LogTemp, Warning, TEXT("BlessedModComponent: Mod %s is not equipped to weapon type %d!"), 
			*Mod->ModName.ToString(), static_cast<int32>(WeaponType));
		return false;
	}
	
	// Remove from equipped mods
	TArray<UBlessedModDataAsset*>* WeaponMods = EquippedMods.Find(WeaponType);
	if (WeaponMods)
	{
		WeaponMods->Remove(Mod);
	}
	
	// Recalculate mod effects
	ApplyModsToWeapon(WeaponType);
	
	UE_LOG(LogTemp, Log, TEXT("BlessedModComponent: Successfully unequipped mod %s from weapon type %d"), 
		*Mod->ModName.ToString(), static_cast<int32>(WeaponType));
	
	// Broadcast unequip event
	OnModUnequipped.Broadcast(Mod, WeaponType);
	
	return true;
}

TArray<UBlessedModDataAsset*> UBlessedModComponent::GetEquippedMods(ECurseWeaponType WeaponType) const
{
	const TArray<UBlessedModDataAsset*>* WeaponMods = EquippedMods.Find(WeaponType);
	if (WeaponMods)
	{
		return *WeaponMods;
	}
	return TArray<UBlessedModDataAsset*>();
}

bool UBlessedModComponent::IsModUnlocked(UBlessedModDataAsset* Mod) const
{
	return UnlockedMods.Contains(Mod);
}

bool UBlessedModComponent::IsModEquipped(UBlessedModDataAsset* Mod, ECurseWeaponType WeaponType) const
{
	const TArray<UBlessedModDataAsset*>* WeaponMods = EquippedMods.Find(WeaponType);
	if (WeaponMods)
	{
		return WeaponMods->Contains(Mod);
	}
	return false;
}

void UBlessedModComponent::ApplyModsToWeapon(ECurseWeaponType WeaponType)
{
	if (!WeaponComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("BlessedModComponent: Cannot apply mods, WeaponComponent is null!"));
		return;
	}
	
	// Get base weapon stats from weapon component
	FCurseWeaponStats BaseStats = WeaponComponent->GetWeaponStats(WeaponType);
	
	// Get equipped mods for this weapon
	const TArray<UBlessedModDataAsset*> WeaponMods = GetEquippedMods(WeaponType);
	
	if (WeaponMods.Num() == 0)
	{
		// No mods equipped, ensure base stats are used
		WeaponComponent->WeaponStatsMap.Add(WeaponType, BaseStats);
		return;
	}
	
	// Calculate cumulative modifiers
	float TotalDamageMultiplier = 1.0f;
	float TotalFireRateMultiplier = 1.0f;
	float TotalProjectileSpeedMultiplier = 1.0f;
	int32 TotalPenetrationBonus = 0;
	float TotalCooldownMultiplier = 1.0f;
	
	for (const UBlessedModDataAsset* Mod : WeaponMods)
	{
		if (Mod)
		{
			const FModStatModifiers& Modifiers = Mod->StatModifiers;
			
			// Multiply multipliers (cumulative)
			TotalDamageMultiplier *= Modifiers.DamageMultiplier;
			TotalFireRateMultiplier *= Modifiers.FireRateMultiplier;
			TotalProjectileSpeedMultiplier *= Modifiers.ProjectileSpeedMultiplier;
			TotalCooldownMultiplier *= Modifiers.CooldownMultiplier;
			
			// Add bonuses (additive)
			TotalPenetrationBonus += Modifiers.PenetrationBonus;
			
			UE_LOG(LogTemp, Verbose, TEXT("BlessedModComponent: Applying mod %s - Damage: %.2fx, FireRate: %.2fx, Speed: %.2fx, Penetration: +%d, Cooldown: %.2fx"),
				*Mod->ModName.ToString(),
				Modifiers.DamageMultiplier,
				Modifiers.FireRateMultiplier,
				Modifiers.ProjectileSpeedMultiplier,
				Modifiers.PenetrationBonus,
				Modifiers.CooldownMultiplier);
		}
	}
	
	// Apply cumulative modifiers to base stats
	FCurseWeaponStats ModifiedStats = BaseStats;
	ModifiedStats.Damage *= TotalDamageMultiplier;
	ModifiedStats.FireRate *= TotalFireRateMultiplier;
	ModifiedStats.ProjectileSpeed *= TotalProjectileSpeedMultiplier;
	ModifiedStats.PenetrationCount += TotalPenetrationBonus;
	ModifiedStats.CooldownTime *= TotalCooldownMultiplier;
	
	// Clamp values to reasonable ranges
	ModifiedStats.Damage = FMath::Max(ModifiedStats.Damage, 1.0f);
	ModifiedStats.FireRate = FMath::Max(ModifiedStats.FireRate, 0.1f);
	ModifiedStats.ProjectileSpeed = FMath::Max(ModifiedStats.ProjectileSpeed, 100.0f);
	ModifiedStats.PenetrationCount = FMath::Max(ModifiedStats.PenetrationCount, 0);
	ModifiedStats.CooldownTime = FMath::Max(ModifiedStats.CooldownTime, 0.1f);
	
	// Update weapon stats in weapon component
	WeaponComponent->WeaponStatsMap.Add(WeaponType, ModifiedStats);
	
	UE_LOG(LogTemp, Log, TEXT("BlessedModComponent: Applied %d mods to weapon type %d. Final stats - Damage: %.2f, FireRate: %.2f, Speed: %.2f, Penetration: %d, Cooldown: %.2f"),
		WeaponMods.Num(),
		static_cast<int32>(WeaponType),
		ModifiedStats.Damage,
		ModifiedStats.FireRate,
		ModifiedStats.ProjectileSpeed,
		ModifiedStats.PenetrationCount,
		ModifiedStats.CooldownTime);
}

void UBlessedModComponent::RemoveModsFromWeapon(ECurseWeaponType WeaponType)
{
	if (!WeaponComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("BlessedModComponent: Cannot remove mods, WeaponComponent is null!"));
		return;
	}
	
	// Get base weapon stats and reset to them
	FCurseWeaponStats BaseStats = WeaponComponent->GetWeaponStats(WeaponType);
	WeaponComponent->WeaponStatsMap.Add(WeaponType, BaseStats);
	
	UE_LOG(LogTemp, Log, TEXT("BlessedModComponent: Reset weapon type %d to base stats"), static_cast<int32>(WeaponType));
}

int32 UBlessedModComponent::GetAvailableModSlots(ECurseWeaponType WeaponType) const
{
	// Default max slots per weapon (this should ideally come from CurseWeaponDataAsset)
	// For now, use a default of 3 max slots
	const int32 MaxSlots = 3;
	
	// Get currently equipped mods
	const TArray<UBlessedModDataAsset*> WeaponMods = GetEquippedMods(WeaponType);
	const int32 EquippedCount = WeaponMods.Num();
	
	// Calculate available slots
	const int32 AvailableSlots = MaxSlots - EquippedCount;
	
	return FMath::Max(AvailableSlots, 0);
}
