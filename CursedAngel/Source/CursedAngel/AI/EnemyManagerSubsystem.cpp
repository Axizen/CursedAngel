// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/EnemyManagerSubsystem.h"
#include "AI/EnemyBase.h"
#include "AI/EnemyDirectorSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

UEnemyManagerSubsystem::UEnemyManagerSubsystem()
{
	// Default reaction table: weaker enemies use simple, readable counters; elites and bosses
	// use deliberate positioning and counter patterns. Designers can replace/tune these rules.
	FEnemyReactionRule Rule;
	Rule.EnemyType = EEnemyType::Fodder; Rule.bMatchWeaponPreference = true; Rule.PreferredPlayerWeapon = ECurseWeaponType::CorruptionRail;
	Rule.Tactic = EEnemyAdaptiveTactic::RushPlayer; Rule.AttackPattern = TEXT("RushCounter"); Rule.EngagementDistanceMultiplier = 0.6f; ReactionTable.Add(Rule);
	Rule = FEnemyReactionRule(); Rule.EnemyType = EEnemyType::Fodder; Rule.bMatchAirbornePreference = true;
	Rule.Tactic = EEnemyAdaptiveTactic::PunishAirborne; Rule.AttackPattern = TEXT("AntiAir"); Rule.EngagementDistanceMultiplier = 0.9f; ReactionTable.Add(Rule);
	Rule = FEnemyReactionRule(); Rule.EnemyType = EEnemyType::Elite; Rule.bMatchWeaponPreference = true; Rule.PreferredPlayerWeapon = ECurseWeaponType::CorruptionRail;
	Rule.Tactic = EEnemyAdaptiveTactic::StrafePlayer; Rule.AttackPattern = TEXT("FlankRanged"); Rule.EngagementDistanceMultiplier = 1.4f; ReactionTable.Add(Rule);
	Rule = FEnemyReactionRule(); Rule.EnemyType = EEnemyType::Elite; Rule.bMatchAirbornePreference = true;
	Rule.Tactic = EEnemyAdaptiveTactic::PunishAirborne; Rule.AttackPattern = TEXT("AntiAirBurst"); Rule.EngagementDistanceMultiplier = 1.1f; ReactionTable.Add(Rule);
	Rule = FEnemyReactionRule(); Rule.EnemyType = EEnemyType::MiniBoss; Rule.bMatchDestructionPreference = true;
	Rule.Tactic = EEnemyAdaptiveTactic::DisplaceFromDestruction; Rule.AttackPattern = TEXT("AreaDenial"); Rule.EngagementDistanceMultiplier = 1.2f; ReactionTable.Add(Rule);
	Rule = FEnemyReactionRule(); Rule.EnemyType = EEnemyType::Boss; Rule.bMatchAirbornePreference = true;
	Rule.Tactic = EEnemyAdaptiveTactic::PunishAirborne; Rule.AttackPattern = TEXT("ArenaAntiAir"); Rule.EngagementDistanceMultiplier = 1.0f; ReactionTable.Add(Rule);
}

void UEnemyManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	if (UEnemyDirectorSubsystem* Director = GetWorld()->GetSubsystem<UEnemyDirectorSubsystem>())
	{
		Director->OnProfileChanged.AddDynamic(this, &UEnemyManagerSubsystem::HandleProfileChanged);
	}
	if (GetWorld()) GetWorld()->GetTimerManager().SetTimer(RefreshTimerHandle, this, &UEnemyManagerSubsystem::RefreshAllEnemyReactions, FMath::Max(0.05f, ReactionRefreshInterval), true);
}

void UEnemyManagerSubsystem::Deinitialize()
{
	if (GetWorld()) GetWorld()->GetTimerManager().ClearTimer(RefreshTimerHandle);
	if (UEnemyDirectorSubsystem* Director = GetWorld() ? GetWorld()->GetSubsystem<UEnemyDirectorSubsystem>() : nullptr)
	{
		Director->OnProfileChanged.RemoveDynamic(this, &UEnemyManagerSubsystem::HandleProfileChanged);
	}
	RegisteredEnemies.Empty();
	Super::Deinitialize();
}

void UEnemyManagerSubsystem::RegisterEnemy(AEnemyBase* Enemy)
{
	if (!IsValid(Enemy)) return;
	RegisteredEnemies.AddUnique(Enemy);
	RefreshEnemyReaction(Enemy);
}

void UEnemyManagerSubsystem::UnregisterEnemy(AEnemyBase* Enemy)
{
	RegisteredEnemies.Remove(Enemy);
}

void UEnemyManagerSubsystem::RefreshEnemyReaction(AEnemyBase* Enemy)
{
	if (!IsValid(Enemy)) return;
	FEnemyReactionRule Rule;
	if (FindReactionForEnemy(Enemy->EnemyType, Rule))
	{
		Enemy->ApplyAdaptiveReaction(Rule.Tactic, Rule.EngagementDistanceMultiplier, Rule.AttackPattern);
	}
	else
	{
		Enemy->ApplyAdaptiveReaction(EEnemyAdaptiveTactic::Default, 1.0f, NAME_None);
	}
}

void UEnemyManagerSubsystem::RefreshAllEnemyReactions()
{
	for (int32 Index = RegisteredEnemies.Num() - 1; Index >= 0; --Index)
	{
		AEnemyBase* Enemy = RegisteredEnemies[Index];
		if (!IsValid(Enemy)) { RegisteredEnemies.RemoveAtSwap(Index); continue; }
		RefreshEnemyReaction(Enemy);
	}
}

bool UEnemyManagerSubsystem::FindReactionForEnemy(EEnemyType EnemyType, FEnemyReactionRule& OutRule) const
{
	const UEnemyDirectorSubsystem* Director = GetWorld() ? GetWorld()->GetSubsystem<UEnemyDirectorSubsystem>() : nullptr;
	if (!Director || Director->GetAdaptationConfidence() <= 0.0f) return false;
	for (const FEnemyReactionRule& Rule : ReactionTable)
	{
		if (Rule.EnemyType == EnemyType && RuleMatches(Rule, Director)) { OutRule = Rule; return true; }
	}
	return false;
}

bool UEnemyManagerSubsystem::RuleMatches(const FEnemyReactionRule& Rule, const UEnemyDirectorSubsystem* Director) const
{
	if (!Director || Director->GetAdaptationConfidence() < Rule.MinimumConfidence) return false;
	if (Rule.bMatchWeaponPreference && !Director->PrefersWeapon(Rule.PreferredPlayerWeapon)) return false;
	if (Rule.bMatchAirbornePreference && !Director->PrefersAirborneCombat()) return false;
	if (Rule.bMatchDestructionPreference && !Director->HasHighDestructionActivity()) return false;
	return Rule.bMatchWeaponPreference || Rule.bMatchAirbornePreference || Rule.bMatchDestructionPreference;
}

void UEnemyManagerSubsystem::HandleProfileChanged(const FAdaptivePlayerProfile& Profile)
{
	RefreshAllEnemyReactions();
}
