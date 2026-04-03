// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CursedAngelCharacter.h"
#include "Frank.generated.h"

// Forward declarations
class UHairPhysicsComponent;
class UCurseWeaponComponent;
class ACursedAngelCharacter;
class UAIBehaviorComponent;

/**
 * Frank's mode of operation
 */
UENUM(BlueprintType)
enum class EFrankMode : uint8
{
	Companion UMETA(DisplayName = "Companion"),
	Playable UMETA(DisplayName = "Playable")
};


/**
 * Secondary character (AI companion / playable)
 * Solo Dev Note: AI companion for prototype, playable segments in future
 *
 * COMPONENT OVERVIEW:
 * - FurPhysics: Hair physics component for fur simulation (ears, tail)
 *
 * FUTURE ENHANCEMENTS:
 * - Add UFrankAIComponent for companion AI behavior
 * - Add playable mode toggle (switch between Ripley and Frank)
 * - Add unique Frank abilities (curse-based powers)
 * - Add Frank-specific UI elements
 * - Add Frank progression system (separate from Ripley)
 *
 * DESIGN NOTES:
 * - Frank serves as AI companion in prototype
 * - Future: Playable in optional segments (similar to Ashley RE4)
 * - Future: Unique curse-based abilities (different from Ripley)
 * - Physics components work automatically (no special handling needed)
 */
UCLASS()
class CURSEDANGEL_API AFrank : public ACursedAngelCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AFrank();

	// Frank's current mode (Companion or Playable)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frank|Mode")
	EFrankMode CurrentMode = EFrankMode::Companion;

	/**
	 * Set Frank's mode (Companion or Playable)
	 * @param NewMode The new mode to set
	 */
	UFUNCTION(BlueprintCallable, Category = "Frank|Mode")
	void SetMode(EFrankMode NewMode);

	/**
	 * Follow the player character
	 * @param Player The player character to follow
	 */
	UFUNCTION(BlueprintCallable, Category = "Frank|AI")
	void FollowPlayer(ACursedAngelCharacter* Player);

	/**
	 * Assist in combat by attacking the target
	 * @param Target The target to attack
	 */
	UFUNCTION(BlueprintCallable, Category = "Frank|AI")
	void AssistInCombat(AActor* Target);

	// ===== AI Behavior System =====

	/** AI behavior component for managing companion AI behaviors */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UAIBehaviorComponent> AIBehaviorComponent;

	// Input handling (for future playable mode)
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Initialize from data asset (populate AI behaviors)
	virtual void InitializeFromDataAsset() override;
};