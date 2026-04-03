// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameplayTagContainer.h"
#include "CursedAngelAnimInstance.generated.h"

class ACursedAngelCharacter;
class UAnimMontage;

/**
 * Animation form types for linked layer switching
 */
UENUM(BlueprintType)
enum class EAnimationForm : uint8
{
	Normal UMETA(DisplayName = "Normal Form"),
	CursedAngel UMETA(DisplayName = "Cursed Angel Form")
};

/**
 * Anim instance for CursedAngel character with linked animation layers,
 * mesh swapping, and comprehensive state management
 */
UCLASS(Blueprintable)
class CURSEDANGEL_API UCursedAngelAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UCursedAngelAnimInstance();

	//~ Begin UAnimInstance Interface
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	//~ End UAnimInstance Interface

	// ===========================
	// Transformation Properties
	// ===========================

	/** Current animation form (Normal or Cursed Angel) */
	UPROPERTY(BlueprintReadWrite, Category = "Animation|Transformation")
	EAnimationForm CurrentForm;

	/** Animation layer class for Normal form */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Transformation")
	TSubclassOf<UAnimInstance> NormalFormLayerClass;

	/** Animation layer class for Cursed Angel form */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Transformation")
	TSubclassOf<UAnimInstance> CursedAngelFormLayerClass;

	/** Skeletal mesh for Normal form (Mechanic_Girl) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Transformation")
	TObjectPtr<USkeletalMesh> NormalFormMesh;

	/** Skeletal mesh for Cursed Angel form (Cat_Girl) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Transformation")
	TObjectPtr<USkeletalMesh> CursedAngelFormMesh;

	// ===========================
	// Transformation Functions
	// ===========================

	/** Switch to Normal form with linked layer and mesh swap */
	UFUNCTION(BlueprintCallable, Category = "Animation|Transformation")
	void SwitchToNormalForm();

	/** Switch to Cursed Angel form with linked layer and mesh swap */
	UFUNCTION(BlueprintCallable, Category = "Animation|Transformation")
	void SwitchToCursedAngelForm();

	/** Swap skeletal mesh based on animation form */
	UFUNCTION(BlueprintCallable, Category = "Animation|Transformation")
	void SwapMesh(EAnimationForm Form);

	/** Called when animation form changes (Blueprint event) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Transformation")
	void OnFormChanged(EAnimationForm NewForm);

	// ===========================
	// Locomotion Properties
	// ===========================

	/** Current movement speed */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	float Speed;

	/** Movement direction relative to character rotation (-180 to 180) */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	float Direction;

	/** Is character in air (jumping/falling) */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	bool bIsInAir;

	/** Is character crouching */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	bool bIsCrouching;

	/** Character velocity vector */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	FVector Velocity;

	/** Ground speed (2D velocity magnitude) */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
	float GroundSpeed;

	// ===========================
	// Combat Properties
	// ===========================

	/** Is character performing attack */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
	bool bIsAttacking;

	/** Is character dodging */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
	bool bIsDodging;

	/** Is character blocking */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
	bool bIsBlocking;

	/** Is character stunned */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
	bool bIsStunned;

	/** Current combo attack index */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
	int32 ComboIndex;

	/** Is character aiming (ranged weapon) */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
	bool bIsAiming;

	// ===========================
	// Foot IK Properties
	// ===========================

	/** Left foot IK vertical offset */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	float LeftFootIKOffset;

	/** Right foot IK vertical offset */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	float RightFootIKOffset;

	/** Left foot rotation for IK */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	FRotator LeftFootRotation;

	/** Right foot rotation for IK */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	FRotator RightFootRotation;

	/** Hip vertical offset for IK */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	float HipOffset;

	// ===========================
	// Montage Functions
	// ===========================

	/** Play animation montage with custom blend time */
	UFUNCTION(BlueprintCallable, Category = "Animation|Montage")
	void PlayMontageWithBlend(UAnimMontage* MontageToPlay, float BlendInTime = 0.25f);

	/** Stop animation montage with custom blend time */
	UFUNCTION(BlueprintCallable, Category = "Animation|Montage")
	void StopMontageWithBlend(UAnimMontage* MontageToStop, float BlendOutTime = 0.25f);

protected:
	// ===========================
	// Protected Members
	// ===========================

	/** Cached reference to owning character */
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	TObjectPtr<ACursedAngelCharacter> OwnerCharacter;

	// ===========================
	// Protected Functions
	// ===========================

	/** Update all animation variables (locomotion, combat, IK) */
	void UpdateAnimationVariables();

	/** Update locomotion-related variables (speed, direction, etc.) */
	void UpdateLocomotionVariables();

	/** Update combat-related variables (attacking, dodging, etc.) */
	void UpdateCombatVariables();

	/** Update foot IK offsets and rotations */
	void UpdateFootIK();

	/** Link animation layer class by name */
	void LinkAnimationLayer(TSubclassOf<UAnimInstance> LayerClass, FName LayerName);
};
