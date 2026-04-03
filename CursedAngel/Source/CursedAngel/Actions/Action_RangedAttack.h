// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ActionBase.h"
#include "Action_RangedAttack.generated.h"

// Forward declarations
class UCurseWeaponComponent;
enum class ECurseWeaponType : uint8;

/**
 * UAction_RangedAttack
 * 
 * Action that integrates with CurseWeaponComponent for ranged combat.
 * Optionally switches weapons before firing.
 * Montage is optional for throw animations (e.g., Rogue_throw1-4).
 */
UCLASS(Blueprintable)
class CURSEDANGEL_API UAction_RangedAttack : public UActionBase
{
	GENERATED_BODY()

public:
	/** The type of curse weapon to fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranged Attack")
	ECurseWeaponType WeaponType;

	/** If true, switch to WeaponType before firing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranged Attack", meta = (ToolTip = "Switch to this weapon before firing"))
	bool bSwitchWeapon;

public:
	UAction_RangedAttack();

	// UActionBase interface
	virtual void OnActivate() override;
	virtual void OnCancel() override;
};
