// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CurrencyComponent.generated.h"

/**
 * Delegate for currency change events
 * Broadcasts when any currency value changes (DataFragments, CurseEssence, or CodeKeys)
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnCurrencyChanged, int32, DataFragments, int32, CurseEssence, int32, CodeKeys, FName, CurrencyType);

/**
 * Component that manages currency tracking for the player
 * Tracks three types of currency:
 * - Data Fragments: Primary currency, dropped by all enemies and destructibles
 * - Curse Essence: Rare currency, dropped by Elite/MiniBoss/Boss enemies
 * - Code Keys: Collectible currency, hidden in levels
 * Supports replication for multiplayer
 */
UCLASS(ClassGroup=(Currency), meta=(BlueprintSpawnableComponent))
class CURSEDANGEL_API UCurrencyComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCurrencyComponent();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:	
	// ========== Properties ==========

	/** Data Fragments - Primary currency dropped by all enemies and destructibles */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_DataFragments, Category = "Currency")
	int32 DataFragments;

	/** Curse Essence - Rare currency dropped by Elite/MiniBoss/Boss enemies */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_CurseEssence, Category = "Currency")
	int32 CurseEssence;

	/** Code Keys - Collectible currency hidden in levels */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_CodeKeys, Category = "Currency")
	int32 CodeKeys;

	// ========== Blueprint Events ==========

	/** Called when any currency value changes */
	UPROPERTY(BlueprintAssignable, Category = "Currency|Events")
	FOnCurrencyChanged OnCurrencyChanged;

	// ========== Functions ==========

	/**
	 * Add Data Fragments
	 * @param Amount Amount to add (negative values will be clamped to 0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	void AddDataFragments(int32 Amount);

	/**
	 * Add Curse Essence
	 * @param Amount Amount to add (negative values will be clamped to 0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	void AddCurseEssence(int32 Amount);

	/**
	 * Add Code Keys
	 * @param Amount Amount to add (negative values will be clamped to 0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	void AddCodeKeys(int32 Amount);

	/**
	 * Spend Data Fragments
	 * @param Amount Amount to spend
	 * @return True if transaction successful, false if insufficient funds
	 */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	bool SpendDataFragments(int32 Amount);

	/**
	 * Spend Curse Essence
	 * @param Amount Amount to spend
	 * @return True if transaction successful, false if insufficient funds
	 */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	bool SpendCurseEssence(int32 Amount);

	/**
	 * Spend Code Keys
	 * @param Amount Amount to spend
	 * @return True if transaction successful, false if insufficient funds
	 */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	bool SpendCodeKeys(int32 Amount);

	/**
	 * Get current Data Fragments
	 * @return Current Data Fragments amount
	 */
	UFUNCTION(BlueprintPure, Category = "Currency")
	int32 GetDataFragments() const;

	/**
	 * Get current Curse Essence
	 * @return Current Curse Essence amount
	 */
	UFUNCTION(BlueprintPure, Category = "Currency")
	int32 GetCurseEssence() const;

	/**
	 * Get current Code Keys
	 * @return Current Code Keys amount
	 */
	UFUNCTION(BlueprintPure, Category = "Currency")
	int32 GetCodeKeys() const;

private:
	// ========== Replication Functions ==========

	/** Called when DataFragments is replicated */
	UFUNCTION()
	void OnRep_DataFragments();

	/** Called when CurseEssence is replicated */
	UFUNCTION()
	void OnRep_CurseEssence();

	/** Called when CodeKeys is replicated */
	UFUNCTION()
	void OnRep_CodeKeys();
};
