// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Projectiles/CurseProjectile.h"
#include "CurseProjectileHitscan.generated.h"

/**
 * Hitscan projectile for CorruptionRail weapon
 * Performs instant line trace with multi-penetration support
 */
UCLASS()
class CURSEDANGEL_API ACurseProjectileHitscan : public ACurseProjectile
{
	GENERATED_BODY()
	
public:	
	ACurseProjectileHitscan();

	// ========== Properties ==========

	/** Maximum range of the hitscan trace */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitscan")
	float MaxRange;

	/** Whether to draw debug lines for the trace */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitscan")
	bool bDrawDebugLine;

	// ========== Functions ==========

	/**
	 * Override Initialize to perform hitscan immediately
	 * @param Stats Weapon stats to configure this projectile
	 * @param InOwner Character that fired this projectile
	 */
	virtual void Initialize(const FCurseWeaponStats& Stats, ARipley* InOwner) override;

protected:
	/**
	 * Perform the hitscan line trace
	 * Handles multi-penetration and applies damage to all hit actors
	 */
	void PerformHitscan();

	/**
	 * Process a single hit result
	 * @param Hit The hit result to process
	 * @return True if we should continue tracing (penetration)
	 */
	bool ProcessHit(const FHitResult& Hit);
};
