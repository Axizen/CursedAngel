// Copyright Cursed Angel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "ActionComponent.generated.h"

// Forward declarations
class UActionDataAsset;
class UActionBase;

/**
 * UActionComponent
 * Manages and executes actions for characters.
 * Handles action execution, queueing for combo chains, cooldowns, and gameplay tag state tracking.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CURSEDANGEL_API UActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UActionComponent();

	// ===== Properties =====

	/** Array of action data assets available to this character */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actions")
	TArray<TObjectPtr<UActionDataAsset>> AvailableActions;

	/** The currently executing action (null if none) */
	UPROPERTY(BlueprintReadOnly, Category = "Actions")
	TObjectPtr<UActionBase> CurrentAction;

	/** Queue of actions to execute after current action completes (for combo chains) */
	UPROPERTY(BlueprintReadOnly, Category = "Actions")
	TArray<FName> ActionQueue;

	/** Active gameplay tags representing character state (Attacking, Dodging, Airborne, etc.) */
	UPROPERTY(BlueprintReadOnly, Category = "Actions")
	FGameplayTagContainer ActiveTags;

	/** Map of action names to remaining cooldown times */
	UPROPERTY(BlueprintReadOnly, Category = "Actions")
	TMap<FName, float> ActionCooldowns;

	// ===== Core Functions =====

	/**
	 * Execute an action by name.
	 * Checks cooldown, tags, instantiates action, and activates it.
	 * @param ActionName - Name of the action to execute
	 * @return true if action was successfully executed, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "Actions")
	bool ExecuteAction(FName ActionName);

	/**
	 * Queue an action to execute after the current action completes.
	 * Used for combo chains where next action is buffered during current action.
	 * @param ActionName - Name of the action to queue
	 */
	UFUNCTION(BlueprintCallable, Category = "Actions")
	void QueueAction(FName ActionName);

	/**
	 * Cancel the currently executing action.
	 * Calls OnCancel, clears CurrentAction, and removes granted tags.
	 */
	UFUNCTION(BlueprintCallable, Category = "Actions")
	void CancelCurrentAction();

	/**
	 * Get an action data asset by name.
	 * @param ActionName - Name of the action to find
	 * @return Action data asset if found, null otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "Actions")
	UActionDataAsset* GetAction(FName ActionName) const;

	/**
	 * Check if an action can be executed based on tags and cooldown.
	 * @param ActionData - Action data asset to check
	 * @return true if action can be executed, false otherwise
	 */
	bool CanExecuteAction(const UActionDataAsset* ActionData) const;

	// ===== Tag Management =====

	/**
	 * Add a gameplay tag to the active tags container.
	 * @param Tag - Tag to add
	 */
	UFUNCTION(BlueprintCallable, Category = "Actions|Tags")
	void AddTag(FGameplayTag Tag);

	/**
	 * Remove a gameplay tag from the active tags container.
	 * @param Tag - Tag to remove
	 */
	UFUNCTION(BlueprintCallable, Category = "Actions|Tags")
	void RemoveTag(FGameplayTag Tag);

	/**
	 * Check if the active tags container has a specific tag.
	 * @param Tag - Tag to check
	 * @return true if tag is present, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "Actions|Tags")
	bool HasTag(FGameplayTag Tag) const;

	// ===== UActorComponent Overrides =====

	/** Called every frame to update cooldowns and tick active action */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	/** Process the action queue when current action ends */
	void ProcessActionQueue();

	/** Clear the current action and remove granted tags */
	void ClearCurrentAction();
};
