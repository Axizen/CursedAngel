// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CombatTypes.h"
#include "UtilityDataAsset.generated.h"

/**
 * Struct containing configuration data for a utility actor
 */
USTRUCT(BlueprintType)
struct FUtilityData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility")
	FName UtilityType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility")
	ECurseWeaponType RequiredWeapon = ECurseWeaponType::CorruptionRail;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility")
	bool bRequiresFrank = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility")
	float ActivationDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility")
	float CooldownDuration = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility")
	FText EffectDescription;

	FUtilityData()
		: UtilityType(NAME_None)
		, RequiredWeapon(ECurseWeaponType::CorruptionRail)
		, bRequiresFrank(true)
		, ActivationDuration(2.0f)
		, CooldownDuration(5.0f)
	{
	}
};

/**
 * Data asset for configuring utility actor types and their requirements
 * Allows designers to define utility types like doors, switches, platforms, and secrets
 */
UCLASS(BlueprintType)
class CURSEDANGEL_API UUtilityDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UUtilityDataAsset();

	/**
	 * Map of utility configurations keyed by utility type name
	 * Example types: Door, Switch, Platform, Secret
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility Configuration")
	TMap<FName, FUtilityData> UtilityConfigs;

	/**
	 * Get utility data for a specific utility type
	 * @param UtilityType The name of the utility type to retrieve
	 * @return The utility data if found, otherwise default FUtilityData
	 */
	UFUNCTION(BlueprintCallable, Category = "Utility")
	FUtilityData GetUtilityData(FName UtilityType) const;

	/**
	 * Check if a utility configuration exists for the given type
	 * @param UtilityType The name of the utility type to check
	 * @return True if the utility type exists in the configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Utility")
	bool HasUtilityConfig(FName UtilityType) const;
};
