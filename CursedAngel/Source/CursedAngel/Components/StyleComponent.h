// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatTypes.h"
#include "Components/CurseWeaponComponent.h"
#include "StyleComponent.generated.h"

/**
 * Style Component - Tracks combat performance and manages DMC-style ranking system
 * Handles combo tracking, weapon variety detection, timing bonuses, and rank progression
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CURSEDANGEL_API UStyleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStyleComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ========== Properties ==========

	/** Current accumulated style points */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Style System")
	float CurrentStylePoints;

	/** Current style rank (D/C/B/A/S) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Style System")
	EStyleRank CurrentRank;

	/** Current combo count (successive kills without breaking) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Style System")
	int32 ComboCount;

	/** Time of last hit (for decay calculation) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Style System")
	float LastHitTime;

	/** Last weapon used (for variety detection) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Style System")
	ECurseWeaponType LastWeaponUsed;

	/** Count of environmental kills in current combo */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Style System")
	int32 EnvironmentalKillCount;

	/** Configuration for style points calculation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style System|Config")
	FStylePointsConfig StylePointsConfig;

	/** Map of rank data (thresholds, multipliers, decay rates) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style System|Config")
	TMap<EStyleRank, FStyleRankData> RankDataMap;

	/** Time in seconds before combo starts decaying (no hits) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style System|Config")
	float ComboDecayDelay = 3.0f;

	/** Time window in seconds for timing bonus (quick successive kills) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style System|Config")
	float TimingBonusWindow = 2.0f;

	// ========== Functions ==========

	/**
	 * Add style points to the current total and update rank if necessary
	 * @param Points - Amount of points to add
	 */
	UFUNCTION(BlueprintCallable, Category = "Style System")
	void AddStylePoints(float Points);

	/**
	 * Called when an enemy is killed - calculates and awards style points
	 * @param Enemy - The killed enemy actor
	 * @param Weapon - The weapon used for the kill
	 * @param bEnvironmental - Whether this was an environmental kill
	 */
	UFUNCTION(BlueprintCallable, Category = "Style System")
	void OnEnemyKilled(AActor* Enemy, ECurseWeaponType Weapon, bool bEnvironmental);

	/**
	 * Update the current style rank based on accumulated points
	 */
	UFUNCTION(BlueprintCallable, Category = "Style System")
	void UpdateRank();

	/**
	 * Decay style points over time when no hits are registered
	 * @param DeltaTime - Time since last frame
	 */
	UFUNCTION(BlueprintCallable, Category = "Style System")
	void DecayStylePoints(float DeltaTime);

	/**
	 * Reset the combo counter and related tracking
	 */
	UFUNCTION(BlueprintCallable, Category = "Style System")
	void ResetCombo();

	/**
	 * Get the damage multiplier for the current style rank
	 * @return Damage multiplier value
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Style System")
	float GetDamageMultiplier() const;

	/**
	 * Get the current style rank
	 * @return Current EStyleRank
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Style System")
	EStyleRank GetCurrentRank() const { return CurrentRank; }

	/**
	 * Get the current style points
	 * @return Current style points value
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Style System")
	float GetCurrentStylePoints() const { return CurrentStylePoints; }

	/**
	 * Get the current combo count
	 * @return Current combo count
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Style System")
	int32 GetComboCount() const { return ComboCount; }

	// ========== Blueprint Events ==========

	/**
	 * Called when the style rank changes
	 * @param NewRank - The new rank
	 * @param OldRank - The previous rank
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Style System|Events")
	void OnRankChanged(EStyleRank NewRank, EStyleRank OldRank);

	/**
	 * Called when style points change
	 * @param Points - Current style points
	 * @param Rank - Current style rank
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Style System|Events")
	void OnStylePointsChanged(float Points, EStyleRank Rank);

	/**
	 * Called when combo count increases
	 * @param NewComboCount - The new combo count
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Style System|Events")
	void OnComboIncreased(int32 NewComboCount);

private:
	/**
	 * Initialize default rank data if not configured
	 */
	void InitializeDefaultRankData();

	/**
	 * Calculate style points for a kill based on various factors
	 * @param Weapon - Weapon used for the kill
	 * @param bEnvironmental - Whether this was an environmental kill
	 * @return Calculated style points
	 */
	float CalculateKillStylePoints(ECurseWeaponType Weapon, bool bEnvironmental);

	/**
	 * Check if weapon variety bonus should be applied
	 * @param Weapon - Current weapon used
	 * @return True if variety bonus applies
	 */
	bool ShouldApplyWeaponVarietyBonus(ECurseWeaponType Weapon) const;

	/**
	 * Check if timing bonus should be applied (quick successive kills)
	 * @return True if timing bonus applies
	 */
	bool ShouldApplyTimingBonus() const;
};
