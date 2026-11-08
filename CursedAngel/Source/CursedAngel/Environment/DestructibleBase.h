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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStructuralDamageStageChanged, EStructuralDamageStage, NewStage, EStructuralDamageStage, PreviousStage);

UENUM(BlueprintType)
enum class EStructuralDamageStage : uint8
{
	Intact UMETA(DisplayName="Intact"),
	Damaged UMETA(DisplayName="Damaged"),
	Critical UMETA(DisplayName="Critical"),
	Destroyed UMETA(DisplayName="Destroyed")
};

/** A destructible structural piece. Broken arena geometry remains absent for the encounter. */
UCLASS()
class CURSEDANGEL_API ADestructibleBase : public AActor
{
	GENERATED_BODY()
public:
	ADestructibleBase();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Health", meta=(ClampMin="0.0", ToolTip="Current structural integrity. Impact damage is accumulated here; reaching zero breaks this piece.")) float Health = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Health", meta=(ClampMin="0.0")) float MaxHealth = 100.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Destructible|Health") EStructuralDamageStage DamageStage = EStructuralDamageStage::Intact;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Destructible|Mesh") UStaticMeshComponent* IntactMeshComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Chunks") TArray<UStaticMesh*> ChunkMeshes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Chunks", meta=(ClampMin="1", ClampMax="50")) int32 ChunkCount = 5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Chunks", meta=(ClampMin="0.0", ClampMax="20000.0")) float ChunkImpulseStrength = 500.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Chunks", meta=(ClampMin="0.0", ToolTip="Zero keeps debris until the level debris budget removes it.")) float ChunkLifetime = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Chunks") TSubclassOf<AActor> DebrisActorClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Chunks", meta=(ClampMin="0.0", ClampMax="1.0")) float DebrisUpwardBias = 0.35f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Rewards", meta=(ClampMin="0")) int32 DataDropAmount = 10;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Rewards") TSubclassOf<AActor> DataPickupClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Rewards") bool bAutoCollectData = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Rewards", meta=(ClampMin="0.0", ClampMax="1000.0")) float AutoCollectRadius = 200.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Feedback") UNiagaraSystem* DestructionVFX = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Feedback") USoundBase* DestructionSFX = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Adaptive", meta=(ClampMin="0.0")) float DestructionProfileWeight = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Structure") FName ArenaRegionId = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Structure") bool bPermanentArenaChange = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Impact", meta=(ClampMin="0.0")) float MinimumImpactSpeed = 500.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Impact", meta=(ClampMin="0.0")) float ImpactDamagePer1000Speed = 20.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Impact", meta=(ClampMin="0.0", ClampMax="10.0")) float MaximumStyleImpactMultiplier = 3.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Damage Stages", meta=(ClampMin="0.0", ClampMax="1.0", ToolTip="Integrity fraction at or below which the piece enters the damaged state. Bind OnDamageStageChanged for cracks/dents.")) float DamagedHealthFraction = 0.66f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Damage Stages", meta=(ClampMin="0.0", ClampMax="1.0", ToolTip="Integrity fraction at or below which the structure is critical and can trigger warning effects.")) float CriticalHealthFraction = 0.25f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Advanced") bool bUseChaosDestruction = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Advanced") UGeometryCollection* ChaosGeometryCollection = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Advanced", meta=(ClampMin="1000.0", ClampMax="10000000.0")) float DamageThreshold = 500000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Advanced") bool bEnableClustering = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Advanced") EClusterConnectionTypeEnum ClusterConnectionType = EClusterConnectionTypeEnum::Chaos_PointImplicit;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Advanced", meta=(ClampMin="1000.0", ClampMax="10000000.0")) float FieldForceStrength = 1000000.0f;
	UPROPERTY(BlueprintAssignable, Category="Destructible|Events") FOnDestructibleBroken OnDestructibleBroken;
	UPROPERTY(BlueprintAssignable, Category="Destructible|Events") FOnStructuralImpact OnStructuralImpact;
	UPROPERTY(BlueprintAssignable, Category="Destructible|Events") FOnStructuralDamageStageChanged OnDamageStageChanged;

	UFUNCTION(BlueprintCallable, Category="Destructible") virtual void TakeDamageCustom(float DamageAmount, AActor* DamageCauser);
	UFUNCTION(BlueprintCallable, Category="Destructible|Impact") void ApplyStructuralImpact(float ImpactSpeed, AActor* ImpactCauser, float StyleMultiplier = 1.0f);
	UFUNCTION(BlueprintCallable, Category="Destructible") virtual void DestroyObject();
	UFUNCTION(BlueprintCallable, Category="Destructible") void SpawnChunksPreFractured();
	UFUNCTION(BlueprintCallable, Category="Destructible") void SpawnChunksChaos();
	UFUNCTION(BlueprintCallable, Category="Destructible") void SpawnDataCurrency();
	virtual void Tick(float DeltaTime) override;
protected:
	virtual void BeginPlay() override;
	void UpdateDamageStage();
	UPROPERTY() TArray<AStaticMeshActor*> ChunkPool;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Destructible|Advanced", meta=(ClampMin="5", ClampMax="100")) int32 MaxPoolSize = 20;
	UPROPERTY() TMap<AStaticMeshActor*, float> ActiveChunks;
	UPROPERTY() TObjectPtr<AActor> LastDamageCauser = nullptr;
	bool bHasBeenDestroyed = false;
	AStaticMeshActor* GetPooledChunk();
	void ReturnChunkToPool(AStaticMeshActor* Chunk);
};
