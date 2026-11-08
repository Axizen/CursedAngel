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

/** Destructible prop with optional Chaos fracture or pooled pre-fractured debris. */
UCLASS()
class CURSEDANGEL_API ADestructibleBase : public AActor
{
	GENERATED_BODY()
public:
	ADestructibleBase();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Health", meta=(ClampMin="0.0", ToolTip="Current durability. Damage reaching zero triggers the break event once.")) float Health = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Health", meta=(ClampMin="0.0", ToolTip="Durability restored at BeginPlay.")) float MaxHealth = 100.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Destructible|Mesh") UStaticMeshComponent* IntactMeshComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Chunks", meta=(ToolTip="Pre-fractured mesh pieces used when Chaos fracture is disabled or unavailable.")) TArray<UStaticMesh*> ChunkMeshes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Chunks", meta=(ClampMin="1", ClampMax="50", ToolTip="Number of pooled debris pieces spawned when this prop breaks.")) int32 ChunkCount = 5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Chunks", meta=(ClampMin="0.0", ClampMax="20000.0", ToolTip="Impulse applied to debris, producing the arcade-like burst of a prop break.")) float ChunkImpulseStrength = 500.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Chunks", meta=(ClampMin="1.0", ClampMax="10.0", ToolTip="Seconds debris remains active before returning to its pool.")) float ChunkLifetime = 3.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Rewards", meta=(ClampMin="0")) int32 DataDropAmount = 10;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Rewards") TSubclassOf<AActor> DataPickupClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Rewards") bool bAutoCollectData = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Rewards", meta=(ClampMin="0.0", ClampMax="1000.0")) float AutoCollectRadius = 200.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Feedback") UNiagaraSystem* DestructionVFX = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Feedback") USoundBase* DestructionSFX = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Adaptive", meta=(ClampMin="0.0", ToolTip="Relative gameplay weight added to the adaptive player profile when this prop is destroyed. Set larger for major environmental destruction.")) float DestructionProfileWeight = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Advanced", meta=(ToolTip="Use Chaos Geometry Collection fracture when configured; otherwise pre-fractured chunks are used.")) bool bUseChaosDestruction = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Advanced") UGeometryCollection* ChaosGeometryCollection = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Advanced", meta=(ClampMin="1000.0", ClampMax="10000000.0")) float DamageThreshold = 500000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Advanced") bool bEnableClustering = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Advanced") EClusterConnectionTypeEnum ClusterConnectionType = EClusterConnectionTypeEnum::Chaos_PointImplicit;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Advanced", meta=(ClampMin="1000.0", ClampMax="10000000.0")) float FieldForceStrength = 1000000.0f;
	UPROPERTY(BlueprintAssignable, Category="Destructible|Events", meta=(ToolTip="Broadcast once after break, before the actor is cleaned up. DamageCauser enables player/weapon attribution.")) FOnDestructibleBroken OnDestructibleBroken;

	UFUNCTION(BlueprintCallable, Category="Destructible") virtual void TakeDamageCustom(float DamageAmount, AActor* DamageCauser);
	UFUNCTION(BlueprintCallable, Category="Destructible") virtual void DestroyObject();
	UFUNCTION(BlueprintCallable, Category="Destructible") void SpawnChunksPreFractured();
	UFUNCTION(BlueprintCallable, Category="Destructible") void SpawnChunksChaos();
	UFUNCTION(BlueprintCallable, Category="Destructible") void SpawnDataCurrency();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	UPROPERTY() TArray<AStaticMeshActor*> ChunkPool;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Advanced", meta=(ClampMin="5", ClampMax="100", ToolTip="Maximum pooled debris actors retained by this prop.")) int32 MaxPoolSize = 20;
	UPROPERTY() TMap<AStaticMeshActor*, float> ActiveChunks;
	UPROPERTY() TObjectPtr<AActor> LastDamageCauser = nullptr;
	bool bHasBeenDestroyed = false;
	AStaticMeshActor* GetPooledChunk();
	void ReturnChunkToPool(AStaticMeshActor* Chunk);
};
