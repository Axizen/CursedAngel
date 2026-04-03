// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ActionBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Action_AirDash.generated.h"

/**
 * Air dash action - allows characters to dash in the air.
 * Refactored from AirDashComponent for action system integration.
 */
UCLASS(Blueprintable)
class CURSEDANGEL_API UAction_AirDash : public UActionBase
{
	GENERATED_BODY()

public:
	UAction_AirDash();

	// ===== Properties =====

	/** Speed of the air dash */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Air Dash")
	float DashSpeed;

	/** Duration of the air dash */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Air Dash")
	float DashDuration;

	/** Maximum number of air dashes allowed per jump */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Air Dash")
	int32 MaxAirDashes;

	/** Current number of air dashes used this jump */
	UPROPERTY(BlueprintReadOnly, Category = "Air Dash")
	int32 CurrentAirDashes;

	// ===== Overrides =====

	/** Check if the action can be activated (airborne and dashes remaining) */
	virtual bool CanActivate() const override;

	/** Called when the action is activated - performs the dash */
	virtual void OnActivate() override;

	/** Called every frame - tracks dash duration */
	virtual void OnTick(float DeltaTime) override;

	/** Called when the action is cancelled or completes */
	virtual void OnCancel() override;

	// ===== Functions =====

	/** Reset air dashes when character lands (call from OnLanded) */
	UFUNCTION(BlueprintCallable, Category = "Air Dash")
	void ResetAirDashes();

private:
	/** Elapsed time since dash started */
	float ElapsedTime;
};
