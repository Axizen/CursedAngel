// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/EnemyBase.h"
#include "Components/HealthComponent.h"
#include "Characters/CursedAngelCharacter.h"
#include "Components/CursedAngelComponent.h"
#include "Components/StyleComponent.h"
#include "AI/EnemyAIController.h"
#include "Data/EnemyDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create health component
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	// Default values
	bIsAggro = false;
	AggroRange = 1500.0f;
	AttackDamage = 10.0f;
	AttackCooldown = 2.0f;
	LastAttackTime = -999.0f;
	TargetPlayer = nullptr;
	DeathDestroyDelay = 3.0f;

	// Behavior Tree defaults
	BehaviorTreeAsset = nullptr;
	BlackboardAsset = nullptr;
	bUseBehaviorTree = false;

	// Enemy type default
	EnemyType = EEnemyType::Fodder;

	// Currency drop defaults
	CurrencyConfig.MinDataFragments = 5;
	CurrencyConfig.MaxDataFragments = 10;
	CurrencyConfig.MinCurseEssence = 0;
	CurrencyConfig.MaxCurseEssence = 0;
	CurrencyConfig.DropChance = 1.0f;

	// Default stats
	Stats.MaxHealth = 100.0f;
	Stats.Damage = 10.0f;
	Stats.MoveSpeed = 400.0f;
	Stats.AttackRange = 200.0f;
	Stats.AttackCooldown = 2.0f;
	Stats.StyleValue = 10.0f;

	// Set AI controller class
	AIControllerClass = AEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AEnemyBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Ensure AI controller is set
	if (!AIControllerClass)
	{
		AIControllerClass = AEnemyAIController::StaticClass();
	}
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	// Bind to health component death event
	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddDynamic(this, &AEnemyBase::OnDeath);
	}

	// Load stats from data asset if configured
	LoadStatsFromDataAsset();

	// Find player target
	FindPlayerTarget();

	// Broadcast spawned event
	OnEnemySpawned.Broadcast();
}

void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update aggro state based on distance to player
	if (TargetPlayer && HealthComponent && HealthComponent->IsAlive())
	{
		float Distance = GetDistanceToPlayer();
		if (Distance >= 0.0f && Distance <= AggroRange)
		{
			bIsAggro = true;
		}
	}
}

void AEnemyBase::Initialize(const FEnemyStats& InStats)
{
	Stats = InStats;

	// Apply stats to components
	if (HealthComponent)
	{
		HealthComponent->MaxHealth = Stats.MaxHealth;
		HealthComponent->CurrentHealth = Stats.MaxHealth;
	}

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = Stats.MoveSpeed;
	}

	AttackDamage = Stats.Damage;
	AttackCooldown = Stats.AttackCooldown;

	// Apply enemy type and currency drops from stats
	EnemyType = Stats.EnemyType;

	// Configure currency drops based on enemy type
	switch (EnemyType)
	{
	case EEnemyType::Fodder:
		// Fodder enemies drop only Data Fragments
		CurrencyConfig.MinDataFragments = Stats.DataFragmentDrop;
		CurrencyConfig.MaxDataFragments = Stats.DataFragmentDrop + 5;
		CurrencyConfig.MinCurseEssence = 0;
		CurrencyConfig.MaxCurseEssence = 0;
		break;
	case EEnemyType::Elite:
		// Elite enemies drop more Data and some Curse Essence
		CurrencyConfig.MinDataFragments = Stats.DataFragmentDrop;
		CurrencyConfig.MaxDataFragments = Stats.DataFragmentDrop + 10;
		CurrencyConfig.MinCurseEssence = FMath::Max(1, Stats.CurseEssenceDrop);
		CurrencyConfig.MaxCurseEssence = Stats.CurseEssenceDrop + 3;
		break;
	case EEnemyType::MiniBoss:
		// Mini bosses drop significant amounts of both currencies
		CurrencyConfig.MinDataFragments = Stats.DataFragmentDrop;
		CurrencyConfig.MaxDataFragments = Stats.DataFragmentDrop + 20;
		CurrencyConfig.MinCurseEssence = FMath::Max(3, Stats.CurseEssenceDrop);
		CurrencyConfig.MaxCurseEssence = Stats.CurseEssenceDrop + 5;
		break;
	case EEnemyType::Boss:
		// Bosses drop large amounts of both currencies
		CurrencyConfig.MinDataFragments = Stats.DataFragmentDrop;
		CurrencyConfig.MaxDataFragments = Stats.DataFragmentDrop + 50;
		CurrencyConfig.MinCurseEssence = FMath::Max(5, Stats.CurseEssenceDrop);
		CurrencyConfig.MaxCurseEssence = Stats.CurseEssenceDrop + 10;
		break;
	}

	// Copy attack patterns from stats
	AttackPatterns = Stats.AttackPatterns;

	// Select initial attack pattern if available
	if (AttackPatterns.Num() > 0)
	{
		SelectAttackPattern();
	}
}

void AEnemyBase::PerformAttack()
{
	if (!CanAttack() || !TargetPlayer)
	{
		return;
	}

	// Check if player is in range
	float Distance = GetDistanceToPlayer();
	if (Distance < 0.0f || Distance > Stats.AttackRange)
	{
		return;
	}

	// Update last attack time
	LastAttackTime = GetWorld()->GetTimeSeconds();

	// Apply damage to player if they have a health component
	UHealthComponent* PlayerHealth = TargetPlayer->FindComponentByClass<UHealthComponent>();
	if (PlayerHealth)
	{
		PlayerHealth->TakeDamage(AttackDamage, EDamageType::Physical, this);
	}

	// Broadcast attack event for Blueprint VFX/animations
	OnAttackPerformed.Broadcast();
}

