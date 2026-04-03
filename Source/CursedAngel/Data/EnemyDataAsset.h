// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CombatTypes.h"
#include "EnemyDataAsset.generated.h"

/**
 * Data Asset for configuring enemy stats
 * Allows designers to configure all enemy types in the editor
 */
UCLASS(BlueprintType)
class CURSEDANGEL_API UEnemyDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Map of enemy type names to their configuration stats */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Configuration")
	TMap<FName, FEnemyStats> EnemyConfigs;

	/** Get enemy stats for a specific enemy type */
	UFUNCTION(BlueprintCallable, Category = "Enemy Configuration")
	FEnemyStats GetEnemyStats(FName EnemyType) const;

	/** Check if enemy type has configuration */
	UFUNCTION(BlueprintCallable, Category = "Enemy Configuration")
	bool HasEnemyConfig(FName EnemyType) const;

	/** Get all configured enemy type names */
	UFUNCTION(BlueprintCallable, Category = "Enemy Configuration")
	TArray<FName> GetAllEnemyTypes() const;
};
