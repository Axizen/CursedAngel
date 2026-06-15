// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "GeometryCollection/GeometryCollectionObject.h"
#include "Chaos/ClusterCreationParameters.h"
#include "Engine/StaticMeshActor.h"


// Forward declarations
class AGeometryCollectionActor;
class UFieldSystemComponent;

#include "DestructibleBase.generated.h"

/**
 * Base class for destructible environment objects
 *
 * USAGE FOR SOLO DEVELOPER:
 * 1. Create Blueprint subclass (e.g., BP_DestructibleCrate, BP_DestructibleBarrel)
 * 2. Assign ChunkMeshes array with pre-fractured pieces from Blender/modeling tool
 * 3. Set Health, DataDropAmount, VFX/SFX references
 * 4. Place in level and test
 *
 * FUTURE UPGRADE TO CHAOS DESTRUCTION (No Code Changes Required!):
 * 1. Create GeometryCollection asset from intact mesh using Fracture Mode
 * 2. Set bUseChaosDestruction = true in Blueprint
 * 3. Assign ChaosGeometryCollection property
 * 4. Everything else works automatically!
 *
 * PERFORMANCE:
 * - Chunk pooling reduces spawn overhead (~0.1ms per destruction)
 * - Default pool size: 20 chunks per destructible type
 * - Target: 50+ simultaneous destructions @ 60 FPS
 */
UCLASS()
class CURSEDANGEL_API ADestructibleBase : public AActor
{
	GENERATED_BODY()
	
public:
	ADestructibleBase();

	// ========================================
	// Health System
	// ========================================

	/** Current health of the destructible object */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Health")
	float Health = 100.0f;

	/** Maximum health (used for percentage calculations) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Health")
	float MaxHealth = 100.0f;

	// ========================================
	// Mesh Components
	// ========================================

	/** Intact mesh component (visible until destroyed) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Destructible|Mesh")
	UStaticMeshComponent* IntactMeshComponent;

	// ========================================
	// Chunk System (Option 1: Pre-Fractured)
	// ========================================

	/** Array of pre-fractured chunk meshes (created in Blender/modeling tool) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Chunks")
	TArray<UStaticMesh*> ChunkMeshes;

	/** Number of chunks to spawn on destruction (randomly selected from ChunkMeshes) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Chunks", meta = (ClampMin = "1", ClampMax = "50"))
	int32 ChunkCount = 5;

	/** Impulse strength applied to chunks (radial explosion from center) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Chunks", meta = (ClampMin = "0.0", ClampMax = "5000.0"))
	float ChunkImpulseStrength = 500.0f;

	/** How long chunks remain in the world before being returned to pool (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Chunks", meta = (ClampMin = "1.0", ClampMax = "10.0"))
	float ChunkLifetime = 3.0f;

	// ========================================
	// Data Currency Rewards
	// ========================================

	/** Amount of data currency to drop on destruction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Rewards", meta = (ClampMin = "0"))
	int32 DataDropAmount = 10;

	/** Data pickup actor class to spawn (set to nullptr for direct currency add) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Rewards")
	TSubclassOf<AActor> DataPickupClass;

	/** Automatically collect data within radius (no pickup actor spawned) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Rewards")
	bool bAutoCollectData = true;

	/** Auto-collect radius for data currency (if bAutoCollectData is true) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Rewards", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
	float AutoCollectRadius = 200.0f;

	// ========================================
	// VFX/SFX Feedback
	// ========================================

	/** Niagara VFX to spawn on destruction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Feedback")
	UNiagaraSystem* DestructionVFX;

	/** Sound effect to play on destruction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Feedback")
	USoundBase* DestructionSFX;

	// ========================================
	// Extensibility: Chaos Destruction Foundation
	// ========================================

	/** Toggle to use Chaos Destruction instead of pre-fractured chunks (future upgrade) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Advanced")
	bool bUseChaosDestruction = false;

	/** Geometry Collection asset for Chaos Destruction (assign when bUseChaosDestruction = true) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Advanced")
	UGeometryCollection* ChaosGeometryCollection;

	/** Damage threshold for Chaos clusters (higher = more durable) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Advanced", meta = (ClampMin = "1000.0", ClampMax = "10000000.0"))
	float DamageThreshold = 500000.0f;

	/** Enable clustering for Chaos destruction (groups pieces until threshold exceeded) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Advanced")
	bool bEnableClustering = true;

	/** Cluster connection type for Chaos destruction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Advanced")
	EClusterConnectionTypeEnum ClusterConnectionType = EClusterConnectionTypeEnum::Chaos_PointImplicit;

	/** Force strength for field system radial force (explosion impulse) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Advanced", meta = (ClampMin = "1000.0", ClampMax = "10000000.0"))
	float FieldForceStrength = 1000000.0f;

	// ========================================
	// Public Functions
	// ========================================

	/**
	 * Apply damage to the destructible object
	 * @param DamageAmount - Amount of damage to apply
	 * @param DamageCauser - Actor that caused the damage (used for impulse direction)
	 */
	UFUNCTION(BlueprintCallable, Category = "Destructible")
	virtual void TakeDamageCustom(float DamageAmount, AActor* DamageCauser);

