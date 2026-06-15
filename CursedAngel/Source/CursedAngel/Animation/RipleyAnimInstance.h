// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/CursedAngelAnimInstance.h"
#include "RipleyAnimInstance.generated.h"

class ARipley;
class USoftBodyPhysicsComponent;

/**
 * URipleyAnimInstance - Animation instance for the Ripley character
 *
 * Extends UCursedAngelAnimInstance to add Ripley-specific animation state:
 * - Caches references to Ripley's 6 SoftBody physics components
 * - Tracks transformation state (bIsTransformed) from UCursedAngelComponent
 *
 * Parent class for ABP_Ripley.
 * Linked layers (ABP_RipleyCA / ABP_RipleyNormal) access these props via URipleyLinkedAnimInstance.
 */
UCLASS(Blueprintable)
class CURSEDANGEL_API URipleyAnimInstance : public UCursedAngelAnimInstance
{
	GENERATED_BODY()

public:
	//~ Begin UAnimInstance Interface
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	//~ End UAnimInstance Interface

	// ===========================
	// Soft Body References
	// ===========================

	/** Soft body component for left breast simulation */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Soft Body")
	TObjectPtr<USoftBodyPhysicsComponent> SoftBody_BreastL;

	/** Soft body component for right breast simulation */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Soft Body")
	TObjectPtr<USoftBodyPhysicsComponent> SoftBody_BreastR;

	/** Soft body component for left thigh simulation */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Soft Body")
	TObjectPtr<USoftBodyPhysicsComponent> SoftBody_ThighL;

	/** Soft body component for right thigh simulation */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Soft Body")
	TObjectPtr<USoftBodyPhysicsComponent> SoftBody_ThighR;

	/** Soft body component for left butt simulation */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Soft Body")
	TObjectPtr<USoftBodyPhysicsComponent> SoftBody_ButtL;

	/** Soft body component for right butt simulation */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Soft Body")
	TObjectPtr<USoftBodyPhysicsComponent> SoftBody_ButtR;

	// ===========================
	// Transformation State
	// ===========================

	/** True when Ripley is in Cursed Angel transformed state */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Transformation")
	bool bIsTransformed;

	// ===========================
	// Soft Body Component-Space Positions
	// ===========================

	/** Left breast soft body position in component space (pre-computed on game thread) */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Soft Body")
	FVector SoftBodyCS_BreastL;

	/** Right breast soft body position in component space (pre-computed on game thread) */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Soft Body")
	FVector SoftBodyCS_BreastR;

	/** Left thigh soft body position in component space (pre-computed on game thread) */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Soft Body")
	FVector SoftBodyCS_ThighL;

	/** Right thigh soft body position in component space (pre-computed on game thread) */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Soft Body")
	FVector SoftBodyCS_ThighR;

	/** Left butt soft body position in component space (pre-computed on game thread) */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Soft Body")
	FVector SoftBodyCS_ButtL;

	/** Right butt soft body position in component space (pre-computed on game thread) */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Soft Body")
	FVector SoftBodyCS_ButtR;

protected:
	/** Cached reference to the owning ARipley character */
	UPROPERTY()
	TObjectPtr<ARipley> RipleyOwner;
};
