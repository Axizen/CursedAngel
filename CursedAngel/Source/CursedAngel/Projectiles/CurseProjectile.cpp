// Copyright Epic Games, Inc. All Rights Reserved.

#include "Projectiles/CurseProjectile.h"
#include "Characters/Ripley.h"
#include "Components/HealthComponent.h"
#include "ReactiveArenaElement.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"

ACurseProjectile::ACurseProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create collision component
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(15.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	RootComponent = CollisionComponent;

	// Create mesh component
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(CollisionComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Create projectile movement component (optional, can be disabled in derived classes)
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 3000.0f;
	ProjectileMovement->MaxSpeed = 3000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.0f;

	// Default values
	Damage = 25.0f;
	ProjectileSpeed = 3000.0f;
	MaxLifetime = 10.0f;
	bPenetrates = false;
	PenetrationCount = 1;
	CurrentPenetrations = 0;
	WeaponType = ECurseWeaponType::CorruptionRail;
	DamageType = EDamageType::Curse;
	OwnerCharacter = nullptr;

	// Set initial lifespan
	InitialLifeSpan = MaxLifetime;
}

void ACurseProjectile::BeginPlay()
{
	Super::BeginPlay();

	// Bind hit event
	CollisionComponent->OnComponentHit.AddDynamic(this, &ACurseProjectile::OnHit);

	// Set lifetime timer
	if (MaxLifetime > 0.0f)
	{
		GetWorldTimerManager().SetTimer(LifetimeTimerHandle, this, &ACurseProjectile::DestroyProjectile, MaxLifetime, false);
	}

	// Fire Blueprint event
	OnProjectileSpawned();
}

void ACurseProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACurseProjectile::Initialize(const FCurseWeaponStats& Stats, ARipley* InOwner)
{
	Damage = Stats.Damage;
	ProjectileSpeed = Stats.ProjectileSpeed;
	bPenetrates = Stats.PenetrationCount > 1;
	PenetrationCount = Stats.PenetrationCount;
	OwnerCharacter = InOwner;

	// Update projectile movement speed if component exists
	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = ProjectileSpeed;
		ProjectileMovement->MaxSpeed = ProjectileSpeed;
	}
}

void ACurseProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Ignore hits with owner
	if (OtherActor == OwnerCharacter || OtherActor == this)
	{
		return;
	}

	// Check if we've already hit this actor (for penetration)
	if (HasHitActor(OtherActor))
	{
		return;
	}

	// Fire Blueprint event
	OnProjectileHit(OtherActor, Hit.ImpactPoint);

	// Try to apply damage
	bool bDamageApplied = ApplyDamageToTarget(OtherActor);

	// Check for reactive elements
	CheckReactiveElement(OtherActor);

	// Handle penetration logic
	if (bDamageApplied)
	{
		HitActors.Add(OtherActor);
		CurrentPenetrations++;

		// Destroy if we've exceeded penetration count or can't penetrate
		if (!bPenetrates || CurrentPenetrations >= PenetrationCount)
		{
			DestroyProjectile();
		}
	}
	else
	{
		// Hit something we can't damage (wall, etc.), destroy projectile
		DestroyProjectile();
	}
}

bool ACurseProjectile::ApplyDamageToTarget(AActor* Target)
{
	if (!Target)
	{
		return false;
	}

	// Find health component on target
	UHealthComponent* HealthComp = Target->FindComponentByClass<UHealthComponent>();
	if (HealthComp && HealthComp->IsAlive())
	{
		// Apply damage
		HealthComp->TakeDamage(Damage, DamageType, OwnerCharacter);
		return true;
	}

	return false;
}

bool ACurseProjectile::CheckReactiveElement(AActor* Target)
{
	if (!Target)
	{
		return false;
	}

	// Check if target is a reactive arena element
	AReactiveArenaElement* ReactiveElement = Cast<AReactiveArenaElement>(Target);
	if (ReactiveElement && ReactiveElement->CanReact(WeaponType))
	{
		// Trigger the reaction
		ReactiveElement->TriggerReaction(WeaponType, GetActorLocation());
		return true;
	}

	return false;
}

void ACurseProjectile::DestroyProjectile()
{
	// Fire Blueprint event
	OnProjectileDestroyed();

	// Clear timer
	if (LifetimeTimerHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(LifetimeTimerHandle);
	}

	// Destroy actor
	Destroy();
}

bool ACurseProjectile::HasHitActor(AActor* Actor) const
{
	return HitActors.Contains(Actor);
}
