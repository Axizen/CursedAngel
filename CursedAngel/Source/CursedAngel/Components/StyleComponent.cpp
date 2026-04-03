// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/StyleComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

UStyleComponent::UStyleComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	// Initialize default values
	CurrentStylePoints = 0.0f;
	CurrentRank = EStyleRank::D;
	ComboCount = 0;
	LastHitTime = 0.0f;
	LastWeaponUsed = ECurseWeaponType::CorruptionRail;
	EnvironmentalKillCount = 0;

	// Initialize default style points config
	StylePointsConfig.BasePoints = 10.0f;
	StylePointsConfig.WeaponVarietyBonus = 5.0f;
	StylePointsConfig.TimingBonus = 10.0f;
	StylePointsConfig.EnvironmentalBonus = 15.0f;

	ComboDecayDelay = 3.0f;
	TimingBonusWindow = 2.0f;
}

void UStyleComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize rank data with default values if not configured
	InitializeDefaultRankData();

	// Reset to starting state
	CurrentStylePoints = 0.0f;
	CurrentRank = EStyleRank::D;
	ComboCount = 0;
	LastHitTime = 0.0f;
	EnvironmentalKillCount = 0;
}

void UStyleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Handle style point decay if no hits for a while
	DecayStylePoints(DeltaTime);
}

void UStyleComponent::InitializeDefaultRankData()
{
	// Only initialize if the map is empty
	if (RankDataMap.Num() == 0)
	{
		// D Rank: 0-100 points
		FStyleRankData RankD;
		RankD.PointsRequired = 0.0f;
		RankD.DamageMultiplier = 1.0f;
		RankD.DecayRate = 5.0f;
		RankD.VFXIntensity = 0.5f;
		RankDataMap.Add(EStyleRank::D, RankD);

		// C Rank: 100-300 points
		FStyleRankData RankC;
		RankC.PointsRequired = 100.0f;
		RankC.DamageMultiplier = 1.1f;
		RankC.DecayRate = 7.0f;
		RankC.VFXIntensity = 0.75f;
		RankDataMap.Add(EStyleRank::C, RankC);

		// B Rank: 300-600 points
		FStyleRankData RankB;
		RankB.PointsRequired = 300.0f;
		RankB.DamageMultiplier = 1.25f;
		RankB.DecayRate = 10.0f;
		RankB.VFXIntensity = 1.0f;
		RankDataMap.Add(EStyleRank::B, RankB);

		// A Rank: 600-1000 points
		FStyleRankData RankA;
		RankA.PointsRequired = 600.0f;
		RankA.DamageMultiplier = 1.5f;
		RankA.DecayRate = 15.0f;
		RankA.VFXIntensity = 1.5f;
		RankDataMap.Add(EStyleRank::A, RankA);

		// S Rank: 1000+ points
		FStyleRankData RankS;
		RankS.PointsRequired = 1000.0f;
		RankS.DamageMultiplier = 2.0f;
		RankS.DecayRate = 20.0f;
		RankS.VFXIntensity = 2.0f;
		RankDataMap.Add(EStyleRank::S, RankS);
	}
}

void UStyleComponent::AddStylePoints(float Points)
{
	if (Points <= 0.0f)
	{
		return;
	}

	CurrentStylePoints += Points;
	
	// Update last hit time
	if (UWorld* World = GetWorld())
	{
		LastHitTime = World->GetTimeSeconds();
	}

	// Update rank if necessary
	UpdateRank();

	// Fire Blueprint event
	OnStylePointsChanged(CurrentStylePoints, CurrentRank);
}

void UStyleComponent::OnEnemyKilled(AActor* Enemy, ECurseWeaponType Weapon, bool bEnvironmental)
{
	if (!Enemy)
	{
		return;
	}

	// Calculate style points for this kill
	float KillPoints = CalculateKillStylePoints(Weapon, bEnvironmental);

	// Apply current rank multiplier
	float RankMultiplier = GetDamageMultiplier();
	KillPoints *= RankMultiplier;

	// Add the points
	AddStylePoints(KillPoints);

	// Increment combo count
	ComboCount++;
	OnComboIncreased(ComboCount);

	// Track environmental kills
	if (bEnvironmental)
	{
		EnvironmentalKillCount++;
	}

	// Update last weapon used
	LastWeaponUsed = Weapon;
}

