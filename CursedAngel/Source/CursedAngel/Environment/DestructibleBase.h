// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "GeometryCollection/GeometryCollectionObject.h"
#include "Chaos/ClusterCreationParameters.h"
#include "Engine/StaticMeshActor.h"
#include "DestructibleBase.generated.h"

class AGeometryCollectionActor;
class UFieldSystemComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDestructibleBroken, AActor*, DamageCauser, float, DestructionWeight);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnStructuralImpact, AActor*, ImpactCauser, float, ImpactSpeed, float, AppliedDamage);

/** A destructible structural piece. Broken arena geometry remains absent for the encounter. */
UCLASS()
class CURSEDANGEL_API ADestructibleBase : public AActor
{
	GENERATED_BODY()
public:
	ADestructibleBase();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Health", meta=(ClampMin="0.0", ToolTip="Current structural integrity. Impact damage is accumulated here; reaching zero breaks this piece.")) float Health = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Health", meta=(ClampMin="0.0")) float MaxHealth = 100.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Destructible|Mesh") UStaticMeshComponent* IntactMeshComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Chunks") TArray<UStaticMesh*> ChunkMeshes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Chunks", meta=(ClampMin="1", ClampMax="50")) int32 ChunkCount = 5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Chunks", meta=(ClampMin="0.0", ClampMax="20000.0")) float ChunkImpulseStrength = 500.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Chunks", meta=(ClampMin="0.0", ToolTip="Zero keeps debris until the level debris budget removes it. Longer-lived debris can be magnetized and thrown.")) float ChunkLifetime = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Chunks", meta=(ToolTip="If set, spawned chunks use this subclass (normally ACombatDebris) to support persistent physical debris and magnetic throws.")) TSubclassOf<AActor> DebrisActorClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Chunks", meta=(ClampMin="0.0", ClampMax="1.0", ToolTip="Fraction of chunk impulse converted to upward lift.")) float DebrisUpwardBias = 0.35f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Rewards", meta=(ClampMin="0")) int32 DataDropAmount = 10;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Rewards") TSubclassOf<AActor> DataPickupClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Rewards") bool bAutoCollectData = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Rewards", meta=(ClampMin="0.0", ClampMax="1000.0")) float AutoCollectRadius = 200.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Feedback") UNiagaraSystem* DestructionVFX = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Feedback") USoundBase* DestructionSFX = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Adaptive", meta=(ClampMin="0.0")) float DestructionProfileWeight = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Structure", meta=(ToolTip="Persistent encounter region identifier. The Environment Combat subsystem remembers this region as broken until the world resets.")) FName ArenaRegionId = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Structure", meta=(ToolTip="When enabled, this piece represents permanent arena geometry such as a bridge, support, catwalk, or cover section.")) bool bPermanentArenaChange = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Impact", meta=(ClampMin="0.0", ToolTip="Impact speeds below this value do not damage structural integrity.")) float MinimumImpactSpeed = 500.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Impact", meta=(ClampMin="0.0", ToolTip="Integrity damage per 1000 cm/s above MinimumImpactSpeed.")) float ImpactDamagePer1000Speed = 20.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Impact", meta=(ClampMin="0.0", ClampMax="10.0", ToolTip="Style multiplier applied to physical impacts; high-style finishers can exceed break thresholds.")) float MaximumStyleImpactMultiplier = 3.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Advanced") bool bUseChaosDestruction = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Advanced") UGeometryCollection* ChaosGeometryCollection = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Advanced", meta=(ClampMin="1000.0", ClampMax="10000000.0")) float DamageThreshold = 500000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Advanced") bool bEnableClustering = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Advanced") EClusterConnectionTypeEnum ClusterConnectionType = EClusterConnectionTypeEnum::Chaos_PointImplicit;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Advanced", meta=(ClampMin="1000.0", ClampMax="10000000.0")) float FieldForceStrength = 1000000.0f;
	UPROPERTY(BlueprintAssignable, Category="Destructible|Events") FOnDestructibleBroken OnDestructibleBroken;
	UPROPERTY(BlueprintAssignable, Category="Destructible|Events") FOnStructuralImpact OnStructuralImpact;

	UFUNCTION(BlueprintCallable, Category="Destructible") virtual void TakeDamageCustom(float DamageAmount, AActor* DamageCauser);
	UFUNCTION(BlueprintCallable, Category="Destructible|Impact", meta=(ToolTip="Apply an impact using measured speed and style multiplier. Use for launchers, tether throws, and enemy-to-structure collisions.")) void ApplyStructuralImpact(float ImpactSpeed, AActor* ImpactCauser, float StyleMultiplier = 1.0f);
	UFUNCTION(BlueprintCallable, Category="Destructible") virtual void DestroyObject();
	UFUNCTION(BlueprintCallable, Category="Destructible") void SpawnChunksPreFractured();
	UFUNCTION(BlueprintCallable, Category="Destructible") void SpawnChunksChaos();
	UFUNCTION(BlueprintCallable, Category="Destructible") void SpawnDataCurrency();
	virtual void Tick(float DeltaTime) override;
protected:
	virtual void BeginPlay() override;
	UPROPERTY() TArray<AStaticMeshActor*> ChunkPool;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Advanced", meta=(ClampMin="5", ClampMax="100")) int32 MaxPoolSize = 20;
	UPROPERTY() TMap<AStaticMeshActor*, float> ActiveChunks;
	UPROPERTY() TObjectPtr<AActor> LastDamageCauser = nullptr;
	bool bHasBeenDestroyed = false;
	AStaticMeshActor* GetPooledChunk();
	void ReturnChunkToPool(AStaticMeshActor* Chunk);
};
