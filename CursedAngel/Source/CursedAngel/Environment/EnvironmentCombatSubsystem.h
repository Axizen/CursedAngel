// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "EnvironmentCombatSubsystem.generated.h"

class ACombatDebris;
class ADestructibleBase;

/** Level coordinator for magnetized debris and persistent encounter geometry changes. */
UCLASS(BlueprintType)
class CURSEDANGEL_API UEnvironmentCombatSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Environment Combat|Debris", meta=(ClampMin="1", ToolTip="Maximum active physics debris retained in the level. Oldest unheld debris is cleaned up when the cap is reached."))
	int32 MaxActiveDebris = 128;

	UFUNCTION(BlueprintCallable, Category="Environment Combat|Debris")
	void RegisterDebris(ACombatDebris* Debris);
	UFUNCTION(BlueprintCallable, Category="Environment Combat|Debris")
	void UnregisterDebris(ACombatDebris* Debris);
	UFUNCTION(BlueprintCallable, Category="Environment Combat|Magnet", meta=(ToolTip="Find and magnetically hold the nearest eligible debris within radius. Returns the debris actor, or null if none is available."))
	ACombatDebris* MagnetizeNearestDebris(AActor* Holder, float Radius);
	UFUNCTION(BlueprintCallable, Category="Environment Combat|Magnet", meta=(ToolTip="Launch debris currently held by Holder toward Target. LaunchSpeed is in cm/s."))
	bool LaunchHeldDebris(AActor* Holder, AActor* Target, float LaunchSpeed, float StyleMultiplier = 1.0f);
	UFUNCTION(BlueprintCallable, Category="Environment Combat|Magnet")
	void ReleaseHeldDebris(AActor* Holder);

	/** Records a broken bridge, support, or prop region for the lifetime of this encounter. */
	UFUNCTION(BlueprintCallable, Category="Environment Combat|Arena")
	void RecordArenaChange(FName RegionId, ADestructibleBase* Source);
	UFUNCTION(BlueprintPure, Category="Environment Combat|Arena")
	bool HasArenaRegionChanged(FName RegionId) const;
	UFUNCTION(BlueprintPure, Category="Environment Combat|Arena")
	TArray<FName> GetChangedArenaRegions() const;

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<ACombatDebris>> ActiveDebris;
	UPROPERTY(Transient)
	TSet<FName> ChangedArenaRegions;
	void EnforceDebrisBudget();
};
