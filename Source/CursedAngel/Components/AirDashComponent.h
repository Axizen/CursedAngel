// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AirDashComponent.generated.h"

class ACharacter;
class UCharacterMovementComponent;

/**
 * Component that handles air dash ability for the character
 * Allows multiple air dashes with cooldown management
 * 
 * @deprecated Use UAction_AirDash instead. This component will be removed in a future version.
 * 
 * Migration Path:
 * 1. Remove AirDashComponent from your character Blueprint
 * 2. Add ActionComponent to your character (or inherit from ACursedAngelCharacter which includes it)
 * 3. Create a UActionDataAsset for air dash:
 *    - Set ActionClass to UAction_AirDash
 *    - Configure DashSpeed, DashDuration, and MaxAirDashes properties
 *    - Set RequiredTags to only allow air dash when airborne
 * 4. Add the air dash action data asset to CharacterDataAsset.AvailableActions array
 * 5. Update input handling to call ActionComponent->ExecuteAction("AirDash") instead of AirDashComponent->PerformAirDash()
 * 6. Bind ResetAirDashes to character OnLanded event: Character->LandedDelegate.AddDynamic(ActionComponent, &UActionComponent::OnCharacterLanded)
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent, DeprecatedNode, DeprecationMessage="Use UAction_AirDash with ActionComponent instead. See class documentation for migration path."))
class CURSEDANGEL_API UAirDashComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAirDashComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Perform an air dash in the specified direction */
	UFUNCTION(BlueprintCallable, Category = "Air Dash")
	void PerformAirDash(FVector Direction);

	/** Check if air dash can be performed */
	UFUNCTION(BlueprintCallable, Category = "Air Dash")
	bool CanAirDash() const;

	/** Reset air dashes (called on landing) */
	UFUNCTION(BlueprintCallable, Category = "Air Dash")
	void ResetAirDashes();

	/** Enable or disable air dash ability */
	UFUNCTION(BlueprintCallable, Category = "Air Dash")
	void SetAirDashEnabled(bool bEnabled);

	/** Get current number of air dashes remaining */
	UFUNCTION(BlueprintCallable, Category = "Air Dash")
	int32 GetCurrentAirDashes() const { return CurrentAirDashes; }

	/** Get max air dashes */
	UFUNCTION(BlueprintCallable, Category = "Air Dash")
	int32 GetMaxAirDashes() const { return MaxAirDashes; }

	/** Check if currently dashing */
	UFUNCTION(BlueprintCallable, Category = "Air Dash")
	bool IsDashing() const { return bIsDashing; }

protected:
	/** Update dash state during active dash */
	void UpdateDash(float DeltaTime);

	/** Called when dash ends */
	void EndDash();

	/** Bind to character's landed event */
	UFUNCTION()
	void OnCharacterLanded(const FHitResult& Hit);

public:
	/** Speed of the air dash */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Air Dash")
	float AirDashSpeed;

	/** Duration of the air dash in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Air Dash")
	float AirDashDuration;

	/** Cooldown between air dashes in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Air Dash")
	float AirDashCooldown;

	/** Maximum number of air dashes before landing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Air Dash")
	int32 MaxAirDashes;

	/** Blueprint event fired when air dash starts */
	UFUNCTION(BlueprintImplementableEvent, Category = "Air Dash")
	void OnAirDashStart(FVector Direction);

	/** Blueprint event fired when air dash ends */
	UFUNCTION(BlueprintImplementableEvent, Category = "Air Dash")
	void OnAirDashEnd(FVector Direction);

protected:
	/** Current number of air dashes remaining */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Air Dash")
	int32 CurrentAirDashes;

	/** Time of last air dash */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Air Dash")
	float LastDashTime;

	/** Whether currently performing a dash */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Air Dash")
	bool bIsDashing;

	/** Current dash direction */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Air Dash")
	FVector DashDirection;

	/** Time remaining in current dash */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Air Dash")
	float DashTimeRemaining;

	/** Whether air dash is enabled */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Air Dash")
	bool bAirDashEnabled;

	/** Cached reference to owner character */
	UPROPERTY()
	ACharacter* OwnerCharacter;

	/** Cached reference to character movement component */
	UPROPERTY()
	UCharacterMovementComponent* MovementComponent;
};
