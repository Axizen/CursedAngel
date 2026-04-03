// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIBehaviorBase.h"
#include "AIBehavior_Combat.generated.h"

class UActionComponent;

/**
 * UAIBehavior_Combat
 * Handles AI combat logic: finding nearest enemy, moving to combat range, executing attack actions
 * Integrates with ActionComponent to execute attacks
 */
UCLASS(Blueprintable)
class CURSEDANGEL_API UAIBehavior_Combat : public UAIBehaviorBase
{
	GENERATED_BODY()

public:
	UAIBehavior_Combat();

	// Combat range to detect and engage enemies
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float CombatRange = 500.0f;

	// Name of the action to execute when attacking (e.g., "RangedAttack", "MeleeAttack")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FName AttackActionName = FName("RangedAttack");

	// Cooldown between attacks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackCooldown = 2.0f;

	// Current target actor
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	AActor* CurrentTarget = nullptr;

	// Override base behavior functions
	virtual void OnActivate() override;
	virtual void OnTick(float DeltaTime) override;
	virtual void OnDeactivate() override;
	virtual bool CanActivate() override;

protected:
	// Helper function to find nearest enemy within combat range
	AActor* FindNearestEnemy();

private:
	// Current cooldown timer
	float CurrentCooldown = 0.0f;
};
