// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CombatTypes.h"
#include "EnemyDirectorSubsystem.generated.h"

/** Per-level observations. Counts are zero at level start and increase only as gameplay events occur. */
USTRUCT(BlueprintType)
struct CURSEDANGEL_API FAdaptivePlayerProfile
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy Director|Profile") int32 CombatEventCount = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy Director|Profile") int32 AirborneCombatEventCount = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy Director|Profile") int32 EnemyKillCount = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy Director|Profile") int32 EnvironmentalKillCount = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy Director|Profile") int32 DamageEventCount = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy Director|Profile") float TotalDamageTaken = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy Director|Profile") int32 DestructibleCount = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy Director|Profile") float DestructionWeight = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy Director|Profile") TMap<ECurseWeaponType, int32> WeaponUseCounts;
};

/** Designer controls for the confidence gate and player-behavior thresholds. */
USTRUCT(BlueprintType)
struct CURSEDANGEL_API FAdaptiveEnemyTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Director|Tuning", meta=(ClampMin="1", ToolTip="Evidence required before enemies adapt. With fewer samples, all adaptive queries return neutral."))
	int32 AdaptationStartSamples = 5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Director|Tuning", meta=(ClampMin="1", ToolTip="Evidence count at which adaptation confidence reaches 1.0."))
	int32 FullConfidenceSamples = 20;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Director|Tuning", meta=(ClampMin="0.0", ToolTip="How much each player-caused destruction contributes to the sample count."))
	float DestructionSampleWeight = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Director|Tuning", meta=(ClampMin="0.0", ClampMax="1.0", ToolTip="A weapon is considered a player preference at or above this fraction of combat observations."))
	float WeaponPreferenceThreshold = 0.55f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Director|Tuning", meta=(ClampMin="0.0", ClampMax="1.0", ToolTip="Airborne combat ratio at or above which enemies may counter aerial play."))
	float AirbornePreferenceThreshold = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Director|Tuning", meta=(ClampMin="0.0", ToolTip="Weighted destruction activity required before enemies may react to prop destruction."))
	float DestructionPreferenceThreshold = 3.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAdaptivePlayerProfileChanged, const FAdaptivePlayerProfile&, Profile);

/** Level-local event collector and query API for adaptive enemy behavior. */
UCLASS(BlueprintType)
class CURSEDANGEL_API UEnemyDirectorSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy Director|Tuning", meta=(ToolTip="Runtime adaptation thresholds. Set via SetTuning from Blueprint BeginPlay or C++."))
	FAdaptiveEnemyTuning Tuning;
	UPROPERTY(BlueprintAssignable, Category="Enemy Director|Events", meta=(ToolTip="Broadcast after an observation changes the level profile."))
	FOnAdaptivePlayerProfileChanged OnProfileChanged;

	/** Record a landed combat event. Use bEnemyKill/bEnvironmentalKill only for the final killing event. */
	UFUNCTION(BlueprintCallable, Category="Enemy Director|Profile")
	void RecordCombatEvent(ECurseWeaponType Weapon, bool bAirborne, bool bEnemyKill, bool bEnvironmentalKill);
	/** Record positive damage actually received by the player. */
	UFUNCTION(BlueprintCallable, Category="Enemy Director|Profile")
	void RecordPlayerDamageTaken(float DamageAmount);
	/** Record a prop destroyed by the player, their weapon/projectile, or an owned child actor. */
	UFUNCTION(BlueprintCallable, Category="Enemy Director|Profile")
	void RecordDestruction(AActor* DamageCauser, float Weight = 1.0f);
	UFUNCTION(BlueprintPure, Category="Enemy Director|Profile")
	FAdaptivePlayerProfile GetPlayerProfile() const { return PlayerProfile; }
	UFUNCTION(BlueprintPure, Category="Enemy Director|Profile")
	float GetAirborneCombatRatio() const;
	UFUNCTION(BlueprintPure, Category="Enemy Director|Profile")
	float GetWeaponUsageRatio(ECurseWeaponType Weapon) const;
	UFUNCTION(BlueprintPure, Category="Enemy Director|Profile")
	float GetAdaptationConfidence() const;
	UFUNCTION(BlueprintPure, Category="Enemy Director|Profile")
	bool PrefersWeapon(ECurseWeaponType Weapon) const;
	UFUNCTION(BlueprintPure, Category="Enemy Director|Profile")
	bool PrefersAirborneCombat() const;
	UFUNCTION(BlueprintPure, Category="Enemy Director|Profile")
	bool HasHighDestructionActivity() const;
	UFUNCTION(BlueprintCallable, Category="Enemy Director|Tuning")
	void SetTuning(const FAdaptiveEnemyTuning& NewTuning);
	UFUNCTION(BlueprintCallable, Category="Enemy Director|Profile")
	void ResetProfile();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy Director|Profile", meta=(AllowPrivateAccess="true"))
	FAdaptivePlayerProfile PlayerProfile;
	int32 GetEffectiveSampleCount() const;
	bool IsPlayerCaused(const AActor* SourceActor) const;
	void BroadcastProfileChanged();
};
