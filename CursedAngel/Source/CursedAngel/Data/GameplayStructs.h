// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "CombatTypes.h"
#include "GameplayStructs.generated.h"

/**
 * Curse data structure
 * Defines properties for all curses in the progression system
 * Curses are collected and refined using data currency
 */
USTRUCT(BlueprintType)
struct FCurseData
{
	GENERATED_BODY()

	/** Unique identifier for this curse */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curse")
	FName CurseID;

	/** Display name of the curse */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curse")
	FText CurseName;

	/** Description of curse effects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curse")
	FText Description;

	/** Data currency cost to refine this curse */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curse")
	int32 RefinementCost = 0;

	/** Type of curse (Skill, Weapon, BlessedMod, Passive) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curse")
	ECurseType Type = ECurseType::Skill;

	/** Icon for UI display */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curse")
	TObjectPtr<UTexture2D> Icon;

	FCurseData()
		: CurseID(NAME_None)
		, CurseName(FText::GetEmpty())
		, Description(FText::GetEmpty())
		, RefinementCost(0)
		, Type(ECurseType::Skill)
		, Icon(nullptr)
	{
	}
};

/**
 * Weapon data structure
 * Contains all weapon stats including ammo, cooldown, and mod compatibility
 * Hybrid ammo + cooldown system inspired by DMC and Ratchet & Clank
 */
USTRUCT(BlueprintType)
struct FWeaponData
{
	GENERATED_BODY()

	/** Unique identifier for this weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FName WeaponID;

	/** Display name of the weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FText WeaponName;

	/** Maximum ammo capacity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo")
	int32 MaxAmmo = 100;

	/** Current ammo count */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo")
	int32 CurrentAmmo = 100;

	/** Base cooldown between shots in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Cooldown")
	float BaseCooldown = 0.5f;

	/** Current cooldown remaining in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Cooldown")
	float CurrentCooldown = 0.0f;

	/** List of compatible Blessed Mod IDs for this weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Mods")
	TArray<FName> CompatibleModIDs;

	/** Currently equipped Blessed Mod ID (NAME_None if none) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Mods")
	FName EquippedModID;

	/** Actor class to spawn when weapon is equipped */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<AActor> WeaponActorClass;

	FWeaponData()
		: WeaponID(NAME_None)
		, WeaponName(FText::GetEmpty())
		, MaxAmmo(100)
		, CurrentAmmo(100)
		, BaseCooldown(0.5f)
		, CurrentCooldown(0.0f)
		, EquippedModID(NAME_None)
		, WeaponActorClass(nullptr)
	{
	}
};

/**
 * Blessed Mod data structure
 * Defines properties for weapon modification system
 * Inspired by Ratchet Deadlocked's Omega Mod system (non-stackable)
 */
USTRUCT(BlueprintType)
struct FBlessedModData
{
	GENERATED_BODY()

	/** Unique identifier for this mod */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blessed Mod")
	FName ModID;

	/** Display name of the mod */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blessed Mod")
	FText ModName;

	/** Description of mod effects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blessed Mod")
	FText Description;

	/** List of compatible weapon IDs for this mod */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blessed Mod")
	TArray<FName> CompatibleWeaponIDs;

	/** Stat modifiers applied by this mod (e.g., "Damage": 1.5, "FireRate": 0.75) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blessed Mod")
	TMap<FName, float> StatModifiers;

	/** Icon for UI display */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blessed Mod")
	TObjectPtr<UTexture2D> Icon;

	/** Whether this mod has been unlocked by the player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blessed Mod")
	bool bIsUnlocked = false;

	FBlessedModData()
		: ModID(NAME_None)
		, ModName(FText::GetEmpty())
		, Description(FText::GetEmpty())
		, Icon(nullptr)
		, bIsUnlocked(false)
	{
	}
};

/**
 * Cursed Angel transformation state
 * Tracks transformation meter, duration, and mesh references
 * Physics components automatically work with both base and transformed meshes
 */
USTRUCT(BlueprintType)
struct FCursedAngelState
{
	GENERATED_BODY()

	/** Current meter value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Angel")
	float MeterCurrent = 0.0f;

	/** Maximum meter value needed to activate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Angel")
	float MeterMax = 100.0f;

	/** Duration of transformation in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Angel")
	float Duration = 10.0f;

	/** Cooldown after transformation ends in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Angel")
	float Cooldown = 20.0f;

	/** Whether transformation is currently active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Angel")
	bool bIsActive = false;

	/** Skeletal mesh to use when transformed (Cat_Girl assets) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Angel")
	TObjectPtr<USkeletalMesh> TransformedMesh;

	/** Original skeletal mesh to restore after transformation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Angel")
	TObjectPtr<USkeletalMesh> BaseMesh;

	FCursedAngelState()
		: MeterCurrent(0.0f)
		, MeterMax(100.0f)
		, Duration(10.0f)
		, Cooldown(20.0f)
		, bIsActive(false)
		, TransformedMesh(nullptr)
		, BaseMesh(nullptr)
	{
	}
};

/**
 * Unlockable data structure
 * Generic structure for progression unlocks (skills, weapons, mods)
 */
USTRUCT(BlueprintType)
struct FUnlockableData
{
	GENERATED_BODY()

	/** Unique identifier for this unlockable */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlockable")
	FName UnlockableID;

	/** Display name of the unlockable */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlockable")
	FText DisplayName;

	/** Required curse to unlock this item (NAME_None if no requirement) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlockable")
	FName RequiredCurse;

	/** Data currency cost to unlock */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlockable")
	int32 DataCost = 0;

	/** Whether this has been unlocked by the player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlockable")
	bool bIsUnlocked = false;

	FUnlockableData()
		: UnlockableID(NAME_None)
		, DisplayName(FText::GetEmpty())
		, RequiredCurse(NAME_None)
		, DataCost(0)
		, bIsUnlocked(false)
	{
	}
};
