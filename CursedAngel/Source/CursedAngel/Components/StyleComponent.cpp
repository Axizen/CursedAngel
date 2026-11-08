// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/StyleComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "AI/EnemyDirectorSubsystem.h"

UStyleComponent::UStyleComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	CurrentStylePoints = 0.0f;
	CurrentRank = EStyleRank::D;
	ComboCount = 0;
	LastHitTime = 0.0f;
	LastWeaponUsed = ECurseWeaponType::CorruptionRail;
	EnvironmentalKillCount = 0;
	StylePointsConfig.BasePoints = 10.0f;
	StylePointsConfig.WeaponVarietyBonus = 5.0f;
	StylePointsConfig.TimingBonus = 10.0f;
	StylePointsConfig.EnvironmentalBonus = 15.0f;
}

void UStyleComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeDefaultRankData();
	CurrentStylePoints = 0.0f;
	CurrentRank = EStyleRank::D;
	ComboCount = 0;
	LastHitTime = 0.0f;
	EnvironmentalKillCount = 0;
}

void UStyleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	DecayStylePoints(DeltaTime);
}

void UStyleComponent::InitializeDefaultRankData()
{
	if (RankDataMap.Num() != 0) return;
	const float Thresholds[] = {0.0f, 100.0f, 300.0f, 600.0f, 1000.0f};
	const float Multipliers[] = {1.0f, 1.1f, 1.25f, 1.5f, 2.0f};
	const float Decays[] = {5.0f, 7.0f, 10.0f, 15.0f, 20.0f};
	const float VFX[] = {0.5f, 0.75f, 1.0f, 1.5f, 2.0f};
	for (int32 Index = 0; Index < 5; ++Index)
	{
		FStyleRankData Data;
		Data.PointsRequired = Thresholds[Index];
		Data.DamageMultiplier = Multipliers[Index];
		Data.DecayRate = Decays[Index];
		Data.VFXIntensity = VFX[Index];
		RankDataMap.Add(static_cast<EStyleRank>(Index), Data);
	}
}

void UStyleComponent::AddStylePoints(float Points)
{
	if (Points <= 0.0f) return;
	CurrentStylePoints += Points;
	if (UWorld* World = GetWorld()) LastHitTime = World->GetTimeSeconds();
	UpdateRank();
	OnStylePointsChanged(CurrentStylePoints, CurrentRank);
	OnStylePointsChangedDelegate.Broadcast(CurrentStylePoints, CurrentRank);
}

void UStyleComponent::OnEnemyKilled(AActor* Enemy, ECurseWeaponType Weapon, bool bEnvironmental)
{
	if (!Enemy) return;
	AddStylePoints(CalculateKillStylePoints(Weapon, bEnvironmental) * GetDamageMultiplier());
	++ComboCount;
	OnComboIncreased(ComboCount);
	OnComboIncreasedDelegate.Broadcast(ComboCount);
	if (bEnvironmental) ++EnvironmentalKillCount;
	LastWeaponUsed = Weapon;

	// Feed a per-level sample to the adaptive director. Style points decay; this observation does not.
	if (UWorld* World = GetWorld())
	{
		if (UEnemyDirectorSubsystem* Director = World->GetSubsystem<UEnemyDirectorSubsystem>())
		{
			bool bAirborne = false;
			if (const ACharacter* Character = Cast<ACharacter>(GetOwner()))
			{
				if (const UCharacterMovementComponent* Movement = Character->GetCharacterMovement()) bAirborne = Movement->IsFalling();
			}
			Director->RecordCombatEvent(Weapon, bAirborne, true, bEnvironmental);
		}
	}
}

float UStyleComponent::CalculateKillStylePoints(ECurseWeaponType Weapon, bool bEnvironmental)
{
	float Points = StylePointsConfig.BasePoints;
	if (ShouldApplyWeaponVarietyBonus(Weapon)) Points += StylePointsConfig.WeaponVarietyBonus;
	if (ShouldApplyTimingBonus()) Points += StylePointsConfig.TimingBonus;
	if (bEnvironmental) Points += StylePointsConfig.EnvironmentalBonus;
	return Points;
}

bool UStyleComponent::ShouldApplyWeaponVarietyBonus(ECurseWeaponType Weapon) const
{
	return ComboCount > 0 && Weapon != LastWeaponUsed;
}

bool UStyleComponent::ShouldApplyTimingBonus() const
{
	if (ComboCount == 0) return false;
	if (const UWorld* World = GetWorld()) return World->GetTimeSeconds() - LastHitTime <= TimingBonusWindow;
	return false;
}

void UStyleComponent::UpdateRank()
{
	const EStyleRank OldRank = CurrentRank;
	EStyleRank NewRank = EStyleRank::D;
	for (int32 Index = static_cast<int32>(EStyleRank::S); Index >= static_cast<int32>(EStyleRank::C); --Index)
	{
		const EStyleRank Rank = static_cast<EStyleRank>(Index);
		const FStyleRankData* Data = RankDataMap.Find(Rank);
		if (Data && CurrentStylePoints >= Data->PointsRequired) { NewRank = Rank; break; }
	}
	if (NewRank != OldRank)
	{
		CurrentRank = NewRank;
		OnRankChanged(NewRank, OldRank);
		OnStyleRankChangedDelegate.Broadcast(NewRank, OldRank);
	}
}

void UStyleComponent::DecayStylePoints(float DeltaTime)
{
	if (CurrentStylePoints <= 0.0f || !GetWorld() || GetWorld()->GetTimeSeconds() - LastHitTime < ComboDecayDelay) return;
	const FStyleRankData* Data = RankDataMap.Find(CurrentRank);
	CurrentStylePoints = FMath::Max(0.0f, CurrentStylePoints - (Data ? Data->DecayRate : 5.0f) * DeltaTime);
	if (CurrentStylePoints <= 0.0f) ResetCombo();
	else
	{
		UpdateRank();
		OnStylePointsChanged(CurrentStylePoints, CurrentRank);
		OnStylePointsChangedDelegate.Broadcast(CurrentStylePoints, CurrentRank);
	}
}

void UStyleComponent::ResetCombo()
{
	ComboCount = 0;
	EnvironmentalKillCount = 0;
	CurrentStylePoints = 0.0f;
	const EStyleRank OldRank = CurrentRank;
	CurrentRank = EStyleRank::D;
	if (OldRank != CurrentRank)
	{
		OnRankChanged(CurrentRank, OldRank);
		OnStyleRankChangedDelegate.Broadcast(CurrentRank, OldRank);
	}
	OnStylePointsChanged(CurrentStylePoints, CurrentRank);
	OnStylePointsChangedDelegate.Broadcast(CurrentStylePoints, CurrentRank);
	OnComboIncreased(ComboCount);
	OnComboIncreasedDelegate.Broadcast(ComboCount);
}

float UStyleComponent::GetDamageMultiplier() const
{
	const FStyleRankData* Data = RankDataMap.Find(CurrentRank);
	return Data ? Data->DamageMultiplier : 1.0f;
}
