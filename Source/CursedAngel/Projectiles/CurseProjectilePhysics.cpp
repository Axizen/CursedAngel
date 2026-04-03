// Copyright Epic Games, Inc. All Rights Reserved.

#include "Projectiles/CurseProjectilePhysics.h"
#include "Characters/Ripley.h"
#include "Components/HealthComponent.h"
#include "ReactiveArenaElement.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"

ACurseProjectilePhysics::ACurseProjectilePhysics()
{
	// Enable projectile movement for physics simulation
	if (ProjectileMovement)
	{
		ProjectileMovement->SetActive(true);
		ProjectileMovement->ProjectileGravityScale = 0.3f; // Slight gravity for arc
		ProjectileMovement->bShouldBounce = true;
		ProjectileMovement->Bounciness = 0.3f;
	}

	// Default values
	GravityScale = 0.3f;
	Bounciness = 0.3f;
	MaxBounces = 2;
	CurrentBounces = 0;
	WeaponType = ECurseWeaponType::FragmentNeedles;
	
	// Physics projectiles have medium lifetime
	MaxLifetime = 5.0f;
	InitialLifeSpan = MaxLifetime;
}

void ACurseProjectilePhysics::Initialize(const FCurseWeaponStats& Stats, ARipley* InOwner)
{
	// Call parent initialize
	Super::Initialize(Stats, InOwner);

	// Configure projectile movement component
	if (ProjectileMovement)
	{
		ProjectileMovement->ProjectileGravityScale = GravityScale;
		ProjectileMovement->Bounciness = Bounciness;
		ProjectileMovement->bShouldBounce = (MaxBounces > 0);
	}
}

void ACurseProjectilePhysics::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Ignore hits with owner
	if (OtherActor == OwnerCharacter || OtherActor == this)
	{
		return;
	}

	// Check if we've already hit this actor (for penetration)
	if (HasHitActor(OtherActor))
	{
		// Try to bounce if we hit the same actor again
		if (CurrentBounces < MaxBounces)
		{
			HandleBounce(Hit);
		}
		return;
	}

	// Fire Blueprint event
	OnProjectileHit(OtherActor, Hit.ImpactPoint);

	// Try to apply damage
	bool bDamageApplied = ApplyDamageToTarget(OtherActor);

	// Check for reactive elements
	CheckReactiveElement(OtherActor);

	// Handle hit logic
	if (bDamageApplied)
	{
		HitActors.Add(OtherActor);
		CurrentPenetrations++;

		// Check if we should bounce or destroy
		if (CurrentBounces < MaxBounces && bPenetrates)
		{
			// Try to bounce
			if (!HandleBounce(Hit))
			{
				// Bounce failed, destroy
				DestroyProjectile();
			}
		}
		else
		{
			// No more bounces or can't penetrate, destroy
			DestroyProjectile();
		}
	}
	else
	{
		// Hit something we can't damage (wall, etc.)
		if (CurrentBounces < MaxBounces)
		{
			// Try to bounce off walls
			if (!HandleBounce(Hit))
			{
				DestroyProjectile();
			}
		}
		else
		{
			// No more bounces, destroy
			DestroyProjectile();
		}
	}
}

bool ACurseProjectilePhysics::HandleBounce(const FHitResult& Hit)
{
	if (!ProjectileMovement)
	{
		return false;
	}

	// Increment bounce count
	CurrentBounces++;

	// Check if we've exceeded max bounces
	if (CurrentBounces >= MaxBounces)
	{
		return false;
	}

	// The ProjectileMovementComponent handles the actual bounce physics
	// We just need to track the bounce count and potentially modify velocity
	
	// Reduce velocity slightly with each bounce
	FVector CurrentVelocity = ProjectileMovement->Velocity;
	ProjectileMovement->Velocity = CurrentVelocity * Bounciness;

	return true;
}
