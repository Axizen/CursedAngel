// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "RipleyLinkedAnimInstance.generated.h"

class URipleyAnimInstance;

/**
 * URipleyLinkedAnimInstance - Linked animation instance for Ripley's layered animation
 *
 * Parent class for ABP_RipleyCA and ABP_RipleyNormal (linked animation layers).
 * Each frame, copies GroundSpeed and bIsInAir from the main URipleyAnimInstance so
 * that transition conditions in the AnimGraph work without duplicating BP event graph logic.
 *
 * Also exposes MainInstance so ABP_RipleyNormal can access SoftBody refs for Modify Bone nodes.
 */
UCLASS(Blueprintable)
class CURSEDANGEL_API URipleyLinkedAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	//~ Begin UAnimInstance Interface
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	//~ End UAnimInstance Interface

	// ===========================
	// Main Instance Reference
	// ===========================

	/** Reference to the main Ripley animation instance (URipleyAnimInstance on the primary mesh) */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Linked")
	TObjectPtr<URipleyAnimInstance> MainInstance;

	// ===========================
	// Locomotion State (mirrored from MainInstance)
	// ===========================

	/** Ground speed mirrored from the main animation instance */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	float GroundSpeed;

	/** True when the character is in the air, mirrored from the main animation instance */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	bool bIsInAir;

	// ===========================
	// Soft Body Component-Space Positions (mirrored from MainInstance)
	// ===========================

	/** Left breast soft body position in component space */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Soft Body")
	FVector SoftBodyCS_BreastL;

	/** Right breast soft body position in component space */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Soft Body")
	FVector SoftBodyCS_BreastR;

	/** Left thigh soft body position in component space */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Soft Body")
	FVector SoftBodyCS_ThighL;

	/** Right thigh soft body position in component space */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Soft Body")
	FVector SoftBodyCS_ThighR;

	/** Left butt soft body position in component space */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Soft Body")
	FVector SoftBodyCS_ButtL;

	/** Right butt soft body position in component space */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Soft Body")
	FVector SoftBodyCS_ButtR;
};