bool AEnemyBase::FindPlayerTarget()
{
	if (TargetPlayer)
	{
		return true;
	}

	// Find player character in world
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (PlayerPawn)
	{
		TargetPlayer = Cast<ACursedAngelCharacter>(PlayerPawn);
		return TargetPlayer != nullptr;
	}

	return false;
}

float AEnemyBase::GetDistanceToPlayer() const
{
	if (!TargetPlayer)
	{
		return -1.0f;
	}

	return FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation());
}

bool AEnemyBase::CanAttack() const
{
	if (!HealthComponent || !HealthComponent->IsAlive())
	{
		return false;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();
	return (CurrentTime - LastAttackTime) >= AttackCooldown;
}

void AEnemyBase::OnDeath(AActor* Killer, EDamageType DamageType)
{
	HandleDeath(Killer);
}

void AEnemyBase::HandleDeath(AActor* Killer)
{
	// Disable collision
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Disable movement
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DisableMovement();
	}

	// Disable AI
	if (GetController())
	{
		GetController()->UnPossess();
	}

	// Notify player's style component
	if (TargetPlayer)
	{
		UStyleComponent* StyleComp = TargetPlayer->GetStyleComponent();
		if (StyleComp)
		{
			// Determine weapon type from killer (if it's a projectile, it should have weapon info)
			ECurseWeaponType WeaponUsed = TargetPlayer->GetCurrentCurseWeapon();
			// Environmental kill detection would need to be passed from damage source
			bool bEnvironmentalKill = false;
			
			StyleComp->OnEnemyKilled(this, WeaponUsed, bEnvironmentalKill);
		}

		// Award data points to player's cursed angel component
		UCursedAngelComponent* CursedAngelComp = TargetPlayer->GetCursedAngelComponent();
		if (CursedAngelComp)
		{
			// Award data points based on style value
			CursedAngelComp->AddDataPoints(Stats.StyleValue);
		}
	}

	// Broadcast death event for Blueprint VFX/animations
	OnEnemyDeath.Broadcast(Killer);

	// Spawn currency drops before destruction
	SpawnCurrencyDrops();

	// Destroy after delay
	FTimerHandle DestroyTimerHandle;
	FTimerDelegate DestroyDelegate;
	DestroyDelegate.BindLambda([this]()
	{
		Destroy();
	});
	GetWorld()->GetTimerManager().SetTimer(
		DestroyTimerHandle,
		DestroyDelegate,
		DeathDestroyDelay,
		false
	);
}

void AEnemyBase::SpawnCurrencyDrops()
{
	// Check drop chance first
	if (FMath::FRand() > CurrencyConfig.DropChance)
	{
		return;
	}

	// Calculate random Data Fragment amount
	int32 DataFragmentsToDrop = FMath::RandRange(CurrencyConfig.MinDataFragments, CurrencyConfig.MaxDataFragments);

	// Calculate random Curse Essence amount (only for Elite/MiniBoss/Boss)
	int32 CurseEssenceToDrop = 0;
	if (EnemyType != EEnemyType::Fodder)
	{
		CurseEssenceToDrop = FMath::RandRange(CurrencyConfig.MinCurseEssence, CurrencyConfig.MaxCurseEssence);
	}

	// TODO: Spawn currency pickup actors or add directly to player's CurrencyComponent
	// For now, log the drops for debugging
	UE_LOG(LogTemp, Log, TEXT("%s dropped %d Data Fragments and %d Curse Essence"), *GetName(), DataFragmentsToDrop, CurseEssenceToDrop);

	// Once CurrencyComponent is created, implement auto-collection:
	// Find player within collection radius and add currency directly to their CurrencyComponent
	// Or spawn pickup actors if auto-collection is disabled
}

void AEnemyBase::SelectAttackPattern()
{
	if (AttackPatterns.Num() == 0)
	{
		CurrentAttackPattern = NAME_None;
		return;
	}

	// Randomly select an attack pattern
	int32 RandomIndex = FMath::RandRange(0, AttackPatterns.Num() - 1);
	CurrentAttackPattern = AttackPatterns[RandomIndex];

	UE_LOG(LogTemp, Log, TEXT("%s selected attack pattern: %s"), *GetName(), *CurrentAttackPattern.ToString());
}

FName AEnemyBase::GetCurrentAttackPattern() const
{
	return CurrentAttackPattern;
}

void AEnemyBase::LoadStatsFromDataAsset()
{
	// Check if data asset and type name are configured
	if (!EnemyConfigAsset || EnemyTypeName.IsNone())
	{
		return;
	}

	// Get stats from data asset
	FEnemyStats LoadedStats = EnemyConfigAsset->GetEnemyStats(EnemyTypeName);
	
	// Check if stats were found (MaxHealth is a required field, so check if it's valid)
	if (LoadedStats.MaxHealth > 0.0f)
	{
		// Initialize with loaded stats
		Initialize(LoadedStats);
		
		UE_LOG(LogTemp, Log, TEXT("%s successfully loaded stats from data asset: Type=%s, Health=%.0f"), 
			*GetName(), *EnemyTypeName.ToString(), LoadedStats.MaxHealth);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s failed to load stats from data asset: Type=%s not found"), 
			*GetName(), *EnemyTypeName.ToString());
	}
}
