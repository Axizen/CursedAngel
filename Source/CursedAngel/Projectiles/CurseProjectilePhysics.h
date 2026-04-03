// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Projectiles/CurseProjectile.h"
#include "CurseProjectilePhysics.generated.h"

/**
 * Physics-based projectile for FragmentNeedles weapon
 * Uses ballistic trajectory with optional bouncing
 */
UCLASS()
class CURSEDANGEL_API ACurseProjectilePhysics : public ACurseProjectile
{
	GENERATED_BODY()
	
public:	
	ACurseProjectilePhysics();

	// ========== Properties ==========

	/** Gravity scale for ballistic trajectory (1.0 = normal gravity) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Projectile")
	float GravityScale;

	/** Bounciness of the projectile (0.0 = no bounce, 1.0 = perfect bounce) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Projectile")
	float Bounciness;

	/** Maximum number of bounces before destruction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Projectile")
	int32 MaxBounces;

	/** Current number of bounces performed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Projectile")
	int32 CurrentBounces;

	// ========== Functions ==========

	/**
	 * Override Initialize to configure physics properties
	 * @param Stats Weapon stats to configure this projectile
	 * @param InOwner Character that fired this projectile
	 */
	virtual void Initialize(const FCurseWeaponStats& Stats, ARipley* InOwner) override;

	/**
	 * Override OnHit to handle bounce logic
	 */
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

protected:
	/**
	 * Handle bounce logic
	 * @param Hit The hit result containing bounce information
	 * @return True if bounce was successful
	 */
	bool HandleBounce(const FHitResult& Hit);
};
