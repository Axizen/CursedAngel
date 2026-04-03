// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "CombatTypes.h"
#include "Components/CurseWeaponComponent.h"
#include "CurseProjectile.generated.h"

class ARipley;
class UHealthComponent;
class AReactiveArenaElement;

/**
 * Base projectile class for curse weapons
 * Handles hit detection, damage application, penetration, and reactive element triggering
 */
UCLASS(Abstract, Blueprintable)
class CURSEDANGEL_API ACurseProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	ACurseProjectile();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	// ========== Components ==========

	/** Collision component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* CollisionComponent;

	/** Projectile movement component (optional, used for physics-based projectiles) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProjectileMovementComponent* ProjectileMovement;

	/** Visual mesh component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	// ========== Properties ==========

	/** Damage dealt by this projectile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float Damage;

	/** Speed of the projectile (if using movement component) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float ProjectileSpeed;

	/** Maximum lifetime before auto-destruction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float MaxLifetime;

	/** Whether this projectile can penetrate targets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	bool bPenetrates;

	/** Maximum number of targets this projectile can penetrate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	int32 PenetrationCount;

	/** Current number of penetrations performed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	int32 CurrentPenetrations;

	/** Type of curse weapon that spawned this projectile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	ECurseWeaponType WeaponType;

	/** Type of damage dealt by this projectile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	EDamageType DamageType;

	/** Character that owns this projectile */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	ARipley* OwnerCharacter;

	// ========== Functions ==========

	/**
	 * Initialize the projectile with weapon stats and owner
	 * @param Stats Weapon stats to configure this projectile
	 * @param Owner Character that fired this projectile
	 */
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	virtual void Initialize(const FCurseWeaponStats& Stats, ARipley* InOwner);

	/**
	 * Called when projectile hits something
	 * @param HitComp Component that was hit
	 * @param OtherActor Actor that was hit
	 * @param OtherComp Component on the other actor that was hit
	 * @param NormalImpulse Normal impulse from the hit
	 * @param Hit Hit result information
	 */
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/**
	 * Apply damage to a target actor
	 * @param Target Actor to damage
	 * @return True if damage was successfully applied
	 */
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	virtual bool ApplyDamageToTarget(AActor* Target);

	/**
	 * Check if target is a reactive element and trigger it
	 * @param Target Actor to check
	 * @return True if target was a reactive element and was triggered
	 */
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	virtual bool CheckReactiveElement(AActor* Target);

	// ========== Blueprint Events ==========

	/** Called when projectile is spawned */
	UFUNCTION(BlueprintImplementableEvent, Category = "Projectile|Events")
	void OnProjectileSpawned();

	/** Called when projectile hits an actor */
	UFUNCTION(BlueprintImplementableEvent, Category = "Projectile|Events")
	void OnProjectileHit(AActor* HitActor, FVector HitLocation);

	/** Called when projectile is destroyed */
	UFUNCTION(BlueprintImplementableEvent, Category = "Projectile|Events")
	void OnProjectileDestroyed();

protected:
	/** Timer handle for lifetime management */
	FTimerHandle LifetimeTimerHandle;

	/** Actors that have already been hit (for penetration tracking) */
	UPROPERTY()
	TArray<AActor*> HitActors;

	/** Destroy the projectile after lifetime expires */
	void DestroyProjectile();

	/** Check if an actor has already been hit */
	bool HasHitActor(AActor* Actor) const;
};
