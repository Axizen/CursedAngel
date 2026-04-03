// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actions/ActionBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Action_Dodge.generated.h"

/**
 * UAction_Dodge
 * Handles dodge rolls and evasion mechanics with optional invulnerability frames.
 * Duration-based action with automatic end after DodgeDuration.
 */
UCLASS(Blueprintable)
class CURSEDANGEL_API UAction_Dodge : public UActionBase
{
	GENERATED_BODY()

public:
	/** Direction to dodge in (relative to character) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge")
	FVector DodgeDirection;

	/** Speed of the dodge movement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge")
	float DodgeSpeed;

	/** Duration of the dodge in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge")
	float DodgeDuration;

	/** Grant invulnerability during dodge (i-frames) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge", meta = (Tooltip = "Grant invulnerability during dodge"))
	bool bInvulnerable;

	UAction_Dodge();

	// UActionBase interface
	virtual void OnActivate() override;
	virtual void OnTick(float DeltaTime) override;
	virtual void OnCancel() override;

private:
	/** Tracks elapsed time during dodge */
	float ElapsedTime;

	/** Cached original collision settings for restoration */
	ECollisionEnabled::Type OriginalCollisionType;

	/** Whether we modified collision settings */
	bool bModifiedCollision;
};