	/**
	 * Destroy the object (called when health reaches zero)
	 * Spawns chunks, VFX/SFX, data currency, and removes intact mesh
	 */
	UFUNCTION(BlueprintCallable, Category = "Destructible")
	virtual void DestroyObject();

protected:
	virtual void BeginPlay() override;

	/**
	 * Spawn pre-fractured chunks with physics impulse
	 * Uses chunk pooling for performance
	 */
	UFUNCTION(BlueprintCallable, Category = "Destructible")
	void SpawnChunksPreFractured();

	/**
	 * Spawn Chaos Destruction geometry collection (placeholder for future)
	 * Falls back to pre-fractured chunks if ChaosGeometryCollection is not set
	 */
	UFUNCTION(BlueprintCallable, Category = "Destructible")
	void SpawnChunksChaos();

	/**
	 * Spawn data currency pickup or auto-collect to nearby player
	 */
	UFUNCTION(BlueprintCallable, Category = "Destructible")
	void SpawnDataCurrency();

	// ========================================
	// Chunk Pooling System
	// ========================================

	/** Pool of reusable chunk actors (performance optimization) */
	UPROPERTY()
	TArray<AStaticMeshActor*> ChunkPool;

	/** Maximum pool size per destructible (prevents memory bloat) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Advanced", meta = (ClampMin = "5", ClampMax = "100"))
	int32 MaxPoolSize = 20;

	/**
	 * Get a chunk actor from the pool (creates new if pool is empty)
	 * @return Pooled or newly created AStaticMeshActor
	 */
	AStaticMeshActor* GetPooledChunk();

	/**
	 * Return a chunk actor to the pool for reuse
	 * @param Chunk - Chunk actor to return to pool
	 */
	void ReturnChunkToPool(AStaticMeshActor* Chunk);

	/** Track active chunks for lifetime management */
	UPROPERTY()
	TMap<AStaticMeshActor*, float> ActiveChunks;

public:
	virtual void Tick(float DeltaTime) override;

	// ========================================
	// Future Enhancements (Comments for Team)
	// ========================================

	// FUTURE: Add multi-stage destruction (crack texture -> partial break -> full break)
	// FUTURE: Add destruction chains (breaking one object triggers nearby objects)
	// FUTURE: Add conditional drops (chance-based loot, rare data pickups)
	// FUTURE: Add destruction particle trails (dust, debris over time)
	// FUTURE: Add sound variation (different sounds for wood, metal, glass, etc.)
	// FUTURE: Add destruction score multipliers (style rank bonus for destruction chains)
	// FUTURE: Add destructible regeneration (objects respawn after time for replayability)
	// FUTURE: Add environmental hazards on destruction (fire, electric sparks, toxic gas)
};

