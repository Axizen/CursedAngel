// Copyright Cursed Angel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AIBehaviorComponent.generated.h"

// Forward declarations
class UAIBehaviorBase;

/**
 * UAIBehaviorComponent
 * Manages AI behaviors for NPCs.
 * Handles behavior execution, priority-based auto-selection, and behavior state tracking.
 * Mirrors ActionComponent architecture for consistency.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CURSEDANGEL_API UAIBehaviorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UAIBehaviorComponent();

	// ===== Properties =====

	/** Array of available behaviors for this AI (populated in Blueprint or CharacterDataAsset) */
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category = "AI Behavior")
	TArray<TObjectPtr<UAIBehaviorBase>> AvailableBehaviors;

	/** The currently executing behavior (null if none) */
	UPROPERTY(BlueprintReadOnly, Category = "AI Behavior")
	TObjectPtr<UAIBehaviorBase> CurrentBehavior;

	/** Automatically select highest priority valid behavior when no behavior is active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior", meta = (ToolTip = "Automatically select highest priority valid behavior"))
	bool bAutoSelectBehavior = true;

	// ===== Core Functions =====

	/**
	 * Execute a specific behavior by name.
	 * Checks CanActivate, calls OnActivate, and sets as CurrentBehavior.
	 * @param BehaviorName - Name of the behavior to execute
	 * @return true if behavior was successfully executed, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Behavior")
	bool ExecuteBehavior(FName BehaviorName);

	/**
	 * Stop the currently executing behavior.
	 * Calls OnDeactivate and clears CurrentBehavior.
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Behavior")
	void StopCurrentBehavior();

	/**
	 * Get a behavior by name.
	 * @param BehaviorName - Name of the behavior to find
	 * @return Behavior instance if found, null otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Behavior")
	UAIBehaviorBase* GetBehavior(FName BehaviorName) const;

	/**
	 * Select the highest priority valid behavior that CanActivate.
	 * @return Best valid behavior, or null if none can activate
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Behavior")
	UAIBehaviorBase* SelectBestBehavior();

	// ===== UActorComponent Overrides =====

	/** Called every frame to tick active behavior and perform auto-selection */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	/** Initialize behaviors with owner reference */
	void InitializeBehaviors();
};
