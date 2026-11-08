// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "EnvironmentCombatSubsystem.generated.h"

class ACombatDebris;
class ADestructibleBase;

/** Encounter-persistent state for a named structural route or arena region. */
USTRUCT(BlueprintType)
struct CURSEDANGEL_API FArenaRegionState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Environment Combat|Arena")
	FName RegionId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Environment Combat|Arena")
	bool bChanged = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Environment Combat|Arena")
	int32 StructuralPiecesBroken = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Environment Combat|Arena")
	float LastImpactSpeed = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnArenaRegionChanged, FName, RegionId, ADestructibleBase*, Source);

/**
 * Level coordinator for magnetized debris and persistent encounter geometry changes.
 * Owns encounter-local state only; it does not persist damage into future level loads.
 */
UCLASS(BlueprintType)
class CURSEDANGEL_API UEnvironmentCombatSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Environment Combat|Debris", meta=(ClampMin="1", ToolTip="Maximum active physics debris retained in the level. Oldest unheld debris is cleaned up when the cap is reached."))
	int32 MaxActiveDebris = 128;

	UPROPERTY(BlueprintAssignable, Category="Environment Combat|Arena|Events", meta=(ToolTip="Broadcast when a named region changes during this encounter. Bind route, platforming, enemy, or HUD reactions here."))
	FOnArenaRegionChanged OnArenaRegionChanged;

	UFUNCTION(BlueprintCallable, Category="Environment Combat|Debris")
	void RegisterDebris(ACombatDebris* Debris);
	UFUNCTION(BlueprintCallable, Category="Environment Combat|Debris")
	void UnregisterDebris(ACombatDebris* Debris);
	UFUNCTION(BlueprintCallable, Category="Environment Combat|Magnet", meta=(ToolTip="Find and magnetically hold the nearest eligible debris within radius. Returns null when no debris can be picked up."))
	ACombatDebris* MagnetizeNearestDebris(AActor* Holder, float Radius);
	UFUNCTION(BlueprintCallable, Category="Environment Combat|Magnet", meta=(ToolTip="Launch debris held by Holder toward Target. LaunchSpeed is in cm/s; StyleMultiplier scales impact damage."))
	bool LaunchHeldDebris(AActor* Holder, AActor* Target, float LaunchSpeed, float StyleMultiplier = 1.0f);
	UFUNCTION(BlueprintCallable, Category="Environment Combat|Magnet")
	void ReleaseHeldDebris(AActor* Holder);

	/** Records a broken bridge, support, or prop region for the lifetime of this encounter. */
	UFUNCTION(BlueprintCallable, Category="Environment Combat|Arena")
	void RecordArenaChange(FName RegionId, ADestructibleBase* Source);
	UFUNCTION(BlueprintCallable, Category="Environment Combat|Arena")
	void RecordStructuralPieceBroken(FName RegionId, ADestructibleBase* Source, float ImpactSpeed);
	UFUNCTION(BlueprintPure, Category="Environment Combat|Arena")
	bool HasArenaRegionChanged(FName RegionId) const;
	UFUNCTION(BlueprintPure, Category="Environment Combat|Arena")
	FArenaRegionState GetArenaRegionState(FName RegionId) const;
	UFUNCTION(BlueprintPure, Category="Environment Combat|Arena")
	TArray<FName> GetChangedArenaRegions() const;

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<ACombatDebris>> ActiveDebris;
	UPROPERTY(Transient)
	TMap<FName, FArenaRegionState> ArenaRegions;
	void EnforceDebrisBudget();
};
