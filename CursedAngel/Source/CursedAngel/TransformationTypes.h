// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraSystem.h"
#include "TransformationTypes.generated.h"

/**
 * Enum representing the current state of transformation
 */
UENUM(BlueprintType)
enum class ETransformationState : uint8
{
	Normal UMETA(DisplayName = "Normal"),
	Transforming UMETA(DisplayName = "Transforming"),
	Transformed UMETA(DisplayName = "Transformed"),
	Reverting UMETA(DisplayName = "Reverting")
};

/**
 * Struct containing visual configuration for transformation
 */
USTRUCT(BlueprintType)
struct FTransformationVisuals
{
	GENERATED_BODY()

	/** Mesh used in normal state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation|Visuals")
	USkeletalMesh* NormalMesh;

	/** Mesh used in transformed state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation|Visuals")
	USkeletalMesh* TransformedMesh;

	/** Materials used in normal state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation|Visuals")
	TArray<UMaterialInterface*> NormalMaterials;

	/** Materials used in transformed state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation|Visuals")
	TArray<UMaterialInterface*> TransformedMaterials;

	/** Emissive intensity for glow effect during transformation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation|Visuals")
	float EmissiveIntensity;

	/** Color of the glow effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation|Visuals")
	FLinearColor GlowColor;

	FTransformationVisuals()
		: NormalMesh(nullptr)
		, TransformedMesh(nullptr)
		, EmissiveIntensity(5.0f)
		, GlowColor(FLinearColor(0.0f, 1.0f, 1.0f, 1.0f))
	{
	}
};

/**
 * Struct containing VFX configuration for transformation
 */
USTRUCT(BlueprintType)
struct FTransformationVFX
{
	GENERATED_BODY()

	/** VFX played when transformation activates */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation|VFX")
	UNiagaraSystem* ActivationVFX;

	/** VFX that loops during transformation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation|VFX")
	UNiagaraSystem* LoopVFX;

	/** VFX played when transformation ends */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation|VFX")
	UNiagaraSystem* DeactivationVFX;

	/** VFX for wings during transformation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation|VFX")
	UNiagaraSystem* WingVFX;

	/** VFX for aura around character */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation|VFX")
	UNiagaraSystem* AuraVFX;

	/** VFX for movement trail */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation|VFX")
	UNiagaraSystem* TrailVFX;

	FTransformationVFX()
		: ActivationVFX(nullptr)
		, LoopVFX(nullptr)
		, DeactivationVFX(nullptr)
		, WingVFX(nullptr)
		, AuraVFX(nullptr)
		, TrailVFX(nullptr)
	{
	}
};

/**
 * Struct containing audio configuration for transformation
 */
USTRUCT(BlueprintType)
struct FTransformationAudio
{
	GENERATED_BODY()

	/** Sound played when transformation activates */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation|Audio")
	USoundBase* ActivationSound;

	/** Sound that loops during transformation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation|Audio")
	USoundBase* LoopSound;

	/** Sound played when transformation ends */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation|Audio")
	USoundBase* DeactivationSound;

	/** Ambient sound during transformation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation|Audio")
	USoundBase* AmbientSound;

	FTransformationAudio()
		: ActivationSound(nullptr)
		, LoopSound(nullptr)
		, DeactivationSound(nullptr)
		, AmbientSound(nullptr)
	{
	}
};

/**
 * Struct containing movement configuration for transformation
 */
USTRUCT(BlueprintType)
struct FTransformationMovement
{
	GENERATED_BODY()

	/** Multiplier for movement speed during transformation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation|Movement")
	float SpeedMultiplier;

	/** Multiplier for jump height during transformation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation|Movement")
	float JumpMultiplier;

	/** Multiplier for air control during transformation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation|Movement")
	float AirControlMultiplier;

	/** Speed of air dash ability */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation|Movement")
	float AirDashSpeed;

	/** Cooldown between air dashes in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation|Movement")
	float AirDashCooldown;

	FTransformationMovement()
		: SpeedMultiplier(1.5f)
		, JumpMultiplier(1.3f)
		, AirControlMultiplier(1.5f)
		, AirDashSpeed(2000.0f)
		, AirDashCooldown(1.0f)
	{
	}
};
