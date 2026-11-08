// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CombatTypes.h"
#include "EnemyDirectorSubsystem.generated.h"

/** Accumulated observations for adaptive enemy behavior. Counts begin at zero and are gathered during play. */
USTRUCT(BlueprintType)
struct CURSEDANGEL_API FAdaptivePlayerProfile
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy Director|Profile")
	int32 CombatEventCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy Director|Profile")
	int32 AirborneCombatEventCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy Director|Profile")
	int32 EnemyKillCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy Director|Profile")
	int32 EnvironmentalKillCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy Director|Profile")
	int32 DamageEventCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy Director|Profile")
	float TotalDamageTaken = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy Director|Profile")
	int32 DestructibleCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy Director|Profile")
	float DestructionWeight = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy Director|Profile")
	TMap<ECurseWeaponType, int32> WeaponUseCounts;
};

/** Blueprint-tunable evidence thresholds for turning observations into an adaptation confidence. */
USTRUCT(BlueprintType)
struct CURSEDANGEL_API FAdaptiveEnemyTuning
{
	GENERATED_BODY()

	/** Number of observations required before adaptation can begin. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Director|Tuning", meta = (ClampMin = "1"))
	int32 AdaptationStartSamples = 5;

	/** Number of observations at which adaptation confidence reaches 1. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Director|Tuning", meta = (ClampMin = "1"))
	int32 FullConfidenceSamples = 20;

	/** Multiplier applied to weighted environmental destruction observations. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Director|Tuning", meta = (ClampMin = "0.0"))
	float DestructionSampleWeight = 1.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAdaptivePlayerProfileChanged, const FAdaptivePlayerProfile&, Profile);

/** Level-local collector and query API for player behavior observations used by enemy AI. */
UCLASS(BlueprintType, Blueprintable)
class CURSEDANGEL_API UEnemyDirectorSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Blueprint-tunable adaptation settings. Configure through a subsystem Blueprint or SetTuning. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Director|Tuning")
	FAdaptiveEnemyTuning Tuning;

	/** Broadcast after a profile observation has been recorded. */
	UPROPERTY(BlueprintAssignable, Category = "Enemy Director|Events")
	FOnAdaptivePlayerProfileChanged OnProfileChanged;

	/** Record a combat action; optionally count it as a kill and/or environmental kill. */
	UFUNCTION(BlueprintCallable, Category = "Enemy Director|Profile")
	void RecordCombatEvent(ECurseWeaponType Weapon, bool bAirborne, bool bEnemyKill, bool bEnvironmentalKill);

	/** Record damage received by the player. Non-positive values are ignored. */
	UFUNCTION(BlueprintCallable, Category = "Enemy Director|Profile")
	void RecordPlayerDamageTaken(float DamageAmount);

	/** Record a destructible broken by a player-caused actor (player, weapon, or projectile). */
	UFUNCTION(BlueprintCallable, Category = "Enemy Director|Profile")
	void RecordDestruction(AActor* DamageCauser, float Weight = 1.0f);

	/** Return a copy of the current level profile. Empty at level start. */
	UFUNCTION(BlueprintPure, Category = "Enemy Director|Profile")
	FAdaptivePlayerProfile GetPlayerProfile() const { return PlayerProfile; }

	/** Fraction of combat observations performed airborne, in [0,1]. */
	UFUNCTION(BlueprintPure, Category = "Enemy Director|Profile")
	float GetAirborneCombatRatio() const;

	/** Fraction of recorded combat actions using this weapon, in [0,1]. */
	UFUNCTION(BlueprintPure, Category = "Enemy Director|Profile")
	float GetWeaponUsageRatio(ECurseWeaponType Weapon) const;

	/** Adaptation confidence in [0,1]; remains zero until the configured evidence threshold is met. */
	UFUNCTION(BlueprintPure, Category = "Enemy Director|Profile")
	float GetAdaptationConfidence() const;

	/** Set tuning at runtime, including from Blueprint BeginPlay. */
	UFUNCTION(BlueprintCallable, Category = "Enemy Director|Tuning")
	void SetTuning(const FAdaptiveEnemyTuning& NewTuning);

	/** Clear observations, useful when explicitly restarting a level's adaptive encounter. */
	UFUNCTION(BlueprintCallable, Category = "Enemy Director|Profile")
	void ResetProfile();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy Director|Profile", meta = (AllowPrivateAccess = "true"))
	FAdaptivePlayerProfile PlayerProfile;

	int32 GetEffectiveSampleCount() const;
	bool IsPlayerCaused(const AActor* SourceActor) const;
	void BroadcastProfileChanged();
};
