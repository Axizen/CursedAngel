// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/EnemyManagerSubsystem.h"
#include "AI/EnemyBase.h"
#include "AI/EnemyDirectorSubsystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

UEnemyManagerSubsystem::UEnemyManagerSubsystem()
{
	FEnemyReactionRule Rule;
	Rule.EnemyType = EEnemyType::Fodder; Rule.bMatchWeaponPreference = true; Rule.PreferredPlayerWeapon = ECurseWeaponType::CorruptionRail;
	Rule.Tactic = EEnemyAdaptiveTactic::RushPlayer; Rule.AttackPattern = TEXT("RushCounter"); Rule.EngagementDistanceMultiplier = 0.6f; ReactionTable.Add(Rule);
	Rule = FEnemyReactionRule(); Rule.EnemyType = EEnemyType::Fodder; Rule.bMatchAirbornePreference = true;
	Rule.Tactic = EEnemyAdaptiveTactic::AntiAirLine; Rule.AttackPattern = TEXT("AntiAir"); Rule.EngagementDistanceMultiplier = 1.0f; ReactionTable.Add(Rule);
	Rule = FEnemyReactionRule(); Rule.EnemyType = EEnemyType::Elite; Rule.bMatchWeaponPreference = true; Rule.PreferredPlayerWeapon = ECurseWeaponType::CorruptionRail;
	Rule.Tactic = EEnemyAdaptiveTactic::StrafePlayer; Rule.AttackPattern = TEXT("FlankRanged"); Rule.EngagementDistanceMultiplier = 1.4f; ReactionTable.Add(Rule);
	Rule = FEnemyReactionRule(); Rule.EnemyType = EEnemyType::Elite; Rule.bMatchAirbornePreference = true;
	Rule.Tactic = EEnemyAdaptiveTactic::PunishAirborne; Rule.AttackPattern = TEXT("AntiAirBurst"); Rule.EngagementDistanceMultiplier = 1.1f; ReactionTable.Add(Rule);
	Rule = FEnemyReactionRule(); Rule.EnemyType = EEnemyType::MiniBoss; Rule.bMatchDestructionPreference = true;
	Rule.Tactic = EEnemyAdaptiveTactic::DisplaceFromDestruction; Rule.AttackPattern = TEXT("AreaDenial"); Rule.EngagementDistanceMultiplier = 1.2f; ReactionTable.Add(Rule);
	Rule = FEnemyReactionRule(); Rule.EnemyType = EEnemyType::Boss; Rule.bMatchAirbornePreference = true;
	Rule.Tactic = EEnemyAdaptiveTactic::PunishAirborne; Rule.AttackPattern = TEXT("ArenaAntiAir"); ReactionTable.Add(Rule);
	Rule = FEnemyReactionRule(); Rule.EnemyType = EEnemyType::Elite; Rule.bMatchWeaponPreference = true; Rule.PreferredPlayerWeapon = ECurseWeaponType::VoidMines;
	Rule.Tactic = EEnemyAdaptiveTactic::FlankGuard; Rule.AttackPattern = TEXT("RearGuard"); Rule.EngagementDistanceMultiplier = 1.0f; ReactionTable.Add(Rule);
}

void UEnemyManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	if (UEnemyDirectorSubsystem* Director = GetWorld()->GetSubsystem<UEnemyDirectorSubsystem>()) Director->OnProfileChanged.AddDynamic(this, &UEnemyManagerSubsystem::HandleProfileChanged);
	GetWorld()->GetTimerManager().SetTimer(CoordinationTimerHandle, this, &UEnemyManagerSubsystem::UpdateCoordinatedPositions, FMath::Max(0.05f, CoordinationInterval), true);
}

