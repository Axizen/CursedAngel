// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CursedAngelCharacter.h"
#include "Ripley.generated.h"

// Forward declarations
class UAirDashComponent;
// TODO: Create these components in future steps
// class UDataManagerComponent;
// class UWeaponManagerComponent;
// class UBlessedModManagerComponent;
class UHairPhysicsComponent;
class UClothPhysicsComponent;
class USoftBodyPhysicsComponent;

/**
 * ARipley - Ranged combat specialist character
 * 
 * Inherits from ACursedAngelCharacter to gain:
 * - Camera system
 * - Input system
 * - Combat components (CurseWeaponComponent, CursedAngelComponent, StyleComponent)
 * - Base combat functions
 * 
 * Adds Ripley-specific features:
 * - Air dash mobility
 * - Weapon management system
 * - Blessed mod system
 * - Advanced physics (hair, cloth, soft body)
 * - Ranged combat focus (70% ranged, 30% melee)
 */
UCLASS(config=Game)
class ARipley : public ACursedAngelCharacter
{
	GENERATED_BODY()

	/** Air Dash Component - Ripley's signature mobility ability */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UAirDashComponent* AirDashComponent;

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
	float OriginalJumpZVelocity;
	float OriginalAirControl;
	float OriginalCameraFOV;

public:
	/** Ripley-specific component getters */
	FORCEINLINE class UAirDashComponent* GetAirDashComponent() const { return AirDashComponent; }
	// TODO: Add getters for these components in future steps
	// FORCEINLINE class UDataManagerComponent* GetDataManagerComponent() const { return DataManagerComponent; }
	// FORCEINLINE class UWeaponManagerComponent* GetWeaponManagerComponent() const { return WeaponManagerComponent; }
	// FORCEINLINE class UBlessedModManagerComponent* GetBlessedModManagerComponent() const { return BlessedModManagerComponent; }
	FORCEINLINE class UHairPhysicsComponent* GetHairPhysicsComponent() const { return HairPhysicsComponent; }
	FORCEINLINE class UClothPhysicsComponent* GetClothPhysicsComponent() const { return ClothPhysicsComponent; }
	FORCEINLINE class USoftBodyPhysicsComponent* GetSoftBodyPhysicsComponent() const { return SoftBodyPhysicsComponent; }
};
