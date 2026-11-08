// Copyright Epic Games, Inc. All Rights Reserved.

#include "Environment/EnvironmentCombatSubsystem.h"
#include "Environment/CombatDebris.h"
#include "Environment/DestructibleBase.h"
#include "Engine/World.h"

void UEnvironmentCombatSubsystem::RegisterDebris(ACombatDebris* Debris)
{
	if (!IsValid(Debris)) return;
	ActiveDebris.AddUnique(Debris);
	EnforceDebrisBudget();
}

void UEnvironmentCombatSubsystem::UnregisterDebris(ACombatDebris* Debris)
{
	ActiveDebris.Remove(Debris);
}

ACombatDebris* UEnvironmentCombatSubsystem::MagnetizeNearestDebris(AActor* Holder, float Radius)
{
	if (!IsValid(Holder) || Radius <= 0.0f) return nullptr;
	ACombatDebris* Nearest = nullptr;
	float NearestDistanceSq = FMath::Square(Radius);
	for (int32 Index = ActiveDebris.Num() - 1; Index >= 0; --Index)
	{
		ACombatDebris* Debris = ActiveDebris[Index];
		if (!IsValid(Debris)) { ActiveDebris.RemoveAtSwap(Index); continue; }
		if (!Debris->bMagnetizable || Debris->bIsMagnetized) continue;
		const float DistanceSq = FVector::DistSquared(Holder->GetActorLocation(), Debris->GetActorLocation());
		if (DistanceSq < NearestDistanceSq) { NearestDistanceSq = DistanceSq; Nearest = Debris; }
	}
	return Nearest && Nearest->BeginMagnetize(Holder) ? Nearest : nullptr;
}

bool UEnvironmentCombatSubsystem::LaunchHeldDebris(AActor* Holder, AActor* Target, float LaunchSpeed, float StyleMultiplier)
{
	if (!IsValid(Holder) || !IsValid(Target) || LaunchSpeed <= 0.0f) return false;
	for (ACombatDebris* Debris : ActiveDebris)
	{
		if (IsValid(Debris) && Debris->GetMagnetHolder() == Holder)
		{
			const FVector Direction = (Target->GetActorLocation() - Debris->GetActorLocation()).GetSafeNormal();
			Debris->LaunchDebris(Direction, LaunchSpeed, StyleMultiplier);
			return true;
		}
	}
	return false;
}

void UEnvironmentCombatSubsystem::ReleaseHeldDebris(AActor* Holder)
{
	for (ACombatDebris* Debris : ActiveDebris) if (IsValid(Debris) && Debris->GetMagnetHolder() == Holder) Debris->ReleaseMagnet();
}

void UEnvironmentCombatSubsystem::RecordArenaChange(FName RegionId, ADestructibleBase* Source)
{
	if (!RegionId.IsNone()) ChangedArenaRegions.Add(RegionId);
}

bool UEnvironmentCombatSubsystem::HasArenaRegionChanged(FName RegionId) const
{
	return !RegionId.IsNone() && ChangedArenaRegions.Contains(RegionId);
}

TArray<FName> UEnvironmentCombatSubsystem::GetChangedArenaRegions() const
{
	TArray<FName> Regions;
	ChangedArenaRegions.GenerateKeyArray(Regions);
	return Regions;
}

void UEnvironmentCombatSubsystem::EnforceDebrisBudget()
{
	const int32 Budget = FMath::Max(1, MaxActiveDebris);
	while (ActiveDebris.Num() > Budget)
	{
		int32 OldestIndex = INDEX_NONE;
		float OldestTime = TNumericLimits<float>::Max();
		for (int32 Index = 0; Index < ActiveDebris.Num(); ++Index)
		{
			ACombatDebris* Debris = ActiveDebris[Index];
			if (!IsValid(Debris) || Debris->bIsMagnetized) continue;
			if (Debris->GetSpawnTime() < OldestTime) { OldestTime = Debris->GetSpawnTime(); OldestIndex = Index; }
		}
		if (OldestIndex == INDEX_NONE) break;
		ACombatDebris* Oldest = ActiveDebris[OldestIndex];
		ActiveDebris.RemoveAtSwap(OldestIndex);
		if (IsValid(Oldest)) Oldest->Destroy();
	}
}
