// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIBehaviorBase.h"
#include "AIBehavior_Follow.generated.h"

/**
 * AI Behavior for following a target (typically the player).
 * Handles companion follow logic with distance-based activation and optional teleporting.
 */
UCLASS(Blueprintable)
class CURSEDANGEL_API UAIBehavior_Follow : public UAIBehaviorBase
{
	GENERATED_BODY()

public:
	UAIBehavior_Follow();

	// Minimum distance before AI starts following
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow")
	float FollowDistance;

	// Maximum distance before teleporting to target
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow", meta = (Tooltip = "Teleport if further than this"))
	float MaxFollowDistance;

	// Whether to teleport if too far from target
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow")
	bool bTeleportIfTooFar;

protected:
	// UAIBehaviorBase interface
	virtual void OnActivate() override;
	virtual void OnTick(float DeltaTime) override;
	virtual void OnDeactivate() override;
	virtual bool CanActivate() override;

private:
	// Cached player reference
	UPROPERTY()
	AActor* TargetActor;

	// Helper to get distance to player
	float GetDistanceToTarget() const;
};
