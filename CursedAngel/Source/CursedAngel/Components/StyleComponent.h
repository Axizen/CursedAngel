// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatTypes.h"
#include "Components/CurseWeaponComponent.h"
#include "StyleComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStyleRankChanged, EStyleRank, NewRank, EStyleRank, OldRank);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStylePointsChanged, float, Points, EStyleRank, Rank);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComboIncreased, int32, NewComboCount);

USTRUCT(BlueprintType)
struct CURSEDANGEL_API FStyleRankData
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Style Rank") float PointsRequired = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Style Rank") float DamageMultiplier = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Style Rank") float DecayRate = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Style Rank") float VFXIntensity = 1.0f;
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CURSEDANGEL_API UStyleComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UStyleComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Style System") float CurrentStylePoints = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Style System") EStyleRank CurrentRank = EStyleRank::D;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Style System") int32 ComboCount = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Style System") float LastHitTime = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Style System") ECurseWeaponType LastWeaponUsed = ECurseWeaponType::CorruptionRail;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Style System") int32 EnvironmentalKillCount = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Style System|Config") FStylePointsConfig StylePointsConfig;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Style System|Config") TMap<EStyleRank, FStyleRankData> RankDataMap;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Style System|Config", meta=(ToolTip="Seconds without a hit before current style points start decaying.")) float ComboDecayDelay = 3.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Style System|Config", meta=(ToolTip="Timing window in seconds for consecutive-kill style bonuses.")) float TimingBonusWindow = 2.0f;

	UFUNCTION(BlueprintCallable, Category="Style System") void AddStylePoints(float Points);
	UFUNCTION(BlueprintCallable, Category="Style System") void OnEnemyKilled(AActor* Enemy, ECurseWeaponType Weapon, bool bEnvironmental);
	UFUNCTION(BlueprintCallable, Category="Style System") void UpdateRank();
	UFUNCTION(BlueprintCallable, Category="Style System") void DecayStylePoints(float DeltaTime);
	UFUNCTION(BlueprintCallable, Category="Style System") void ResetCombo();
	UFUNCTION(BlueprintPure, Category="Style System") float GetDamageMultiplier() const;
	UFUNCTION(BlueprintPure, Category="Style System") EStyleRank GetCurrentRank() const { return CurrentRank; }
	UFUNCTION(BlueprintPure, Category="Style System") float GetCurrentStylePoints() const { return CurrentStylePoints; }
	UFUNCTION(BlueprintPure, Category="Style System") int32 GetComboCount() const { return ComboCount; }

	UFUNCTION(BlueprintImplementableEvent, Category="Style System|Events") void OnRankChanged(EStyleRank NewRank, EStyleRank OldRank);
	UFUNCTION(BlueprintImplementableEvent, Category="Style System|Events") void OnStylePointsChanged(float Points, EStyleRank Rank);
	UFUNCTION(BlueprintImplementableEvent, Category="Style System|Events") void OnComboIncreased(int32 NewComboCount);
	UPROPERTY(BlueprintAssignable, Category="Style System|Delegates") FOnStyleRankChanged OnStyleRankChangedDelegate;
	UPROPERTY(BlueprintAssignable, Category="Style System|Delegates") FOnStylePointsChanged OnStylePointsChangedDelegate;
	UPROPERTY(BlueprintAssignable, Category="Style System|Delegates") FOnComboIncreased OnComboIncreasedDelegate;

private:
	void InitializeDefaultRankData();
	float CalculateKillStylePoints(ECurseWeaponType Weapon, bool bEnvironmental);
	bool ShouldApplyWeaponVarietyBonus(ECurseWeaponType Weapon) const;
	bool ShouldApplyTimingBonus() const;
};
