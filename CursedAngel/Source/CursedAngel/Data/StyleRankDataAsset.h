// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CombatTypes.h"
#include "StyleRankDataAsset.generated.h"

/**
 * Data Asset for configuring style rank system
 * Allows designers to configure rank thresholds, multipliers, decay rates, and point awards
 */
UCLASS(BlueprintType)
class CURSEDANGEL_API UStyleRankDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Map of style ranks to their configuration data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style Rank Configuration")
	TMap<EStyleRank, FStyleRankData> RankConfigs;

	/** Configuration for style points calculation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style Points Configuration")
	FStylePointsConfig PointsConfig;

	/** Get rank data for a specific style rank */
	UFUNCTION(BlueprintCallable, Category = "Style Rank Configuration")
	FStyleRankData GetRankData(EStyleRank Rank) const;

	/** Check if rank has configuration */
	UFUNCTION(BlueprintCallable, Category = "Style Rank Configuration")
	bool HasRankConfig(EStyleRank Rank) const;

	/** Get the rank for a given points value */
	UFUNCTION(BlueprintCallable, Category = "Style Rank Configuration")
	EStyleRank GetRankForPoints(float Points) const;

	/** Get style points configuration */
	UFUNCTION(BlueprintCallable, Category = "Style Points Configuration")
	FStylePointsConfig GetPointsConfig() const { return PointsConfig; }
};