float UStyleComponent::CalculateKillStylePoints(ECurseWeaponType Weapon, bool bEnvironmental)
{
	float Points = StylePointsConfig.BasePoints;

	// Weapon variety bonus
	if (ShouldApplyWeaponVarietyBonus(Weapon))
	{
		Points += StylePointsConfig.WeaponVarietyBonus;
	}

	// Timing bonus (quick successive kills)
	if (ShouldApplyTimingBonus())
	{
		Points += StylePointsConfig.TimingBonus;
	}

	// Environmental kill bonus
	if (bEnvironmental)
	{
		Points += StylePointsConfig.EnvironmentalBonus;
	}

	return Points;
}

bool UStyleComponent::ShouldApplyWeaponVarietyBonus(ECurseWeaponType Weapon) const
{
	// Apply bonus if switching weapons (not using the same weapon as last kill)
	return (ComboCount > 0 && Weapon != LastWeaponUsed);
}

bool UStyleComponent::ShouldApplyTimingBonus() const
{
	// Apply bonus if kill happened within timing window since last hit
	if (ComboCount == 0)
	{
		return false;
	}

	if (UWorld* World = GetWorld())
	{
		float CurrentTime = World->GetTimeSeconds();
		float TimeSinceLastHit = CurrentTime - LastHitTime;
		return (TimeSinceLastHit <= TimingBonusWindow);
	}

	return false;
}

void UStyleComponent::UpdateRank()
{
	EStyleRank OldRank = CurrentRank;
	EStyleRank NewRank = EStyleRank::D;

	// Determine new rank based on points
	// Check from highest to lowest
	if (CurrentStylePoints >= RankDataMap[EStyleRank::S].PointsRequired)
	{
		NewRank = EStyleRank::S;
	}
	else if (CurrentStylePoints >= RankDataMap[EStyleRank::A].PointsRequired)
	{
		NewRank = EStyleRank::A;
	}
	else if (CurrentStylePoints >= RankDataMap[EStyleRank::B].PointsRequired)
	{
		NewRank = EStyleRank::B;
	}
	else if (CurrentStylePoints >= RankDataMap[EStyleRank::C].PointsRequired)
	{
		NewRank = EStyleRank::C;
	}
	else
	{
		NewRank = EStyleRank::D;
	}

	// Update rank if changed
	if (NewRank != OldRank)
	{
		CurrentRank = NewRank;
		OnRankChanged(NewRank, OldRank);
	}
}

void UStyleComponent::DecayStylePoints(float DeltaTime)
{
	if (CurrentStylePoints <= 0.0f)
	{
		return;
	}

	// Check if enough time has passed since last hit
	if (UWorld* World = GetWorld())
	{
		float CurrentTime = World->GetTimeSeconds();
		float TimeSinceLastHit = CurrentTime - LastHitTime;

		if (TimeSinceLastHit >= ComboDecayDelay)
		{
			// Get decay rate for current rank
			float DecayRate = 5.0f; // Default decay rate
			if (RankDataMap.Contains(CurrentRank))
			{
				DecayRate = RankDataMap[CurrentRank].DecayRate;
			}

			// Decay points
			CurrentStylePoints -= DecayRate * DeltaTime;
			
			// Clamp to zero
			if (CurrentStylePoints < 0.0f)
			{
				CurrentStylePoints = 0.0f;
				ResetCombo();
			}

			// Update rank if necessary
			UpdateRank();

			// Fire Blueprint event
			OnStylePointsChanged(CurrentStylePoints, CurrentRank);
		}
	}
}

void UStyleComponent::ResetCombo()
{
	ComboCount = 0;
	EnvironmentalKillCount = 0;
	CurrentStylePoints = 0.0f;
	CurrentRank = EStyleRank::D;
	
	// Fire events
	OnStylePointsChanged(CurrentStylePoints, CurrentRank);
	OnComboIncreased(ComboCount);
}

float UStyleComponent::GetDamageMultiplier() const
{
	if (RankDataMap.Contains(CurrentRank))
	{
		return RankDataMap[CurrentRank].DamageMultiplier;
	}

	return 1.0f; // Default multiplier
}
