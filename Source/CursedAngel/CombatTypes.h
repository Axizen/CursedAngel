// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CombatTypes.generated.h"

ENUM_CLASS_FLAGS(ECurseWeaponType);

/**
 * Curse weapon type enumeration
 */
UENUM(BlueprintType)
enum class ECurseWeaponType : uint8
{
    CorruptionRail UMETA(DisplayName = "Corruption Rail"),
    FragmentNeedles UMETA(DisplayName = "Fragment Needles"),
    VoidMines UMETA(DisplayName = "Void Mines")
};

/**
 * Spawn pattern for projectile spawning
 * Determines how projectiles are spawned from multiple sockets
 */
UENUM(BlueprintType)
enum class ESpawnPattern : uint8
{
    Single UMETA(DisplayName = "Single"),
    Alternating UMETA(DisplayName = "Alternating"),
    Simultaneous UMETA(DisplayName = "Simultaneous")
};

/**
 * Style rank for combo system
 * DMC-inspired ranking system from D (lowest) to SSS (highest)
 */
UENUM(BlueprintType)
enum class EStyleRank : uint8
{
	D UMETA(DisplayName = "D"),
	C UMETA(DisplayName = "C"),
	B UMETA(DisplayName = "B"),
	A UMETA(DisplayName = "A"),
	S UMETA(DisplayName = "S"),
	SS UMETA(DisplayName = "SS"),
	SSS UMETA(DisplayName = "SSS")
};


/**
 * Damage type classification for combat system
 */
UENUM(BlueprintType)
enum class EDamageType : uint8
{
	Physical UMETA(DisplayName = "Physical"),
	Curse UMETA(DisplayName = "Curse"),
	Environmental UMETA(DisplayName = "Environmental")
};

/**
 * Enemy type classification for AI behavior and difficulty
 * Determines enemy strength, complexity, and currency drops
 */
UENUM(BlueprintType)
enum class EEnemyType : uint8
{
	Fodder UMETA(DisplayName = "Fodder"),
	Elite UMETA(DisplayName = "Elite"),
	MiniBoss UMETA(DisplayName = "Mini Boss"),
	Boss UMETA(DisplayName = "Boss")
};

/**
 * Currency type enumeration
 * Used to identify different types of currency in the progression system
 */
UENUM(BlueprintType)
enum class ECurrencyType : uint8
{
	DataFragments UMETA(DisplayName = "Data Fragments"),
	CurseEssence UMETA(DisplayName = "Curse Essence"),
	CodeKeys UMETA(DisplayName = "Code Keys")
};

/**
 * Configuration for projectile spawn behavior
 * Defines socket-based spawning pattern and direction
 */
USTRUCT(BlueprintType)
struct FProjectileSpawnConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Config")
	TArray<FName> SpawnSockets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Config")
	ESpawnPattern SpawnPattern = ESpawnPattern::Single;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Config")
	FVector SocketOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Config")
	bool bUseCameraDirection = true;
};

/**
 * Configuration data for curse weapon stats
 */
USTRUCT(BlueprintType)
struct FCurseWeaponStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
	float Damage = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
	float FireRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
	float ProjectileSpeed = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
	float MaxRange = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
	int32 PenetrationCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
	bool bEnvironmentalInteraction = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
	float CooldownTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FProjectileSpawnConfig SpawnConfig;
};

/**
 * Configuration data for currency drops from enemies and destructibles
 */
USTRUCT(BlueprintType)
struct FCurrencyDropConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency Drop")
	int32 MinDataFragments = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency Drop")
	int32 MaxDataFragments = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency Drop")
	int32 MinCurseEssence = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency Drop")
	int32 MaxCurseEssence = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency Drop")
	float DropChance = 1.0f;
};

/**
 * Configuration data for enemy stats
 */
USTRUCT(BlueprintType)
struct FEnemyStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Stats")
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Stats")
	float Damage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Stats")
	float MoveSpeed = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Stats")
	float AttackRange = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Stats")
	float AttackCooldown = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Stats")
	float StyleValue = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Stats")
	EEnemyType EnemyType = EEnemyType::Fodder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Stats")
	int32 DataFragmentDrop = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Stats")
	int32 CurseEssenceDrop = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Stats")
	TArray<FName> AttackPatterns;
};

/**
 * Configuration data for style rank thresholds and multipliers
 */
USTRUCT(BlueprintType)
struct FStyleRankData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style Rank")
	float PointsRequired = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style Rank")
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style Rank")
	float DecayRate = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style Rank")
	float VFXIntensity = 1.0f;
};

/**
 * Configuration for style points calculation
 */
USTRUCT(BlueprintType)
struct FStylePointsConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style Points")
	float BasePoints = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style Points")
	float WeaponVarietyBonus = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style Points")
	float TimingBonus = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style Points")
	float EnvironmentalBonus = 15.0f;
};

/**
 * Curse type classification
 * Used to categorize different types of curses in the progression system
 */
UENUM(BlueprintType)
enum class ECurseType : uint8
{
	Skill UMETA(DisplayName = "Skill"),
	Weapon UMETA(DisplayName = "Weapon"),
	BlessedMod UMETA(DisplayName = "Blessed Mod"),
	Passive UMETA(DisplayName = "Passive")
};

/**
 * Frank segment type for story progression
 * Determines whether Frank's story segments are required or optional
 */
UENUM(BlueprintType)
enum class EFrankSegmentType : uint8
{
	Mandatory UMETA(DisplayName = "Mandatory"),
	Optional UMETA(DisplayName = "Optional")
};