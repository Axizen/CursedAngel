// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/EnemyDirectorSubsystem.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

void UEnemyDirectorSubsystem::RecordCombatEvent(ECurseWeaponType Weapon, bool bAirborne, bool bEnemyKill, bool bEnvironmentalKill)
{
	++PlayerProfile.CombatEventCount;
	if (bAirborne) ++PlayerProfile.AirborneCombatEventCount;
	++PlayerProfile.WeaponUseCounts.FindOrAdd(Weapon);
	if (bEnemyKill)
	{
		++PlayerProfile.EnemyKillCount;
		if (bEnvironmentalKill) ++PlayerProfile.EnvironmentalKillCount;
	}
	BroadcastProfileChanged();
}

void UEnemyDirectorSubsystem::RecordPlayerDamageTaken(float DamageAmount)
{
	if (DamageAmount <= 0.0f) return;
	++PlayerProfile.DamageEventCount;
	PlayerProfile.TotalDamageTaken += DamageAmount;
	BroadcastProfileChanged();
}

void UEnemyDirectorSubsystem::RecordDestruction(AActor* DamageCauser, float Weight)
{
	if (Weight <= 0.0f || !IsPlayerCaused(DamageCauser)) return;
	++PlayerProfile.DestructibleCount;
	PlayerProfile.DestructionWeight += Weight * FMath::Max(0.0f, Tuning.DestructionSampleWeight);
	BroadcastProfileChanged();
}

float UEnemyDirectorSubsystem::GetAirborneCombatRatio() const
{
	return PlayerProfile.CombatEventCount > 0 ? static_cast<float>(PlayerProfile.AirborneCombatEventCount) / PlayerProfile.CombatEventCount : 0.0f;
}

float UEnemyDirectorSubsystem::GetWeaponUsageRatio(ECurseWeaponType Weapon) const
{
	if (PlayerProfile.CombatEventCount <= 0) return 0.0f;
	const int32* Count = PlayerProfile.WeaponUseCounts.Find(Weapon);
	return Count ? static_cast<float>(*Count) / PlayerProfile.CombatEventCount : 0.0f;
}

float UEnemyDirectorSubsystem::GetAdaptationConfidence() const
{
	const int32 Start = FMath::Max(1, Tuning.AdaptationStartSamples);
	const int32 Full = FMath::Max(Start, Tuning.FullConfidenceSamples);
	const int32 Samples = GetEffectiveSampleCount();
	return Samples < Start ? 0.0f : FMath::Clamp(static_cast<float>(Samples) / Full, 0.0f, 1.0f);
}

bool UEnemyDirectorSubsystem::PrefersWeapon(ECurseWeaponType Weapon) const
{
	return GetAdaptationConfidence() > 0.0f && GetWeaponUsageRatio(Weapon) >= FMath::Clamp(Tuning.WeaponPreferenceThreshold, 0.0f, 1.0f);
}

bool UEnemyDirectorSubsystem::PrefersAirborneCombat() const
{
	return GetAdaptationConfidence() > 0.0f && GetAirborneCombatRatio() >= FMath::Clamp(Tuning.AirbornePreferenceThreshold, 0.0f, 1.0f);
}

bool UEnemyDirectorSubsystem::HasHighDestructionActivity() const
{
	return GetAdaptationConfidence() > 0.0f && PlayerProfile.DestructionWeight >= FMath::Max(0.0f, Tuning.DestructionPreferenceThreshold);
}

void UEnemyDirectorSubsystem::SetTuning(const FAdaptiveEnemyTuning& NewTuning)
{
	Tuning = NewTuning;
	Tuning.AdaptationStartSamples = FMath::Max(1, Tuning.AdaptationStartSamples);
	Tuning.FullConfidenceSamples = FMath::Max(Tuning.AdaptationStartSamples, Tuning.FullConfidenceSamples);
	Tuning.DestructionSampleWeight = FMath::Max(0.0f, Tuning.DestructionSampleWeight);
	Tuning.WeaponPreferenceThreshold = FMath::Clamp(Tuning.WeaponPreferenceThreshold, 0.0f, 1.0f);
	Tuning.AirbornePreferenceThreshold = FMath::Clamp(Tuning.AirbornePreferenceThreshold, 0.0f, 1.0f);
	Tuning.DestructionPreferenceThreshold = FMath::Max(0.0f, Tuning.DestructionPreferenceThreshold);
	BroadcastProfileChanged();
}

void UEnemyDirectorSubsystem::ResetProfile()
{
	PlayerProfile = FAdaptivePlayerProfile();
	BroadcastProfileChanged();
}

int32 UEnemyDirectorSubsystem::GetEffectiveSampleCount() const
{
	const float WeightedDestruction = PlayerProfile.DestructionWeight * FMath::Max(0.0f, Tuning.DestructionSampleWeight);
	return PlayerProfile.CombatEventCount + PlayerProfile.DamageEventCount + FMath::FloorToInt(WeightedDestruction);
}

bool UEnemyDirectorSubsystem::IsPlayerCaused(const AActor* SourceActor) const
{
	if (!SourceActor || !GetWorld()) return false;
	const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn) return false;
	const AActor* Current = SourceActor;
	for (int32 Depth = 0; Current && Depth < 8; ++Depth)
	{
		if (Current == PlayerPawn || Current->IsOwnedBy(PlayerPawn) || Current->GetInstigator() == PlayerPawn) return true;
		const AController* Controller = Current->GetInstigatorController();
		if (Controller && Controller->IsPlayerController()) return true;
		Current = Current->GetOwner();
	}
	return false;
}

void UEnemyDirectorSubsystem::BroadcastProfileChanged()
{
	OnProfileChanged.Broadcast(PlayerProfile);
}
