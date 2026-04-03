// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/BlessedModDataAsset.h"

bool UBlessedModDataAsset::IsCompatibleWithWeapon(ECurseWeaponType WeaponType) const
{
	// Universal mods (empty array) work on all weapons
	if (CompatibleWeapons.IsEmpty())
	{
		return true;
	}

	// Weapon-specific mods check compatibility array
	return CompatibleWeapons.Contains(WeaponType);
}

FPrimaryAssetId UBlessedModDataAsset::GetPrimaryAssetId() const
{
	// Return FPrimaryAssetId with type 'BlessedMod' and name from ModName
	// This is required for asset manager integration (future save/load system)
	return FPrimaryAssetId(TEXT("BlessedMod"), FName(*ModName.ToString()));
}
