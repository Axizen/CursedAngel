// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ActionBase.h"
#include "Action_MeleeAttack.generated.h"

/**
 * Melee attack action with combo chain support
 * Integrates with StyleComponent for DMC-style rank system
 * Uses sphere trace for generous hitbox detection
 */
UCLASS(Blueprintable)
class CURSEDANGEL_API UAction_MeleeAttack : public UActionBase
{
	GENERATED_BODY()

public:
	UAction_MeleeAttack();

	// Damage dealt per hit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee Attack")
	float Damage = 30.0f;

	// Style points awarded per hit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee Attack")
	int32 StylePoints = 10;

	// Distance to trace in front of character
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee Attack")
	float TraceDistance = 200.0f;

	// Radius of sphere trace for hit detection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee Attack")
	float TraceRadius = 50.0f;

	// Override base action functions
	virtual void OnActivate() override;
	virtual void OnTick(float DeltaTime) override;
	virtual void OnCancel() override;

protected:
	// Perform sphere trace to find hit actors
	TArray<AActor*> PerformMeleeTrace();

private:
	// Track hit actors to prevent double-hits in same attack
	TArray<AActor*> HitActorsThisAttack;
	
	// Track if damage has been applied this attack (triggered by AnimNotify)
	bool bDamageApplied;
};
