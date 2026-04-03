// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Character.h"
#include "SoftBodyPhysicsComponent.generated.h"

/**
 * Spring-mass system for soft-body physics (breast physics, etc.)
 * Solo Dev Note: Works for both base and Cursed Angel transformation automatically
 * Rest position updates based on current skeleton, no special handling needed
 *
 * PERFORMANCE: ~0.2ms per character (2 soft bodies)
 *
 * FUTURE ENHANCEMENTS FOR SOFT-BODY PHYSICS:
 * - Add multi-point soft body (simulate multiple connected masses for complex shapes)
 * - Add soft-body collision with environment (bounce off walls, floors, obstacles)
 * - Add soft-body deformation on impact (squash/stretch based on collision force)
 * - Add soft-body jiggle physics (secondary motion, inertia-based movement)
 * - Add soft-body muscle simulation (flex on actions like attacks, jumps)
 * - Add soft-body fat simulation (weight-based movement, realistic mass distribution)
 * - Add soft-body clothing interaction (cloth pushes soft-body, two-way interaction)
 * - Add soft-body damage (bruising, swelling visual effects on damage)
 * - Add soft-body animation blending (blend physics with animation system)
 * - Add soft-body constraints (limit maximum displacement per axis)
 * - Add soft-body LOD (reduce simulation quality at distance)
 * - Add soft-body wind influence (environmental wind affects movement)
 * - Add soft-body buoyancy (water interaction, floating behavior)
 * - Add soft-body temperature effects (hot/cold affects stiffness)
 *
 * IMPLEMENTATION NOTES:
 * - Single mass point for prototype (future: multi-point for complex shapes)
 * - Spring force brings mass back to rest position (Hooke's law: F = -k * x)
 * - Character movement/rotation influences soft-body (inertia effects)
 * - MaxDisplacement clamps extreme movements
 */


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CURSEDANGEL_API USoftBodyPhysicsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USoftBodyPhysicsComponent();

	// Soft-body simulation properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Body Physics")
	float Mass = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Body Physics")
	float SpringStiffness = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Body Physics")
	float Damping = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Body Physics")
	float Gravity = 980.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Body Physics")
	FVector RestPosition = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Body Physics")
	float MaxDisplacement = 10.0f;

	// Character movement influence
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Body Physics")
	float MovementInfluence = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Body Physics")
	float RotationInfluence = 0.5f;

	// Performance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Body Physics")
	float UpdateRate = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soft Body Physics")
	bool bUseFixedTimeStep = true;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

	FVector CurrentPosition;
	FVector PreviousPosition;
	FVector Velocity;

	void UpdateSoftBody(float DeltaTime);
	void ApplySpringForce();
	void ApplyCharacterMovement();

	UPROPERTY()
	ACharacter* OwnerCharacter;

	FVector PreviousCharacterLocation;
	FRotator PreviousCharacterRotation;

	float AccumulatedTime = 0.0f;
	float FixedDeltaTime = 1.0f / 60.0f;

	// Future multi-point soft body (commented for future implementation)
	// TArray<FVector> MassPoints;
	// TArray<FVector> MassVelocities;
	// TArray<float> MassWeights;
	// TArray<FVector> MassRestPositions;
	// 
	// void InitializeMultiPointSoftBody();
	// void UpdateMultiPointSoftBody(float DeltaTime);
	// void ApplyMultiPointConstraints();

};
