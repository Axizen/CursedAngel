// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CombatTypes.h"
#include "AI/EnemyManagerSubsystem.h"
#include "EnemyBase.generated.h"

class AEnemyAIController;
class UHealthComponent;
class ACursedAngelCharacter;
class UBehaviorTree;
class UBlackboardData;
class UEnemyDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemySpawned);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyDeath, AActor*, Killer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackPerformed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAdaptiveEnemyReaction, EEnemyAdaptiveTactic, Tactic, float, EngagementDistanceMultiplier, FName, AttackPattern);

/** Base class for enemy combatants; exposes adaptive tactic changes for C++ and Blueprint AI. */
UCLASS()
class CURSEDANGEL_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()
public:
	AEnemyBase();
	virtual void PostInitializeComponents() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
protected:
	virtual void BeginPlay() override;
public:
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components") UHealthComponent* HealthComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Stats") FEnemyStats Stats;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Configuration") UEnemyDataAsset* EnemyConfigAsset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Configuration") FName EnemyTypeName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Stats") EEnemyType EnemyType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|Behavior Tree") UBehaviorTree* BehaviorTreeAsset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|Behavior Tree") UBlackboardData* BlackboardAsset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|Behavior Tree") bool bUseBehaviorTree;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Stats|Currency") FCurrencyDropConfig CurrencyConfig;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Attack Patterns") TArray<FName> AttackPatterns;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Attack Patterns") FName CurrentAttackPattern;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI") bool bIsAggro;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI") float AggroRange;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat") float AttackDamage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat") float AttackCooldown;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat") float LastAttackTime;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI") ACursedAngelCharacter* TargetPlayer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Stats") float DeathDestroyDelay;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI|Adaptive", meta=(ToolTip="Current tactic selected by the level enemy manager.")) EEnemyAdaptiveTactic AdaptiveTactic = EEnemyAdaptiveTactic::Default;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI|Adaptive", meta=(ToolTip="Preferred AI distance multiplier selected by the enemy reaction table.")) float AdaptiveEngagementDistanceMultiplier = 1.0f;

	UPROPERTY(BlueprintAssignable, Category="Enemy|Events") FOnEnemySpawned OnEnemySpawned;
	UPROPERTY(BlueprintAssignable, Category="Enemy|Events") FOnEnemyDeath OnEnemyDeath;
	UPROPERTY(BlueprintAssignable, Category="Enemy|Events") FOnAttackPerformed OnAttackPerformed;
	UPROPERTY(BlueprintAssignable, Category="Enemy|Adaptive|Events", meta=(ToolTip="Fired when the reaction table changes this enemy's tactic or attack pattern.")) FOnAdaptiveEnemyReaction OnAdaptiveReactionApplied;

	UFUNCTION(BlueprintCallable, Category="Enemy") void Initialize(const FEnemyStats& InStats);
	UFUNCTION(BlueprintCallable, Category="Enemy") void LoadStatsFromDataAsset();
	UFUNCTION(BlueprintCallable, Category="Enemy") void PerformAttack();
	UFUNCTION(BlueprintCallable, Category="Enemy") bool FindPlayerTarget();
	UFUNCTION(BlueprintPure, Category="Enemy") float GetDistanceToPlayer() const;
	UFUNCTION(BlueprintPure, Category="Enemy") bool CanAttack() const;
	UFUNCTION(BlueprintCallable, Category="Enemy|Currency") void SpawnCurrencyDrops();
	UFUNCTION(BlueprintCallable, Category="Enemy|Combat") void SelectAttackPattern();
	UFUNCTION(BlueprintPure, Category="Enemy|Combat") FName GetCurrentAttackPattern() const;
	UFUNCTION(BlueprintCallable, Category="Enemy|Adaptive", meta=(ToolTip="Called by UEnemyManagerSubsystem. Blueprint AI can respond to the selected tactic and attack pattern."))
	void ApplyAdaptiveReaction(EEnemyAdaptiveTactic NewTactic, float EngagementDistanceMultiplier, FName AttackPattern);
protected:
	UFUNCTION() void OnDeath(AActor* Killer, EDamageType Type);
	void HandleDeath(AActor* Killer);
	FName ConfiguredAttackPattern;
};
