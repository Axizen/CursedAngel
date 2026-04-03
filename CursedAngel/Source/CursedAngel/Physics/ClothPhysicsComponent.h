// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ClothPhysicsComponent.generated.h"

/**
 * Grid-based cloth physics simulation
 * Solo Dev Note: Works for both base and Cursed Angel transformation automatically
 * Anchor points update based on current skeleton, no special handling needed
 *
 * PERFORMANCE: ~0.8ms per character (5x5 grid)
 * LOD: Reduce grid resolution at distance (future)
 *
 * FUTURE ENHANCEMENTS FOR CLOTH DESTRUCTION:
 * - Add vertex removal on damage (tear cloth dynamically when taking damage)
 * - Add burn/melt effects (remove vertices in radius, visual deterioration)
 * - Add cut lines (slice cloth along damage vector, create clean cuts)
 * - Add cloth regeneration (heal torn cloth over time or via ability)
 * - Add cloth state persistence (save torn state across level transitions)
 * - Integration with damage system (receive damage events from character)
 * - Add cloth tearing sound effects (ripping, tearing audio)
 * - Add frayed edges (spawn particle effects on torn edges)
 * - Add cloth physics on detached pieces (torn cloth falls realistically)
 * - Add cloth repair animations (visual stitching, healing effects)
 *
 * FUTURE PERFORMANCE ENHANCEMENTS:
 * - Add LOD system (reduce grid resolution at distance)
 * - Add cloth sleep state (stop simulation when not moving)
 * - Add spatial hashing for collision (faster collision detection)
 * - Add multi-threading support (simulate multiple cloths in parallel)
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CURSEDANGEL_API UClothPhysicsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UClothPhysicsComponent();

	// Cloth simulation properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloth Physics")
	int32 GridWidth = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloth Physics")
	int32 GridHeight = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloth Physics")
	float ClothWidth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloth Physics")
	float ClothHeight = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloth Physics")
	float Stiffness = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloth Physics")
	float Damping = 0.95f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloth Physics")
	float Gravity = 980.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloth Physics")
	FVector WindForce = FVector::ZeroVector;

	// Collision
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloth Physics")
	bool bEnableCollision = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloth Physics")
	TArray<UPrimitiveComponent*> CollisionComponents;

	// Anchor points (normalized 0-1 coordinates in grid space)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloth Physics")
	TArray<FVector2D> AnchorPoints;

	// Performance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloth Physics")
	float UpdateRate = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloth Physics")
	bool bUseFixedTimeStep = true;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

	struct FClothVertex
	{
		FVector Position;
		FVector PreviousPosition;
		bool bIsAnchored;
		bool bIsDestroyed; // Future: For cloth destruction
	};

	TArray<TArray<FClothVertex>> ClothGrid;

	void InitializeClothGrid();
	void UpdateClothGrid(float DeltaTime);
	void ApplyConstraints();
	void ApplyCollision();

	float AccumulatedTime = 0.0f;
	float FixedDeltaTime = 1.0f / 60.0f;

	// Future cloth destruction functions (commented for future implementation)
	// UFUNCTION(BlueprintCallable, Category = "Cloth Physics|Destruction")
	// void DamageClothVertex(int32 X, int32 Y, float Damage);

	// UFUNCTION(BlueprintCallable, Category = "Cloth Physics|Destruction")
	// void TearCloth(FVector StartPoint, FVector EndPoint);

	// UFUNCTION(BlueprintCallable, Category = "Cloth Physics|Destruction")
	// void BurnCloth(FVector Center, float Radius);

	// UFUNCTION(BlueprintCallable, Category = "Cloth Physics|Destruction")
	// void RegenerateCloth(float DeltaTime);	
};
