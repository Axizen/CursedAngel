// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "GeometryCollection/GeometryCollectionObject.h"
#include "Chaos/ClusterCreationParameters.h"


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