void UEnemyManagerSubsystem::Deinitialize()
{
	if (GetWorld()) GetWorld()->GetTimerManager().ClearTimer(CoordinationTimerHandle);
	if (UEnemyDirectorSubsystem* Director = GetWorld() ? GetWorld()->GetSubsystem<UEnemyDirectorSubsystem>() : nullptr) Director->OnProfileChanged.RemoveDynamic(this, &UEnemyManagerSubsystem::HandleProfileChanged);
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
	if (FindReactionForEnemy(Enemy->EnemyType, Rule)) Enemy->ApplyAdaptiveReaction(Rule.Tactic, Rule.EngagementDistanceMultiplier, Rule.AttackPattern);
	else Enemy->ApplyAdaptiveReaction(EEnemyAdaptiveTactic::Default, 1.0f, NAME_None);
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

void UEnemyManagerSubsystem::UpdateCoordinatedPositions()
{
	UWorld* World = GetWorld();
	if (!World) return;
	APawn* Player = UGameplayStatics::GetPlayerPawn(World, 0);
	if (!Player) return;
	const float Dt = FMath::Max(0.05f, CoordinationInterval);
	const FVector PlayerLocation = Player->GetActorLocation();
	PlayerVelocity = bHasPlayerLocation ? (PlayerLocation - PreviousPlayerLocation) / Dt : FVector::ZeroVector;
	PreviousPlayerLocation = PlayerLocation;
	bHasPlayerLocation = true;
	const FVector Forward = (PlayerVelocity.SizeSquared2D() > 90000.0f ? PlayerVelocity.GetSafeNormal2D() : Player->GetActorForwardVector().GetSafeNormal2D());
	const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
	const bool bAirborne = Cast<ACharacter>(Player) && Cast<ACharacter>(Player)->GetCharacterMovement()->IsFalling();
	TArray<AEnemyBase*> Grunts;
	TArray<AEnemyBase*> Elites;
	for (int32 Index = RegisteredEnemies.Num() - 1; Index >= 0; --Index)
	{
		AEnemyBase* Enemy = RegisteredEnemies[Index];
		if (!IsValid(Enemy)) { RegisteredEnemies.RemoveAtSwap(Index); continue; }
		if (!Enemy->HealthComponent || !Enemy->HealthComponent->IsAlive()) continue;
		if (Enemy->EnemyType == EEnemyType::Fodder) Grunts.Add(Enemy);
		else if (Enemy->EnemyType == EEnemyType::Elite || Enemy->EnemyType == EEnemyType::MiniBoss) Elites.Add(Enemy);
	}

	// Ground grunts form an evenly spaced line across the player's approach vector while
	// aerial play is active. Each enemy gets a unique slot, so updates are coordinated.
	const bool bUseAntiAirLine = bAirborne && GetWorld()->GetSubsystem<UEnemyDirectorSubsystem>() && GetWorld()->GetSubsystem<UEnemyDirectorSubsystem>()->PrefersAirborneCombat();
	for (int32 Index = 0; Index < Grunts.Num(); ++Index)
	{
		AEnemyBase* Grunt = Grunts[Index];
		if (bUseAntiAirLine)
		{
			const float CenteredSlot = static_cast<float>(Index) - (static_cast<float>(Grunts.Num() - 1) * 0.5f);
			Grunt->SetCoordinatedMoveTarget(PlayerLocation - Forward * AntiAirLineDistance + Right * CenteredSlot * AntiAirLineSpacing);
		}
		else if (Grunt->AdaptiveTactic == EEnemyAdaptiveTactic::FlankGuard && PlayerVelocity.SizeSquared2D() > 90000.0f)
		{
			Grunt->SetCoordinatedMoveTarget(PlayerLocation - Forward * FlankGuardRadius + Right * ((Index % 2 == 0 ? 1.0f : -1.0f) * FlankGuardRadius * 0.5f));
		}
		else
		{
			Grunt->ClearCoordinatedMoveTarget();
		}
	}

	// Elite/rear-guard units rotate to cover the player's movement direction, discouraging
	// repeated dashes through a stationary choke point.
	for (int32 Index = 0; Index < Elites.Num(); ++Index)
	{
		AEnemyBase* Elite = Elites[Index];
		if (Elite->AdaptiveTactic == EEnemyAdaptiveTactic::FlankGuard && PlayerVelocity.SizeSquared2D() > 90000.0f)
		{
			const float Side = Index % 2 == 0 ? 1.0f : -1.0f;
			Elite->SetCoordinatedMoveTarget(PlayerLocation - Forward * FlankGuardRadius + Right * Side * FlankGuardRadius);
		}
		else Elite->ClearCoordinatedMoveTarget();
	}
}

int32 UEnemyManagerSubsystem::ApplyDominoKnockback(AActor* ImpactSource, FVector ImpactOrigin, float Radius, float ImpulseStrength, float UpwardImpulse)
{
	if (Radius <= 0.0f || ImpulseStrength <= 0.0f) return 0;
	int32 AffectedCount = 0;
	for (int32 Index = RegisteredEnemies.Num() - 1; Index >= 0; --Index)
	{
		AEnemyBase* Enemy = RegisteredEnemies[Index];
		if (!IsValid(Enemy)) { RegisteredEnemies.RemoveAtSwap(Index); continue; }
		if (Enemy == ImpactSource || !Enemy->HealthComponent || !Enemy->HealthComponent->IsAlive()) continue;
		const FVector Offset = Enemy->GetActorLocation() - ImpactOrigin;
		const float Distance = Offset.Size();
		if (Distance > Radius) continue;
		const float Falloff = 1.0f - FMath::Clamp(Distance / Radius, 0.0f, 1.0f);
		FVector Impulse = Offset.GetSafeNormal2D() * (ImpulseStrength * Falloff);
		Impulse.Z = UpwardImpulse * Falloff;
		Enemy->ApplyCrowdImpact(Impulse);
		++AffectedCount;
	}
	return AffectedCount;
}

void UEnemyManagerSubsystem::HandleProfileChanged(const FAdaptivePlayerProfile& Profile)
{
	RefreshAllEnemyReactions();
}
