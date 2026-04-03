// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CombatTypes.h"
#include "EnemyBase.generated.h"

class AEnemyAIController;
class UHealthComponent;
class ACursedAngelCharacter;
class UBehaviorTree;
class UBlackboardData;
class UEnemyDataAsset;

/**
 * Delegate for enemy spawned event
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemySpawned);

/**
 * Delegate for enemy death event
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyDeath, AActor*, Killer);

/**
 * Delegate for enemy attack event
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackPerformed);

/**
 * Base class for all enemy characters in the game
 * Handles health, AI behavior, and combat interactions
 */
UCLASS()
class CURSEDANGEL_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyBase();

	/** Set the AI controller class for this enemy */
	virtual void PostInitializeComponents() override;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	// ========== Components ==========

	/** Health component for damage and death handling */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComponent;

	// ========== Properties ==========

	/** Enemy stats configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Stats")
	FEnemyStats Stats;

	/** Enemy configuration data asset (for data-driven initialization) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Configuration")
	UEnemyDataAsset* EnemyConfigAsset;

	/** Enemy type name (key into data asset) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Configuration")
	FName EnemyTypeName;

	/** Enemy type classification (Fodder, Elite, MiniBoss, Boss) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Stats")
	EEnemyType EnemyType;

	// ========== Behavior Tree Integration ==========

	/** Behavior Tree asset to use for this enemy (optional) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Behavior Tree")
	UBehaviorTree* BehaviorTreeAsset;

	/** Blackboard data asset for Behavior Tree */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Behavior Tree")
	UBlackboardData* BlackboardAsset;

	/** Whether this enemy uses Behavior Tree AI (if false, uses simple AI) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Behavior Tree")
	bool bUseBehaviorTree;

	// ========== Currency Drops ==========

	/** Configuration for currency drops on death */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Stats|Currency")
	FCurrencyDropConfig CurrencyConfig;

	// ========== Attack Patterns ==========

	/** Available attack patterns for this enemy */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack Patterns")
	TArray<FName> AttackPatterns;

	/** Current active attack pattern */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Attack Patterns")
	FName CurrentAttackPattern;

	/** Whether this enemy is currently aggro'd on the player */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	bool bIsAggro;

	/** Range at which enemy becomes aggro'd */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float AggroRange;

	/** Damage dealt by this enemy's attacks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackDamage;

	/** Cooldown between attacks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackCooldown;

	/** Time of last attack */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float LastAttackTime;

	/** Current target player */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	ACursedAngelCharacter* TargetPlayer;

	/** Delay before destroying actor after death */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Stats")
	float DeathDestroyDelay;

	// ========== Blueprint Events ==========

	/** Called when enemy is spawned */
	UPROPERTY(BlueprintAssignable, Category = "Enemy|Events")
	FOnEnemySpawned OnEnemySpawned;

	/** Called when enemy dies */
	UPROPERTY(BlueprintAssignable, Category = "Enemy|Events")
	FOnEnemyDeath OnEnemyDeath;

	/** Called when enemy performs an attack */
	UPROPERTY(BlueprintAssignable, Category = "Enemy|Events")
	FOnAttackPerformed OnAttackPerformed;

	// ========== Functions ==========

	/**
	 * Initialize enemy with stats
	 * @param InStats Enemy stats to apply (includes EnemyType, AttackPatterns, currency drops)
	 */
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void Initialize(const FEnemyStats& InStats);

	/**
	 * Load stats from data asset and initialize enemy
	 * Uses EnemyConfigAsset and EnemyTypeName to retrieve stats
	 */
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void LoadStatsFromDataAsset();

	/**
	 * Perform an attack on the target
	 */
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void PerformAttack();

	/**
	 * Find and set the player as target
	 * @return True if player was found
	 */
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	bool FindPlayerTarget();

	/**
	 * Get distance to the current target player
	 * @return Distance to player, or -1 if no target
	 */
	UFUNCTION(BlueprintPure, Category = "Enemy")
	float GetDistanceToPlayer() const;

	/**
	 * Check if attack is ready (cooldown elapsed)
	 * @return True if can attack
	 */
	UFUNCTION(BlueprintPure, Category = "Enemy")
	bool CanAttack() const;

	/**
	 * Spawn currency drops based on enemy type and configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Enemy|Currency")
	void SpawnCurrencyDrops();

	/**
	 * Select a random attack pattern from available patterns
	 */
	UFUNCTION(BlueprintCallable, Category = "Enemy|Combat")
	void SelectAttackPattern();

	/**
	 * Get the current attack pattern
	 * @return Current attack pattern name
	 */
	UFUNCTION(BlueprintPure, Category = "Enemy|Combat")
	FName GetCurrentAttackPattern() const;

protected:
	/**
	 * Called when this enemy dies
	 * @param Killer Actor that killed this enemy
	 * @param Type Type of damage that killed this enemy
	 */
	UFUNCTION()
	void OnDeath(AActor* Killer, EDamageType Type);

	/**
	 * Handle death logic (disable collision, notify systems, destroy)
	 */
	void HandleDeath(AActor* Killer);
};
