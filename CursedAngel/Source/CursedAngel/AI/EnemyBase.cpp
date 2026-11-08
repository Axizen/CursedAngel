// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/EnemyBase.h"
#include "AI/EnemyAIController.h"
#include "AI/EnemyManagerSubsystem.h"
#include "Components/HealthComponent.h"
#include "Characters/CursedAngelCharacter.h"
#include "Components/CursedAngelComponent.h"
#include "Components/StyleComponent.h"
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
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	bIsAggro = false; AggroRange = 1500.0f; AttackDamage = 10.0f; AttackCooldown = 2.0f; LastAttackTime = -999.0f; TargetPlayer = nullptr; DeathDestroyDelay = 3.0f;
	BehaviorTreeAsset = nullptr; BlackboardAsset = nullptr; bUseBehaviorTree = false; EnemyType = EEnemyType::Fodder;
	CurrencyConfig.MinDataFragments = 5; CurrencyConfig.MaxDataFragments = 10; CurrencyConfig.MinCurseEssence = 0; CurrencyConfig.MaxCurseEssence = 0; CurrencyConfig.DropChance = 1.0f;
	Stats.MaxHealth = 100.0f; Stats.Damage = 10.0f; Stats.MoveSpeed = 400.0f; Stats.AttackRange = 200.0f; Stats.AttackCooldown = 2.0f; Stats.StyleValue = 10.0f;
	AIControllerClass = AEnemyAIController::StaticClass(); AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AEnemyBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (!AIControllerClass) AIControllerClass = AEnemyAIController::StaticClass();
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	if (HealthComponent) HealthComponent->OnDeath.AddDynamic(this, &AEnemyBase::OnDeath);
	LoadStatsFromDataAsset();
	FindPlayerTarget();
	if (UEnemyManagerSubsystem* Manager = GetWorld()->GetSubsystem<UEnemyManagerSubsystem>()) Manager->RegisterEnemy(this);
	OnEnemySpawned.Broadcast();
}

void AEnemyBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld()) if (UEnemyManagerSubsystem* Manager = GetWorld()->GetSubsystem<UEnemyManagerSubsystem>()) Manager->UnregisterEnemy(this);
	Super::EndPlay(EndPlayReason);
}

void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (TargetPlayer && HealthComponent && HealthComponent->IsAlive())
	{
		const float Distance = GetDistanceToPlayer();
		if (Distance >= 0.0f && Distance <= AggroRange) bIsAggro = true;
	}
}

void AEnemyBase::Initialize(const FEnemyStats& InStats)
{
	Stats = InStats;
	if (HealthComponent) { HealthComponent->MaxHealth = Stats.MaxHealth; HealthComponent->CurrentHealth = Stats.MaxHealth; }
	if (GetCharacterMovement()) GetCharacterMovement()->MaxWalkSpeed = Stats.MoveSpeed;
	AttackDamage = Stats.Damage; AttackCooldown = Stats.AttackCooldown; EnemyType = Stats.EnemyType;
	switch (EnemyType)
	{
	case EEnemyType::Fodder: CurrencyConfig.MinDataFragments = Stats.DataFragmentDrop; CurrencyConfig.MaxDataFragments = Stats.DataFragmentDrop + 5; CurrencyConfig.MinCurseEssence = 0; CurrencyConfig.MaxCurseEssence = 0; break;
	case EEnemyType::Elite: CurrencyConfig.MinDataFragments = Stats.DataFragmentDrop; CurrencyConfig.MaxDataFragments = Stats.DataFragmentDrop + 10; CurrencyConfig.MinCurseEssence = FMath::Max(1, Stats.CurseEssenceDrop); CurrencyConfig.MaxCurseEssence = Stats.CurseEssenceDrop + 3; break;
	case EEnemyType::MiniBoss: CurrencyConfig.MinDataFragments = Stats.DataFragmentDrop; CurrencyConfig.MaxDataFragments = Stats.DataFragmentDrop + 20; CurrencyConfig.MinCurseEssence = FMath::Max(3, Stats.CurseEssenceDrop); CurrencyConfig.MaxCurseEssence = Stats.CurseEssenceDrop + 5; break;
	case EEnemyType::Boss: CurrencyConfig.MinDataFragments = Stats.DataFragmentDrop; CurrencyConfig.MaxDataFragments = Stats.DataFragmentDrop + 50; CurrencyConfig.MinCurseEssence = FMath::Max(5, Stats.CurseEssenceDrop); CurrencyConfig.MaxCurseEssence = Stats.CurseEssenceDrop + 10; break;
	}
	AttackPatterns = Stats.AttackPatterns;
	ConfiguredAttackPattern = AttackPatterns.Num() > 0 ? AttackPatterns[FMath::RandRange(0, AttackPatterns.Num() - 1)] : NAME_None;
	CurrentAttackPattern = ConfiguredAttackPattern;
}

