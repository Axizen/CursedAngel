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
 * Data asset containing character configuration including stats, weapons, moveset, and AI behavior
 * Used for data-driven character initialization (Ripley, Frank, etc.)
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

	/** Movement speed (units per second) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
	float MoveSpeed = 600.0f;

	/** Jump height (Z velocity) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
	float JumpHeight = 600.0f;

	/** Number of air dashes available */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
	int32 AirDashCount = 1;

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
