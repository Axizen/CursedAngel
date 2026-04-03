// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CombatTypes.h"
#include "CurseWeaponDataAsset.generated.h"

/**
 * Data Asset for configuring curse weapon stats
 * Allows designers to configure all weapon types in the editor
 */
UCLASS(BlueprintType)
class CURSEDANGEL_API UCurseWeaponDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Map of weapon types to their configuration stats */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Configuration")
	TMap<ECurseWeaponType, FCurseWeaponStats> WeaponConfigs;

	/** Get weapon stats for a specific weapon type */
	UFUNCTION(BlueprintCallable, Category = "Weapon Configuration")
	FCurseWeaponStats GetWeaponStats(ECurseWeaponType WeaponType) const;

	/** Check if weapon type has configuration */
	UFUNCTION(BlueprintCallable, Category = "Weapon Configuration")
	bool HasWeaponConfig(ECurseWeaponType WeaponType) const;
};
