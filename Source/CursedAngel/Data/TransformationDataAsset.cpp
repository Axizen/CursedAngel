// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/TransformationDataAsset.h"

UTransformationDataAsset::UTransformationDataAsset()
{
	// Initialize with default values from structs
	Visuals = FTransformationVisuals();
	VFXConfig = FTransformationVFX();
	AudioConfig = FTransformationAudio();
	MovementConfig = FTransformationMovement();
}
