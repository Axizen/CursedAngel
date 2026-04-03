// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Projectiles/CurseProjectile.h"
#include "CurseProjectileMine.generated.h"

/**
 * Proximity mine projectile for VoidMines weapon
 * Places a mine that arms after a delay and detonates when enemies are nearby
 */
UCLASS()
class CURSEDANGEL_API ACurseProjectileMine : public ACurseProjectile
{
	GENERATED_BODY()
	
public:	
	ACurseProjectileMine();

protected:
	virtual void BeginPlay() override;

public:
	// ========== Components ==========

	/** Trigger radius component for proximity detection */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* TriggerComponent;

	// ========== Properties ==========

	/** Time before mine becomes armed and active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mine")
	float ArmingTime;

	/** Radius for proximity detection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mine")
	float TriggerRadius;

	/** Radius of explosion damage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mine")
	float ExplosionRadius;

	/** Damage dealt by explosion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mine")
	float ExplosionDamage;

	/** Whether the mine is armed and ready to detonate */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mine")
	bool bIsArmed;

	/** Whether the mine has detonated */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mine")
	bool bHasDetonated;

	// ========== Blueprint Events ==========

	/** Called when mine is armed */
	UFUNCTION(BlueprintImplementableEvent, Category = "Mine|Events")
	void OnMineArmed();

	/** Called when mine detonates */
	UFUNCTION(BlueprintImplementableEvent, Category = "Mine|Events")
	void OnMineDetonated(const TArray<AActor*>& DamagedActors);

	// ========== Functions ==========

	/**
	 * Override Initialize to configure mine properties
	 * @param Stats Weapon stats to configure this projectile
	 * @param InOwner Character that fired this projectile
	 */
	virtual void Initialize(const FCurseWeaponStats& Stats, ARipley* InOwner) override;

	/**
	 * Override OnHit to place mine on surface
	 */
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

protected:
	/** Timer handle for arming delay */
	FTimerHandle ArmingTimerHandle;

	/**
	 * Called when mine becomes armed
	 */
	UFUNCTION()
	void ArmMine();

	/**
	 * Called when something enters the trigger radius
	 */
	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/**
	 * Detonate the mine
	 */
	UFUNCTION()
	void Detonate();

	/**
	 * Apply radial damage to all actors in explosion radius
	 */
	void ApplyExplosionDamage();
};
