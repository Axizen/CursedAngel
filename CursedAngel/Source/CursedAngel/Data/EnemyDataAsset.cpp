// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/EnemyDataAsset.h"

FEnemyStats UEnemyDataAsset::GetEnemyStats(FName EnemyType) const
{
	if (const FEnemyStats* Stats = EnemyConfigs.Find(EnemyType))
	{
		return *Stats;
	}

	// Return default stats if not found
	return FEnemyStats();
}

bool UEnemyDataAsset::HasEnemyConfig(FName EnemyType) const
{
	return EnemyConfigs.Contains(EnemyType);
}

TArray<FName> UEnemyDataAsset::GetAllEnemyTypes() const
{
	TArray<FName> EnemyTypes;
	EnemyConfigs.GetKeys(EnemyTypes);
	return EnemyTypes;
}