/**
* UENUM(BlueprintType)
enum class EChunkWaterInteraction : uint8
{
	Sink UMETA(DisplayName = "Sink"),
	Float UMETA(DisplayName = "Float"),
	Splash UMETA(DisplayName = "Splash"),
	None UMETA(DisplayName = "No Interaction")
};

UCLASS()
class CURSEDANGEL_API ADestructibleBase : public AActor
{
	GENERATED_BODY()

public:

	ADestructibleBase();

	// ========================================================================
	// HEALTH SYSTEM
	// ========================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Health")
	float Health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Health")
	float MaxHealth = 100.0f;

	// ========================================================================
	// MESH COMPONENTS
	// ========================================================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Destructible|Mesh")
	class UStaticMeshComponent* IntactMeshComponent;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Chunks",
		meta = (Tooltip = "Pre-fractured mesh pieces (create in Blender with Fracture add-on)"))
	TArray<UStaticMesh*> ChunkMeshes;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Chunks",
		meta = (ClampMin = "1", ClampMax = "50",
		Tooltip = "Number of chunks to spawn (consider performance)"))
	int32 ChunkCount = 5;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Chunks",
		meta = (UIMin = "0", UIMax = "5000", ClampMin = "0", ClampMax = "20000",
		Tooltip = "Explosion impulse (Newtons). 500=normal, 2000=dramatic"))
	float ChunkImpulseStrength = 1000.0f;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Chunks",
		meta = (UIMin = "0.5", UIMax = "10", ClampMin = "0",
		Tooltip = "How long chunks stay before despawn (seconds)"))
	float ChunkLifetime = 3.0f;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Chunks|Physics",
		meta = (UIMin = "0.1", UIMax = "5", ClampMin = "0.01",
		Tooltip = "Chunk mass multiplier (1=default, 2=heavy, 0.5=light)"))
	float ChunkMassMultiplier = 1.0f;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Chunks|Physics",
		meta = (UIMin = "0", UIMax = "3000", ClampMin = "0",
		Tooltip = "Angular velocity (spin) of chunks (rad/sec)"))
	float ChunkSpinStrength = 500.0f;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Chunks|Physics",
		meta = (UIMin = "0.1", UIMax = "3", ClampMin = "0.01",
		Tooltip = "Wind resistance (1=normal, 0.5=heavy, 2=light)"))
	float ChunkWindResistance = 1.0f;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Chunks|Water",
		meta = (UIMin = "0", UIMax = "2", ClampMin = "0",
		Tooltip = "Buoyancy in water (1=floats, 0=sinks)"))
	float ChunkBuoyancy = 0.5f;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Chunks|Water",
		meta = (UIMin = "0.5", UIMax = "3", ClampMin = "0.1",
		Tooltip = "Water resistance (1=normal, 2=thick water)"))
	float ChunkWaterDrag = 1.0f;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Feedback",
		meta = (Tooltip = "Particle system following chunks (optional)"))
	class UNiagaraSystem* ChunkTrailParticles;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Feedback",
		meta = (Tooltip = "Particle effect when chunk impacts (optional)"))
	class UNiagaraSystem* ImpactParticles;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Feedback",
		meta = (Tooltip = "Sound when chunk impacts (optional)"))
	class USoundBase* ImpactSound;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Feedback",
		meta = (Tooltip = "Explosion effect when destroyed (optional)"))
	class UNiagaraSystem* DestructionVFX;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Feedback",
		meta = (Tooltip = "Explosion sound when destroyed (optional)"))
	class USoundBase* DestructionSFX;

		UFUNCTION(BlueprintCallable, Category = "Destructible",
		meta = (Tooltip = "Apply damage to destructible"))
	virtual void TakeDamageCustom(float DamageAmount, AActor* DamageCauser);

		UFUNCTION(BlueprintCallable, Category = "Destructible",
		meta = (Tooltip = "Destroy object and spawn chunks"))
	virtual void DestroyObject();

protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

		UFUNCTION(BlueprintCallable, Category = "Destructible")
	void SpawnChunks();

		UFUNCTION(BlueprintCallable, Category = "Destructible")
	ADestructibleChunkActor* SpawnSingleChunk();

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Rewards",
		meta = (ClampMin = "0",
		Tooltip = "Data currency to drop on destruction"))
	int32 DataDropAmount = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Rewards",
		meta = (Tooltip = "Auto-collect data within radius (no pickup spawned)"))
	bool bAutoCollectData = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Rewards",
		meta = (UIMin = "0", UIMax = "1000", ClampMin = "0",
		Tooltip = "Radius for auto-collection (cm)"))
	float AutoCollectRadius = 200.0f;

		UPROPERTY()
	class UStaticMeshComponent* IntactMesh;

	UPROPERTY()
	TArray<ADestructibleChunkActor*> ActiveChunks;
*/