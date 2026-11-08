// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CombatTypes.h"
#include "EnemyManagerSubsystem.generated.h"

class AEnemyBase;

UENUM(BlueprintType)
enum class EEnemyAdaptiveTactic : uint8
{
	Default UMETA(DisplayName="Default Combat"),
	RushPlayer UMETA(DisplayName="Rush Player"),
	MaintainRange UMETA(DisplayName="Maintain Range"),
	StrafePlayer UMETA(DisplayName="Strafe Player"),
	PunishAirborne UMETA(DisplayName="Punish Airborne Player"),
	DisplaceFromDestruction UMETA(DisplayName="Displace From Destroyed Props"),
	AntiAirLine UMETA(DisplayName="Anti-Air Firing Line"),
	FlankGuard UMETA(DisplayName="Momentum-Aware Flank Guard")
};

USTRUCT(BlueprintType)
struct CURSEDANGEL_API FEnemyReactionRule
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Reaction", meta=(ToolTip="Enemy strength class this rule applies to.")) EEnemyType EnemyType = EEnemyType::Fodder;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Reaction", meta=(ClampMin="0.0", ClampMax="1.0", ToolTip="Minimum player-profile confidence required before this reaction can be selected.")) float MinimumConfidence = 0.25f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Reaction") ECurseWeaponType PreferredPlayerWeapon = ECurseWeaponType::CorruptionRail;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Reaction") bool bMatchWeaponPreference = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Reaction") bool bMatchAirbornePreference = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Reaction") bool bMatchDestructionPreference = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Reaction") EEnemyAdaptiveTactic Tactic = EEnemyAdaptiveTactic::Default;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Reaction", meta=(ToolTip="Optional pattern name consumed by this enemy's Blueprint or Behavior Tree.")) FName AttackPattern = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Reaction", meta=(ClampMin="0.25", ClampMax="3.0")) float EngagementDistanceMultiplier = 1.0f;
};

/** Level coordinator: batches group positioning, reaction selection and crowd-impact propagation. */
UCLASS(BlueprintType, Blueprintable)
class CURSEDANGEL_API UEnemyManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	UEnemyManagerSubsystem();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Manager|Reaction Table", meta=(ToolTip="Ordered rules. First matching rule for an enemy class wins; place specific counters before broad rules.")) TArray<FEnemyReactionRule> ReactionTable;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Manager|Tuning", meta=(ClampMin="0.05", ClampMax="5.0", ToolTip="Seconds between batched enemy formation/position updates.")) float CoordinationInterval = 0.2f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Manager|Formations", meta=(ClampMin="100.0", ToolTip="Spacing between ground grunts in an anti-air firing line.")) float AntiAirLineSpacing = 180.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Manager|Formations", meta=(ClampMin="100.0", ToolTip="Distance in front of the player at which anti-air grunts form their firing line.")) float AntiAirLineDistance = 350.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Manager|Formations", meta=(ClampMin="100.0", ToolTip="Radius used to position rear guards against player dashes and flanks.")) float FlankGuardRadius = 450.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Manager|Crowd Physics", meta=(ClampMin="100.0", ToolTip="Maximum radius of a tether/brute impact domino knockback.")) float DefaultDominoRadius = 600.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Manager|Crowd Physics", meta=(ClampMin="0.0", ToolTip="Per-target outward velocity impulse at the center of the impact.")) float DefaultDominoImpulse = 1200.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Manager|Crowd Physics", meta=(ClampMin="0.0", ToolTip="Upward velocity impulse added to enemies hit by a domino impact.")) float DominoUpwardImpulse = 500.0f;

	UFUNCTION(BlueprintCallable, Category="Enemy Manager") void RegisterEnemy(AEnemyBase* Enemy);
	UFUNCTION(BlueprintCallable, Category="Enemy Manager") void UnregisterEnemy(AEnemyBase* Enemy);
	UFUNCTION(BlueprintCallable, Category="Enemy Manager") void RefreshEnemyReaction(AEnemyBase* Enemy);
	UFUNCTION(BlueprintCallable, Category="Enemy Manager") void RefreshAllEnemyReactions();
	UFUNCTION(BlueprintCallable, Category="Enemy Manager|Coordination") void UpdateCoordinatedPositions();
	/** Call from gravity tether/heavy collision gameplay. Applies radial, distance-falloff launches to nearby registered enemies. */
	UFUNCTION(BlueprintCallable, Category="Enemy Manager|Crowd Physics", meta=(ToolTip="Propagates a physical knockback through nearby enemies. Use the impact origin and strength from a tethered heavy enemy or explosion."))
	int32 ApplyDominoKnockback(AActor* ImpactSource, FVector ImpactOrigin, float Radius, float ImpulseStrength, float UpwardImpulse);
	UFUNCTION(BlueprintPure, Category="Enemy Manager|Reaction Table") bool FindReactionForEnemy(EEnemyType EnemyType, FEnemyReactionRule& OutRule) const;

private:
	UPROPERTY(Transient) TArray<TObjectPtr<AEnemyBase>> RegisteredEnemies;
	FTimerHandle CoordinationTimerHandle;
	FVector PreviousPlayerLocation = FVector::ZeroVector;
	FVector PlayerVelocity = FVector::ZeroVector;
	bool bHasPlayerLocation = false;
	UFUNCTION() void HandleProfileChanged(const FAdaptivePlayerProfile& Profile);
	bool RuleMatches(const FEnemyReactionRule& Rule, const class UEnemyDirectorSubsystem* Director) const;
};
