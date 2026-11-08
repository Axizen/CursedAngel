// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/EnemyDirectorSubsystem.h"

#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

void UEnemyDirectorSubsystem::RecordCombatEvent(ECurseWeaponType Weapon, bool bAirborne, bool bEnemyKill, bool bEnvironmentalKill)
{
	++PlayerProfile.CombatEventCount;
	if (bAirborne)
	{
		++PlayerProfile.AirborneCombatEventCount;
	}

	++PlayerProfile.WeaponUseCounts.FindOrAdd(Weapon);

	if (bEnemyKill)
	{
		++PlayerProfile.EnemyKillCount;
		if (bEnvironmentalKill)
		{
			++PlayerProfile.EnvironmentalKillCount;
		}
	}

	BroadcastProfileChanged();
}

void UEnemyDirectorSubsystem::RecordPlayerDamageTaken(float DamageAmount)
{
	if (DamageAmount <= 0.0f)
	{
		return;
	}

	++PlayerProfile.DamageEventCount;
	PlayerProfile.TotalDamageTaken += DamageAmount;
	BroadcastProfileChanged();
}

void UEnemyDirectorSubsystem::RecordDestruction(AActor* DamageCauser, float Weight)
{
	if (Weight <= 0.0f || !IsPlayerCaused(DamageCauser))
	{
		return;
	}

	++PlayerProfile.DestructibleCount;
	PlayerProfile.DestructionWeight += Weight * FMath::Max(0.0f, Tuning.DestructionSampleWeight);
	BroadcastProfileChanged();
}

float UEnemyDirectorSubsystem::GetAirborneCombatRatio() const
{
	if (PlayerProfile.CombatEventCount <= 0)
	{
		return 0.0f;
	}

	return static_cast<float>(PlayerProfile.AirborneCombatEventCount) / PlayerProfile.CombatEventCount;
}

float UEnemyDirectorSubsystem::GetWeaponUsageRatio(ECurseWeaponType Weapon) const
{
	if (PlayerProfile.CombatEventCount <= 0)
	{
		return 0.0f;
	}

	const int32* UseCount = PlayerProfile.WeaponUseCounts.Find(Weapon);
	return UseCount ? static_cast<float>(*UseCount) / PlayerProfile.CombatEventCount : 0.0f;
}

float UEnemyDirectorSubsystem::GetAdaptationConfidence() const
{
	const int32 SampleCount = GetEffectiveSampleCount();
	const int32 StartSamples = FMath::Max(1, Tuning.AdaptationStartSamples);
	const int32 FullSamples = FMath::Max(StartSamples, Tuning.FullConfidenceSamples);

	if (SampleCount < StartSamples)
	{
		return 0.0f;
	}

	return FMath::Clamp(static_cast<float>(SampleCount) / FullSamples, 0.0f, 1.0f);
}

void UEnemyDirectorSubsystem::SetTuning(const FAdaptiveEnemyTuning& NewTuning)
{
	Tuning = NewTuning;
	Tuning.AdaptationStartSamples = FMath::Max(1, Tuning.AdaptationStartSamples);
	Tuning.FullConfidenceSamples = FMath::Max(Tuning.AdaptationStartSamples, Tuning.FullConfidenceSamples);
	Tuning.DestructionSampleWeight = FMath::Max(0.0f, Tuning.DestructionSampleWeight);
	BroadcastProfileChanged();
}

void UEnemyDirectorSubsystem::ResetProfile()
{
	PlayerProfile = FAdaptivePlayerProfile();
	BroadcastProfileChanged();
}

int32 UEnemyDirectorSubsystem::GetEffectiveSampleCount() const
{
	const float WeightedDestructionSamples = PlayerProfile.DestructionWeight * FMath::Max(0.0f, Tuning.DestructionSampleWeight);
	return PlayerProfile.CombatEventCount + PlayerProfile.DamageEventCount + FMath::FloorToInt(WeightedDestructionSamples);
}

bool UEnemyDirectorSubsystem::IsPlayerCaused(const AActor* SourceActor) const
{
	if (!SourceActor || !GetWorld())
	{
		return false;
	}

	const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn)
	{
		return false;
	}

	const AActor* CurrentActor = SourceActor;
	for (int32 Depth = 0; CurrentActor && Depth < 8; ++Depth)
	{
		if (CurrentActor == PlayerPawn || CurrentActor->IsOwnedBy(PlayerPawn) || CurrentActor->GetInstigator() == PlayerPawn)
		{
			return true;
		}

		const AController* InstigatorController = CurrentActor->GetInstigatorController();
		if (InstigatorController && InstigatorController->IsPlayerController())
		{
			return true;
		}

		CurrentActor = CurrentActor->GetOwner();
	}

	return false;
}

void UEnemyDirectorSubsystem::BroadcastProfileChanged()
{
	OnProfileChanged.Broadcast(PlayerProfile);
}
