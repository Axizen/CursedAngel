// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/UtilityDataAsset.h"

UUtilityDataAsset::UUtilityDataAsset()
{
	// Initialize with default utility configurations
	// Designers can override these in the editor
	
	// Door utility - requires FragmentNeedles + Frank
	FUtilityData DoorData;
	DoorData.UtilityType = FName("Door");
	DoorData.RequiredWeapon = ECurseWeaponType::FragmentNeedles;
	DoorData.bRequiresFrank = true;
	DoorData.ActivationDuration = 2.0f;
	DoorData.CooldownDuration = 0.0f;
	DoorData.EffectDescription = FText::FromString("Opens locked doors");
	UtilityConfigs.Add(FName("Door"), DoorData);

	// Switch utility - requires CorruptionRail + Frank
	FUtilityData SwitchData;
	SwitchData.UtilityType = FName("Switch");
	SwitchData.RequiredWeapon = ECurseWeaponType::CorruptionRail;
	SwitchData.bRequiresFrank = true;
	SwitchData.ActivationDuration = 1.5f;
	SwitchData.CooldownDuration = 5.0f;
	SwitchData.EffectDescription = FText::FromString("Activates switches and mechanisms");
	UtilityConfigs.Add(FName("Switch"), SwitchData);

	// Platform utility - requires VoidMines + Frank
	FUtilityData PlatformData;
	PlatformData.UtilityType = FName("Platform");
	PlatformData.RequiredWeapon = ECurseWeaponType::VoidMines;
	PlatformData.bRequiresFrank = true;
	PlatformData.ActivationDuration = 2.5f;
	PlatformData.CooldownDuration = 10.0f;
	PlatformData.EffectDescription = FText::FromString("Extends or moves platforms");
	UtilityConfigs.Add(FName("Platform"), PlatformData);

	// Secret utility - requires any weapon + Frank
	FUtilityData SecretData;
	SecretData.UtilityType = FName("Secret");
	SecretData.RequiredWeapon = ECurseWeaponType::CorruptionRail; // Default, but can be any
	SecretData.bRequiresFrank = true;
	SecretData.ActivationDuration = 3.0f;
	SecretData.CooldownDuration = 0.0f;
	SecretData.EffectDescription = FText::FromString("Reveals hidden secrets");
	UtilityConfigs.Add(FName("Secret"), SecretData);
}

FUtilityData UUtilityDataAsset::GetUtilityData(FName UtilityType) const
{
	if (const FUtilityData* FoundData = UtilityConfigs.Find(UtilityType))
	{
		return *FoundData;
	}

	// Return default utility data if not found
	return FUtilityData();
}

bool UUtilityDataAsset::HasUtilityConfig(FName UtilityType) const
{
	return UtilityConfigs.Contains(UtilityType);
}
