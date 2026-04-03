// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CurseWeaponDataAsset.h"

FCurseWeaponStats UCurseWeaponDataAsset::GetWeaponStats(ECurseWeaponType WeaponType) const
{
	if (const FCurseWeaponStats* Stats = WeaponConfigs.Find(WeaponType))
	{
		return *Stats;
	}

	// Return default stats if not found
	return FCurseWeaponStats();
}

bool UCurseWeaponDataAsset::HasWeaponConfig(ECurseWeaponType WeaponType) const
{
	return WeaponConfigs.Contains(WeaponType);
}
