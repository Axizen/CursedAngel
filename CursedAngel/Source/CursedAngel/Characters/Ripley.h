// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CursedAngelCharacter.h"
#include "Physics/SoftBodyPhysicsComponent.h"
#include "Ripley.generated.h"

// Forward declarations
// TODO: Create these components in future steps
// class UDataManagerComponent;
// class UWeaponManagerComponent;
// class UBlessedModManagerComponent;
class UHairPhysicsComponent;
class UClothPhysicsComponent;

/**
 * ARipley - Ranged combat specialist character
 *
 * Inherits from ACursedAngelCharacter to gain:
 * - Camera system
 * - Input system
 * - Combat components (CurseWeaponComponent, CursedAngelComponent, StyleComponent)
 * - Base combat functions
 * - Air dash, coyote time, and variable jump (now fully implemented in base class)
 *
 * Adds Ripley-specific features:
 * - Transformation movement modifiers (speed/jump multipliers during Cursed Angel mode)
 * - Weapon management system
 * - Blessed mod system
 * - Advanced physics (hair, cloth, soft body)
 * - Ranged combat focus (70% ranged, 30% melee)
 *
 * Programmer: Air dash is no longer a separate component — it lives in ACursedAngelCharacter.
 * ApplyTransformationMovementModifiers() now uses the MovementFeel config from CharacterDataAsset
 * as the base for multiplier calculations. RevertTransformationMovementModifiers() calls
 * ApplyMovementFeelConfig() for a clean data-asset-driven restore.
 */
UCLASS(config=Game)
class ARipley : public ACursedAngelCharacter
{
	GENERATED_BODY()

	// TODO: Add these components in future steps
	// /** Data Manager Component - Manages persistent player data */
	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Data, meta = (AllowPrivateAccess = "true"))
	// UDataManagerComponent* DataManagerComponent;

	// /** Weapon Manager Component - Manages weapon inventory and switching */
	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	// UWeaponManagerComponent* WeaponManagerComponent;

	// /** Blessed Mod Manager Component - Manages blessed mod system */
	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	// UBlessedModManagerComponent* BlessedModManagerComponent;

	/** Hair Physics Component - Advanced hair simulation */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Physics, meta = (AllowPrivateAccess = "true"))
	UHairPhysicsComponent* HairPhysicsComponent;

	/** Cloth Physics Component - Cloth simulation for outfit */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Physics, meta = (AllowPrivateAccess = "true"))
	UClothPhysicsComponent* ClothPhysicsComponent;

	/** Soft Body Physics Component - Advanced soft body simulation */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Physics, meta = (AllowPrivateAccess = "true"))
	USoftBodyPhysicsComponent* SoftBodyPhysicsComponent;

	/** Soft Body - Left breast simulation */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics|Soft Body", meta = (AllowPrivateAccess = "true"))
	USoftBodyPhysicsComponent* SoftBody_BreastL;

	/** Soft Body - Right breast simulation */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics|Soft Body", meta = (AllowPrivateAccess = "true"))
	USoftBodyPhysicsComponent* SoftBody_BreastR;

	/** Soft Body - Left thigh simulation */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics|Soft Body", meta = (AllowPrivateAccess = "true"))
	USoftBodyPhysicsComponent* SoftBody_ThighL;

	/** Soft Body - Right thigh simulation */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics|Soft Body", meta = (AllowPrivateAccess = "true"))
	USoftBodyPhysicsComponent* SoftBody_ThighR;

	/** Soft Body - Left butt simulation */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics|Soft Body", meta = (AllowPrivateAccess = "true"))
	USoftBodyPhysicsComponent* SoftBody_ButtL;

	/** Soft Body - Right butt simulation */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics|Soft Body", meta = (AllowPrivateAccess = "true"))
	USoftBodyPhysicsComponent* SoftBody_ButtR;


public:
	ARipley();

protected:
	/** Override virtual functions from base class */
	virtual void OnCombatAction_Implementation() override;
	virtual void OnStyleRankChanged_Implementation(EStyleRank NewRank) override;
	virtual void OnCursedAngelActivated_Implementation() override;
	virtual void OnCursedAngelDeactivated_Implementation() override;

public:
	/** Ripley-specific combat and movement functions */

	/** Apply transformation movement modifiers */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void ApplyTransformationMovementModifiers();

	/** Revert transformation movement modifiers */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void RevertTransformationMovementModifiers();

	/** Called when transformation state changes */
	UFUNCTION()
	void OnTransformationStateChanged(bool bTransformed);

private:
	/** Stored original movement values */
	float OriginalMaxWalkSpeed;
	float OriginalJumpHeight;
	float OriginalAirControl;
	float OriginalCameraFOV;

public:
	/** Ripley-specific component getters */
	// TODO: Add getters for these components in future steps
	// FORCEINLINE class UDataManagerComponent* GetDataManagerComponent() const { return DataManagerComponent; }
	// FORCEINLINE class UWeaponManagerComponent* GetWeaponManagerComponent() const { return WeaponManagerComponent; }
	// FORCEINLINE class UBlessedModManagerComponent* GetBlessedModManagerComponent() const { return BlessedModManagerComponent; }
	FORCEINLINE class UHairPhysicsComponent* GetHairPhysicsComponent() const { return HairPhysicsComponent; }
	FORCEINLINE class UClothPhysicsComponent* GetClothPhysicsComponent() const { return ClothPhysicsComponent; }
	FORCEINLINE USoftBodyPhysicsComponent* GetSoftBodyPhysicsComponent() const { return SoftBodyPhysicsComponent; }
	FORCEINLINE USoftBodyPhysicsComponent* GetSoftBody_BreastL() const { return SoftBody_BreastL; }
	FORCEINLINE USoftBodyPhysicsComponent* GetSoftBody_BreastR() const { return SoftBody_BreastR; }
	FORCEINLINE USoftBodyPhysicsComponent* GetSoftBody_ThighL() const { return SoftBody_ThighL; }
	FORCEINLINE USoftBodyPhysicsComponent* GetSoftBody_ThighR() const { return SoftBody_ThighR; }
	FORCEINLINE USoftBodyPhysicsComponent* GetSoftBody_ButtL() const { return SoftBody_ButtL; }
	FORCEINLINE USoftBodyPhysicsComponent* GetSoftBody_ButtR() const { return SoftBody_ButtR; }
};
