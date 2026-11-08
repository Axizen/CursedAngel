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
			Debris->LaunchDebris((Target->GetActorLocation() - Debris->GetActorLocation()).GetSafeNormal(), LaunchSpeed, StyleMultiplier);
			return true;
		}
	}
	return false;
}

void UEnvironmentCombatSubsystem::ReleaseHeldDebris(AActor* Holder)
{
	for (ACombatDebris* Debris : ActiveDebris)
	{
		if (IsValid(Debris) && Debris->GetMagnetHolder() == Holder) Debris->ReleaseMagnet();
	}
}

void UEnvironmentCombatSubsystem::RecordArenaChange(FName RegionId, ADestructibleBase* Source)
{
	RecordStructuralPieceBroken(RegionId, Source, 0.0f);
}

void UEnvironmentCombatSubsystem::RecordStructuralPieceBroken(FName RegionId, ADestructibleBase* Source, float ImpactSpeed)
{
	if (RegionId.IsNone()) return;
	FArenaRegionState& State = ArenaRegions.FindOrAdd(RegionId);
	State.RegionId = RegionId;
	const bool bWasChanged = State.bChanged;
	State.bChanged = true;
	++State.StructuralPiecesBroken;
	State.LastImpactSpeed = FMath::Max(State.LastImpactSpeed, ImpactSpeed);
	// Notify on each structural break so Blueprint routes can update incrementally (e.g. bridge sections).
	OnArenaRegionChanged.Broadcast(RegionId, Source);
}

bool UEnvironmentCombatSubsystem::HasArenaRegionChanged(FName RegionId) const
{
	const FArenaRegionState* State = ArenaRegions.Find(RegionId);
	return State && State->bChanged;
}

FArenaRegionState UEnvironmentCombatSubsystem::GetArenaRegionState(FName RegionId) const
{
	if (const FArenaRegionState* State = ArenaRegions.Find(RegionId)) return *State;
	FArenaRegionState Empty;
	Empty.RegionId = RegionId;
	return Empty;
}

TArray<FName> UEnvironmentCombatSubsystem::GetChangedArenaRegions() const
{
	TArray<FName> Regions;
	ArenaRegions.GetKeys(Regions);
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
