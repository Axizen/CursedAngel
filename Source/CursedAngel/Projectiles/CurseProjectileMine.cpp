// Copyright Epic Games, Inc. All Rights Reserved.

#include "Projectiles/CurseProjectileMine.h"
#include "Characters/Ripley.h"
#include "Components/HealthComponent.h"
#include "ReactiveArenaElement.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

ACurseProjectileMine::ACurseProjectileMine()
{
	// Disable projectile movement (mine is stationary once placed)
	if (ProjectileMovement)
	{
		ProjectileMovement->SetActive(true);
		ProjectileMovement->ProjectileGravityScale = 1.0f; // Use gravity to fall and stick
		ProjectileMovement->bShouldBounce = false;
	}

	// Create trigger component for proximity detection
	TriggerComponent = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerComponent"));
	TriggerComponent->SetupAttachment(RootComponent);
	TriggerComponent->InitSphereRadius(200.0f);
	TriggerComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerComponent->SetGenerateOverlapEvents(false); // Will enable after arming

	// Default values
	ArmingTime = 0.5f;
	TriggerRadius = 200.0f;
	ExplosionRadius = 400.0f;
	ExplosionDamage = 50.0f;
	bIsArmed = false;
	bHasDetonated = false;
	WeaponType = ECurseWeaponType::VoidMines;
	
	// Mines have long lifetime (until triggered)
	MaxLifetime = 30.0f;
	InitialLifeSpan = MaxLifetime;
}

void ACurseProjectileMine::BeginPlay()
{
	Super::BeginPlay();

	// Bind trigger overlap event
	TriggerComponent->OnComponentBeginOverlap.AddDynamic(this, &ACurseProjectileMine::OnTriggerBeginOverlap);
}

void ACurseProjectileMine::Initialize(const FCurseWeaponStats& Stats, ARipley* InOwner)
{
	// Call parent initialize
	Super::Initialize(Stats, InOwner);

	// Configure trigger radius
	if (TriggerComponent)
	{
		TriggerComponent->SetSphereRadius(TriggerRadius);
	}

	// Explosion damage is based on weapon damage
	ExplosionDamage = Stats.Damage;
}

void ACurseProjectileMine::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Ignore hits with owner
	if (OtherActor == OwnerCharacter || OtherActor == this)
	{
		return;
	}

	// Mine has landed, disable movement and start arming
	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->SetActive(false);
	}

	// Disable collision on main component (we only care about trigger now)
	if (CollisionComponent)
	{
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Fire Blueprint event
	OnProjectileHit(OtherActor, Hit.ImpactPoint);

	// Check for reactive elements at placement location
	CheckReactiveElement(OtherActor);

	// Start arming timer
	GetWorldTimerManager().SetTimer(ArmingTimerHandle, this, &ACurseProjectileMine::ArmMine, ArmingTime, false);
}

void ACurseProjectileMine::ArmMine()
{
	if (bHasDetonated)
	{
		return;
	}

	bIsArmed = true;

	// Enable trigger overlap detection
	if (TriggerComponent)
	{
		TriggerComponent->SetGenerateOverlapEvents(true);
	}

	// Fire Blueprint event for VFX (armed state)
	OnMineArmed();
}

void ACurseProjectileMine::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Only trigger if armed and not already detonated
	if (!bIsArmed || bHasDetonated)
	{
		return;
	}

	// Ignore owner
	if (OtherActor == OwnerCharacter || OtherActor == this)
	{
		return;
	}

	// Check if the overlapping actor has a health component (is a valid target)
	UHealthComponent* HealthComp = OtherActor->FindComponentByClass<UHealthComponent>();
	if (HealthComp && HealthComp->IsAlive())
	{
		// Valid target detected, detonate
		Detonate();
	}
}

void ACurseProjectileMine::Detonate()
{
	if (bHasDetonated)
	{
		return;
	}

	bHasDetonated = true;

	// Apply explosion damage
	ApplyExplosionDamage();

	// Destroy the mine after a short delay to allow VFX
	FTimerHandle DetonateTimerHandle;
	GetWorldTimerManager().SetTimer(DetonateTimerHandle, this, &ACurseProjectileMine::DestroyProjectile, 0.1f, false);
}

void ACurseProjectileMine::ApplyExplosionDamage()
{
	if (!GetWorld())
	{
		return;
	}

	// Get all actors in explosion radius
	TArray<FHitResult> HitResults;
	FVector ExplosionLocation = GetActorLocation();
	
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(ExplosionRadius);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(OwnerCharacter);

	GetWorld()->SweepMultiByChannel(
		HitResults,
		ExplosionLocation,
		ExplosionLocation,
		FQuat::Identity,
		ECC_Pawn,
		SphereShape,
		QueryParams
	);

	// Track damaged actors for Blueprint event
	TArray<AActor*> DamagedActors;

	// Apply damage to all hit actors
	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && HitActor != OwnerCharacter)
		{
			// Calculate distance-based damage falloff
			float Distance = FVector::Dist(ExplosionLocation, HitActor->GetActorLocation());
			float DamageFalloff = 1.0f - (Distance / ExplosionRadius);
			DamageFalloff = FMath::Clamp(DamageFalloff, 0.0f, 1.0f);
			float FinalDamage = ExplosionDamage * DamageFalloff;

			// Apply damage
			UHealthComponent* HealthComp = HitActor->FindComponentByClass<UHealthComponent>();
			if (HealthComp && HealthComp->IsAlive())
			{
				HealthComp->TakeDamage(FinalDamage, DamageType, OwnerCharacter);
				DamagedActors.Add(HitActor);
			}

			// Check for reactive elements
			CheckReactiveElement(HitActor);
		}
	}

	// Fire Blueprint event with all damaged actors
	OnMineDetonated(DamagedActors);

	// Draw debug sphere if in editor
#if WITH_EDITOR
	DrawDebugSphere(
		GetWorld(),
		ExplosionLocation,
		ExplosionRadius,
		32,
		FColor::Orange,
		false,
		2.0f,
		0,
		2.0f
	);
#endif
}