void AEnemyBase::PerformAttack()
{
	if (!CanAttack() || !TargetPlayer) return;
	const float Distance = GetDistanceToPlayer();
	if (Distance < 0.0f || Distance > Stats.AttackRange) return;
	LastAttackTime = GetWorld()->GetTimeSeconds();
	if (UHealthComponent* PlayerHealth = TargetPlayer->FindComponentByClass<UHealthComponent>()) PlayerHealth->TakeDamage(AttackDamage, EDamageType::Physical, this);
	OnAttackPerformed.Broadcast();
}

bool AEnemyBase::FindPlayerTarget()
{
	if (TargetPlayer) return true;
	if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0)) { TargetPlayer = Cast<ACursedAngelCharacter>(PlayerPawn); return TargetPlayer != nullptr; }
	return false;
}

float AEnemyBase::GetDistanceToPlayer() const { return TargetPlayer ? FVector::Dist(GetActorLocation(), TargetPlayer->GetActorLocation()) : -1.0f; }
bool AEnemyBase::CanAttack() const { return HealthComponent && HealthComponent->IsAlive() && GetWorld() && GetWorld()->GetTimeSeconds() - LastAttackTime >= AttackCooldown; }
void AEnemyBase::OnDeath(AActor* Killer, EDamageType DamageType) { HandleDeath(Killer); }

void AEnemyBase::HandleDeath(AActor* Killer)
{
	if (GetCapsuleComponent()) GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (GetCharacterMovement()) GetCharacterMovement()->DisableMovement();
	if (GetController()) GetController()->UnPossess();
	if (TargetPlayer)
	{
		if (UStyleComponent* StyleComp = TargetPlayer->GetStyleComponent())
		{
			const ECurseWeaponType WeaponUsed = TargetPlayer->GetCurrentCurseWeapon();
			const bool bEnvironmentalKill = Killer && Killer != TargetPlayer && !Killer->IsA(AEnemyBase::StaticClass());
			StyleComp->OnEnemyKilled(this, WeaponUsed, bEnvironmentalKill);
		}
		if (UCursedAngelComponent* CursedAngelComp = TargetPlayer->GetCursedAngelComponent()) CursedAngelComp->AddDataPoints(Stats.StyleValue);
	}
	OnEnemyDeath.Broadcast(Killer);
	SpawnCurrencyDrops();
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AEnemyBase::K2_DestroyActor, DeathDestroyDelay, false);
}

void AEnemyBase::SpawnCurrencyDrops()
{
	if (FMath::FRand() > CurrencyConfig.DropChance) return;
	const int32 Data = FMath::RandRange(CurrencyConfig.MinDataFragments, CurrencyConfig.MaxDataFragments);
	const int32 Essence = EnemyType == EEnemyType::Fodder ? 0 : FMath::RandRange(CurrencyConfig.MinCurseEssence, CurrencyConfig.MaxCurseEssence);
	UE_LOG(LogTemp, Log, TEXT("%s dropped %d Data Fragments and %d Curse Essence"), *GetName(), Data, Essence);
}

void AEnemyBase::SelectAttackPattern()
{
	if (AttackPatterns.Num() == 0) { CurrentAttackPattern = NAME_None; return; }
	ConfiguredAttackPattern = AttackPatterns[FMath::RandRange(0, AttackPatterns.Num() - 1)];
	CurrentAttackPattern = ConfiguredAttackPattern;
}

FName AEnemyBase::GetCurrentAttackPattern() const { return CurrentAttackPattern; }

void AEnemyBase::ApplyAdaptiveReaction(EEnemyAdaptiveTactic NewTactic, float EngagementDistanceMultiplier, FName AttackPattern)
{
	const bool bChanged = AdaptiveTactic != NewTactic || !FMath::IsNearlyEqual(AdaptiveEngagementDistanceMultiplier, EngagementDistanceMultiplier) || (AttackPattern != NAME_None && CurrentAttackPattern != AttackPattern);
	AdaptiveTactic = NewTactic;
	AdaptiveEngagementDistanceMultiplier = FMath::Clamp(EngagementDistanceMultiplier, 0.25f, 3.0f);
	CurrentAttackPattern = AttackPattern.IsNone() ? ConfiguredAttackPattern : AttackPattern;
	if (bChanged) OnAdaptiveReactionApplied.Broadcast(AdaptiveTactic, AdaptiveEngagementDistanceMultiplier, CurrentAttackPattern);
}

void AEnemyBase::LoadStatsFromDataAsset()
{
	if (!EnemyConfigAsset || EnemyTypeName.IsNone()) return;
	const FEnemyStats LoadedStats = EnemyConfigAsset->GetEnemyStats(EnemyTypeName);
	if (LoadedStats.MaxHealth > 0.0f) Initialize(LoadedStats);
	else UE_LOG(LogTemp, Warning, TEXT("%s failed to load enemy config: %s"), *GetName(), *EnemyTypeName.ToString());
}
