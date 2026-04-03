// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TransformationTypes.h"
#include "TransformationDataAsset.generated.h"

/**
 * Data Asset for configuring transformation visuals, VFX, audio, and movement
 * Allows designers to configure all transformation parameters without C++ recompilation
 */
UCLASS(BlueprintType)
class CURSEDANGEL_API UTransformationDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UTransformationDataAsset();

	/** Visual configuration for transformation (meshes, materials, glow) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation")
	FTransformationVisuals Visuals;

	/** VFX configuration for transformation (activation, loop, wings, aura, trail) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation")
	FTransformationVFX VFXConfig;

	/** Audio configuration for transformation (activation, loop, deactivation, ambient) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation")
	FTransformationAudio AudioConfig;

	/** Movement configuration for transformation (speed, jump, air control, air dash) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation")
	FTransformationMovement MovementConfig;

	/** Get visual configuration */
	UFUNCTION(BlueprintCallable, Category = "Transformation")
	FTransformationVisuals GetVisualsConfig() const { return Visuals; }

	/** Get VFX configuration */
	UFUNCTION(BlueprintCallable, Category = "Transformation")
	FTransformationVFX GetVFXConfig() const { return VFXConfig; }

	/** Get audio configuration */
	UFUNCTION(BlueprintCallable, Category = "Transformation")
	FTransformationAudio GetAudioConfig() const { return AudioConfig; }

	/** Get movement configuration */
	UFUNCTION(BlueprintCallable, Category = "Transformation")
	FTransformationMovement GetMovementConfig() const { return MovementConfig; }
};
