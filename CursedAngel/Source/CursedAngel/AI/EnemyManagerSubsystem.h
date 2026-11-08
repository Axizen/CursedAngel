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
	DisplaceFromDestruction UMETA(DisplayName="Displace From Destroyed Props")
};

/** A data-driven response for one enemy class and one detected player habit. */
USTRUCT(BlueprintType)
struct CURSEDANGEL_API FEnemyReactionRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Reaction", meta=(ToolTip="Enemy strength class this rule applies to."))
	EEnemyType EnemyType = EEnemyType::Fodder;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Reaction", meta=(ToolTip="Minimum profile confidence required before this reaction can be selected.", ClampMin="0.0", ClampMax="1.0"))
	float MinimumConfidence = 0.25f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Reaction", meta=(ToolTip="Weapon preference that triggers this rule. Set bMatchWeaponPreference false for non-weapon reactions."))
	ECurseWeaponType PreferredPlayerWeapon = ECurseWeaponType::CorruptionRail;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Reaction", meta=(ToolTip="When true, rule requires the player to prefer PreferredPlayerWeapon."))
	bool bMatchWeaponPreference = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Reaction", meta=(ToolTip="When true, rule requires a high airborne combat ratio."))
	bool bMatchAirbornePreference = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Reaction", meta=(ToolTip="When true, rule requires repeated player-caused environmental destruction."))
	bool bMatchDestructionPreference = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Reaction", meta=(ToolTip="Tactic consumed by AEnemyAIController when this reaction wins."))
	EEnemyAdaptiveTactic Tactic = EEnemyAdaptiveTactic::Default;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Reaction", meta=(ToolTip="Optional attack-pattern name. Must correspond to an attack handled by this enemy's Blueprint or behavior tree."))
	FName AttackPattern = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Reaction", meta=(ToolTip="Scales the AI controller's preferred distance while using this tactic.", ClampMin="0.25", ClampMax="3.0"))
	float EngagementDistanceMultiplier = 1.0f;
};

/**
 * Level-scoped adaptive enemy manager. It evaluates accumulated player observations against
 * designer-authored reaction rules and applies the winning tactic/pattern to registered enemies.
 */
UCLASS(BlueprintType, Blueprintable)
class CURSEDANGEL_API UEnemyManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	UEnemyManagerSubsystem();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Manager|Reaction Table", meta=(ToolTip="Ordered rules. The first matching rule for an enemy type wins; put the most specific rules first."))
	TArray<FEnemyReactionRule> ReactionTable;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Manager|Tuning", meta=(ClampMin="0.05", ClampMax="5.0", ToolTip="Maximum time between refreshes of registered enemies after profile changes."))
	float ReactionRefreshInterval = 0.25f;

	UFUNCTION(BlueprintCallable, Category="Enemy Manager")
	void RegisterEnemy(AEnemyBase* Enemy);
	UFUNCTION(BlueprintCallable, Category="Enemy Manager")
	void UnregisterEnemy(AEnemyBase* Enemy);
	UFUNCTION(BlueprintCallable, Category="Enemy Manager")
	void RefreshEnemyReaction(AEnemyBase* Enemy);
	UFUNCTION(BlueprintCallable, Category="Enemy Manager")
	void RefreshAllEnemyReactions();
	UFUNCTION(BlueprintPure, Category="Enemy Manager|Reaction Table")
	bool FindReactionForEnemy(EEnemyType EnemyType, FEnemyReactionRule& OutRule) const;

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<AEnemyBase>> RegisteredEnemies;
	FTimerHandle RefreshTimerHandle;
	UFUNCTION()
	void HandleProfileChanged(const FAdaptivePlayerProfile& Profile);
	bool RuleMatches(const FEnemyReactionRule& Rule, const class UEnemyDirectorSubsystem* Director) const;
};
