// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CombatTypes.h"
#include "CharacterDataAsset.generated.h"

// Forward declarations
class UBehaviorTree;
class UActionDataAsset;
class UAIBehaviorBase;

/**
 * Character role enum for determining character behavior
 */
UENUM(BlueprintType)
enum class ECharacterRole : uint8
{
	Player UMETA(DisplayName = "Player"),
	Companion UMETA(DisplayName = "Companion"),
	NPC UMETA(DisplayName = "NPC")
};

/**
 * Data asset containing character configuration including stats, weapons, moveset, and AI behavior.
 * Used for data-driven character initialization (Ripley, Frank, etc.).
 *
 * Each character (Ripley, Frank) has their own instance of this asset, allowing independent
 * tuning of movement feel, combat feel, and all base stats without touching C++ code.
 *
 * Feel configs:
 *   - MovementFeel (FMovementFeelConfig): R&C-inspired platforming parameters (speed, jump, coyote time, etc.)
 *   - CombatFeel (FCombatFeelConfig): DMC-inspired combat parameters (combo windows, hitstop, dash, dodge)
 *
 * Programmer: ApplyMovementFeelConfig() in ACursedAngelCharacter reads MovementFeel.
 *             ActionComponent reads CombatFeel.ComboWindowDuration/ComboBufferExpiry.
 *             Action_MeleeAttack reads CombatFeel.HitstopDuration/HitstopTimeDilation.
 *             Action_AirDash/Action_Dodge read CombatFeel dash/dodge params.
 */
UCLASS(BlueprintType)
class CURSEDANGEL_API UCharacterDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// ===========================
	// Base Stats
	// ===========================
	
	/** Maximum health points */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
	float MaxHealth = 100.0f;

	// ===========================
	// Movement Feel (R&C Platforming)
	// ===========================

	/** Designer-tunable platforming feel parameters. Each character (Ripley, Frank) has their own instance.
	 *  These are the authoritative movement parameters for this character.
	 *  Inspired by Ratchet & Clank responsive platforming: snappy ground movement, good air control,
	 *  coyote time, variable jump height, and squash-on-landing polish.
	 *
	 *  Programmer: Applied via ACursedAngelCharacter::ApplyMovementFeelConfig(), called from
	 *  InitializeFromDataAsset(). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Feel",
		meta = (Tooltip = "Full platforming feel config for this character."))
	FMovementFeelConfig MovementFeel;

	// ===========================
	// Combat Feel (DMC Combat)
	// ===========================

	/** Designer-tunable combat responsiveness parameters. Each character has their own instance.
	 *  Controls combo windows, hitstop freeze frames, air dash behavior, and dodge i-frame windows.
	 *  Inspired by Devil May Cry snappy, satisfying combat feel.
	 *
	 *  Programmer: Read by ActionComponent (ComboWindowDuration/ComboBufferExpiry),
	 *  ACursedAngelCharacter (DashSpeed/DashDuration/MaxAirDashes/DodgeSpeed/DodgeDuration),
	 *  Action_MeleeAttack (HitstopDuration/HitstopTimeDilation/MeleeLaunchImpulse/AttackCancelWindow),
	 *  Action_Dodge (DodgeIFrameStart/DodgeIFrameEnd), Action_AirDash (DashInputDirectionBlend). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Feel",
		meta = (Tooltip = "Full combat feel config. Controls combo windows, hitstop, dash, and dodge parameters."))
	FCombatFeelConfig CombatFeel;

	// ===========================
	// Combat Stats
	// ===========================
	
	/** Base melee damage per hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float MeleeDamage = 25.0f;

	/** Base ranged damage per projectile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float RangedDamage = 20.0f;

	/** Attack speed multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackSpeed = 1.0f;

	// ===========================
	// Weapon Loadout
	// ===========================
	
	/** Array of curse weapons available to this character */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
	TArray<ECurseWeaponType> AvailableWeapons;

	/** Default weapon to equip on spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
	ECurseWeaponType StartingWeapon;

	// ===========================
	// Moveset Configuration
	// ===========================
	
	/** Melee combo chain animation names */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moveset")
	TArray<FName> MeleeComboChain;

	/** Special ability animation/montage names */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moveset")
	TArray<FName> SpecialAbilities;

	// ===========================
	// Character Role
	// ===========================
	
	/** Role of this character (Player, Companion, NPC) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	ECharacterRole CharacterRole = ECharacterRole::Player;

	// ===========================
	// AI Configuration (Companion only)
	// ===========================
	
	/** Behavior tree for AI companion mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (EditCondition = "CharacterRole == ECharacterRole::Companion", EditConditionHides))
	UBehaviorTree* CompanionBehaviorTree;

	/** Distance to maintain from player when following */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (EditCondition = "CharacterRole == ECharacterRole::Companion", EditConditionHides))
	float FollowDistance = 300.0f;

	/** Range at which companion engages enemies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (EditCondition = "CharacterRole == ECharacterRole::Companion", EditConditionHides))
	float CombatRange = 500.0f;

	/** AI behaviors for companion characters (data-driven AI behavior system) */
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category = "AI", meta = (EditCondition = "CharacterRole == ECharacterRole::Companion", EditConditionHides))
	TArray<TObjectPtr<UAIBehaviorBase>> AIBehaviors;

	// ===========================
	// Action System
	// ===========================
	
	/** Available actions for this character (data-driven action system) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actions")
	TArray<UActionDataAsset*> AvailableActions;
};
