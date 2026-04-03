// Copyright Epic Games, Inc. All Rights Reserved.

#include "Projectiles/CurseProjectileHitscan.h"
#include "Characters/Ripley.h"
#include "Components/HealthComponent.h"
#include "ReactiveArenaElement.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ACurseProjectileHitscan::ACurseProjectileHitscan()
{
	// Hitscan doesn't need movement component
	if (ProjectileMovement)
	{
		ProjectileMovement->SetActive(false);
	}

	// Hitscan doesn't need collision (we use line traces)
	if (CollisionComponent)
	{
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Default values
	MaxRange = 5000.0f;
	bDrawDebugLine = false;
	WeaponType = ECurseWeaponType::CorruptionRail;
	
	// Hitscan projectiles are instant, so very short lifetime
	MaxLifetime = 0.1f;
	InitialLifeSpan = MaxLifetime;
}

void ACurseProjectileHitscan::Initialize(const FCurseWeaponStats& Stats, ARipley* InOwner)
{
	// Call parent initialize
	Super::Initialize(Stats, InOwner);

	// Update max range from stats
	MaxRange = Stats.MaxRange;

	// Perform the hitscan immediately
	PerformHitscan();

	// Destroy after a short delay to allow VFX to spawn
	FTimerHandle DestroyTimerHandle;
	GetWorldTimerManager().SetTimer(DestroyTimerHandle, this, &ACurseProjectileHitscan::DestroyProjectile, 0.05f, false);
}

void ACurseProjectileHitscan::PerformHitscan()
{
	if (!OwnerCharacter)
	{
		return;
	}

	// Get start and end points for the trace
	FVector StartLocation = GetActorLocation();
	FVector ForwardVector = GetActorForwardVector();
	FVector EndLocation = StartLocation + (ForwardVector * MaxRange);

	// Setup collision query parameters
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(OwnerCharacter);
	QueryParams.bTraceComplex = false;
	QueryParams.bReturnPhysicalMaterial = false;

	// Perform multi-line trace for penetration
	TArray<FHitResult> HitResults;
	GetWorld()->LineTraceMultiByChannel(
		HitResults,
		StartLocation,
		EndLocation,
		ECC_Visibility,
		QueryParams
	);

	// Draw debug line if enabled
	if (bDrawDebugLine)
	{
		DrawDebugLine(
			GetWorld(),
			StartLocation,
			EndLocation,
			FColor::Red,
			false,
			2.0f,
			0,
			2.0f
		);
	}

	// Process all hits
	int32 HitsProcessed = 0;
	for (const FHitResult& Hit : HitResults)
	{
		if (Hit.bBlockingHit)
		{
			// Fire Blueprint event for VFX
			OnProjectileHit(Hit.GetActor(), Hit.ImpactPoint);

			// Process the hit
			bool bContinue = ProcessHit(Hit);
			HitsProcessed++;

			// Draw debug point if enabled
			if (bDrawDebugLine)
			{
				DrawDebugSphere(
					GetWorld(),
					Hit.ImpactPoint,
					10.0f,
					12,
					FColor::Yellow,
					false,
					2.0f
				);
			}

			// Check if we should stop (exceeded penetration count or hit something we can't penetrate)
			if (!bContinue || HitsProcessed >= PenetrationCount)
			{
				break;
			}
		}
	}
}

bool ACurseProjectileHitscan::ProcessHit(const FHitResult& Hit)
{
	AActor* HitActor = Hit.GetActor();
	if (!HitActor || HitActor == OwnerCharacter)
	{
		return false;
	}

	// Check if we've already hit this actor
	if (HasHitActor(HitActor))
	{
		return true; // Continue tracing but skip this actor
	}

	// Try to apply damage
	bool bDamageApplied = ApplyDamageToTarget(HitActor);

	// Check for reactive elements
	CheckReactiveElement(HitActor);

	// Track this hit
	if (bDamageApplied)
	{
		HitActors.Add(HitActor);
		CurrentPenetrations++;
	}

	// Continue if we can penetrate and haven't hit max penetrations
	return bPenetrates && CurrentPenetrations < PenetrationCount;
}
