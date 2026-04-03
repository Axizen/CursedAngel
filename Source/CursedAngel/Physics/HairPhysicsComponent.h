// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HairPhysicsComponent.generated.h"

/**
 * Verlet integration-based hair/furHairPhysicsComponent physics simulation
 * Solo Dev Note: Works for both base and Cursed Angel transformation automatically
 * Physics updates based on current mesh, no special handling needed for transformation
 *
 * PERFORMANCE: ~0.5ms per character (10 strands, 5 segments)
 * LOD: Reduce strand count at distance (future)
 *
 * FUTURE ENHANCEMENTS:
 * - Add LOD system (reduce strands at distance for performance)
 * - Add wind zones (environmental wind influence from volume actors)
 * - Add hair cutting/damage (dynamic strand removal on damage)
 * - Add hair wetness (water interaction, increased weight, clumping)
 * - Add hair shader integration (pass strand data to material for rendering)
 * - Add hair-to-hair collision (strand-to-strand physics)
 * - Add animated root points (follow skeletal mesh bones)
 * - Add hair styling (preset configurations, ponytails, braids)
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CURSEDANGEL_API UHairPhysicsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHairPhysicsComponent();

	// Hair simulation properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair Physics")
	int32 StrandCount = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair Physics")
	int32 SegmentsPerStrand = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair Physics")
	float StrandLength = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair Physics")
	float Stiffness = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair Physics")
	float Damping = 0.9f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair Physics")
	float Gravity = 980.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair Physics")
	FVector WindForce = FVector::ZeroVector;

	// Collision
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair Physics")
	bool bEnableCollision = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair Physics")
	TArray<UPrimitiveComponent*> CollisionComponents;

	// Performance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair Physics")
	float UpdateRate = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair Physics")
	bool bUseFixedTimeStep = true;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

	struct FHairStrand
	{
		TArray<FVector> Positions;
		TArray<FVector> PreviousPositions;
		FVector RootPosition;
	};

	TArray<FHairStrand> HairStrands;

	void InitializeStrands();
	void UpdateStrand(FHairStrand& Strand, float DeltaTime);
	void ApplyConstraints(FHairStrand& Strand);
	void ApplyCollision(FHairStrand& Strand);

	float AccumulatedTime = 0.0f;
	float FixedDeltaTime = 1.0f / 60.0f;
};
