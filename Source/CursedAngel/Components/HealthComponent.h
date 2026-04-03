// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatTypes.h"
#include "HealthComponent.generated.h"

/**
 * Delegate for health change events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, NewHealth, float, MaxHealth);

/**
 * Delegate for death events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeath, AActor*, Killer, EDamageType, DamageType);

/**
 * Delegate for damage taken events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageTaken, float, Damage, EDamageType, DamageType);

/**
 * Component that manages health, damage, and death for actors
 * Supports invulnerability frames and different damage types
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class CURSEDANGEL_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHealthComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ========== Properties ==========

	/** Maximum health value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth;

	/** Current health value */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	float CurrentHealth;

	/** Whether this actor is dead */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	bool bIsDead;

	/** Whether this actor is currently invulnerable */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	bool bInvulnerable;

	/** Duration of invulnerability after taking damage (0 = no invulnerability frames) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float InvulnerabilityDuration;

	// ========== Blueprint Events ==========

	/** Called when health changes */
	UPROPERTY(BlueprintAssignable, Category = "Health|Events")
	FOnHealthChanged OnHealthChanged;

	/** Called when actor dies */
	UPROPERTY(BlueprintAssignable, Category = "Health|Events")
	FOnDeath OnDeath;

	/** Called when actor takes damage */
	UPROPERTY(BlueprintAssignable, Category = "Health|Events")
	FOnDamageTaken OnDamageTaken;

	// ========== Functions ==========

	/**
	 * Apply damage to this actor
	 * @param Damage Amount of damage to apply
	 * @param Type Type of damage being applied
	 * @param DamageDealer Actor that dealt the damage
	 * @return Actual damage dealt (may be 0 if invulnerable or dead)
	 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	float TakeDamage(float Damage, EDamageType Type, AActor* DamageDealer);

	/**
	 * Heal this actor
	 * @param Amount Amount of health to restore
	 * @return Actual amount healed
	 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	float Heal(float Amount);

	/**
	 * Instantly kill this actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void Kill();

	/**
	 * Check if this actor is alive
	 * @return True if alive, false if dead
	 */
	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsAlive() const;

	/**
	 * Get current health percentage (0.0 to 1.0)
	 */
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealthPercent() const;

	/**
	 * Set invulnerability state
	 * @param bNewInvulnerable New invulnerability state
	 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetInvulnerable(bool bNewInvulnerable);

private:
	/** Timer handle for invulnerability duration */
	FTimerHandle InvulnerabilityTimerHandle;

	/** Last actor that dealt damage (for death attribution) */
	UPROPERTY()
	AActor* LastDamageDealer;

	/** Last damage type received (for death attribution) */
	EDamageType LastDamageType;

	/** End invulnerability state */
	void EndInvulnerability();
};
