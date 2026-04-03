// Fill out your copyright notice in the Description page of Project Settings.


#include "Physics/HairPhysicsComponent.h"

UHairPhysicsComponent::UHairPhysicsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UHairPhysicsComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize hair strands
	InitializeStrands();

	// Calculate fixed delta time from update rate
	if (UpdateRate > 0.0f)
	{
		FixedDeltaTime = 1.0f / UpdateRate;
	}
}

void UHairPhysicsComponent::InitializeStrands()
{
	HairStrands.Empty();
	HairStrands.Reserve(StrandCount);

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Get owner location as starting point for hair roots
	FVector OwnerLocation = Owner->GetActorLocation();
	FRotator OwnerRotation = Owner->GetActorRotation();

	// Create strands distributed around the head
	for (int32 i = 0; i < StrandCount; ++i)
	{
		FHairStrand NewStrand;

		// Calculate root position (distribute evenly in a circle around head)
		float Angle = (2.0f * PI * i) / StrandCount;
		float Radius = 10.0f; // Small radius around head
		FVector LocalOffset = FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.0f);
		FVector WorldOffset = OwnerRotation.RotateVector(LocalOffset);
		NewStrand.RootPosition = OwnerLocation + WorldOffset;

		// Initialize segment positions
		NewStrand.Positions.Reserve(SegmentsPerStrand);
		NewStrand.PreviousPositions.Reserve(SegmentsPerStrand);

		float SegmentLength = StrandLength / SegmentsPerStrand;

		for (int32 j = 0; j < SegmentsPerStrand; ++j)
		{
			// Start with segments hanging downward
			FVector Position = NewStrand.RootPosition + FVector(0.0f, 0.0f, -SegmentLength * j);
			NewStrand.Positions.Add(Position);
			NewStrand.PreviousPositions.Add(Position);
		}

		HairStrands.Add(NewStrand);
	}
}

void UHairPhysicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (HairStrands.Num() == 0)
	{
		return;
	}

	// Update root positions to follow owner
	AActor* Owner = GetOwner();
	if (Owner)
	{
		FVector OwnerLocation = Owner->GetActorLocation();
		FRotator OwnerRotation = Owner->GetActorRotation();

		for (int32 i = 0; i < HairStrands.Num(); ++i)
		{
			float Angle = (2.0f * PI * i) / StrandCount;
			float Radius = 10.0f;
			FVector LocalOffset = FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.0f);
			FVector WorldOffset = OwnerRotation.RotateVector(LocalOffset);
			HairStrands[i].RootPosition = OwnerLocation + WorldOffset;
		}
	}

	// Fixed timestep simulation for stability
	if (bUseFixedTimeStep)
	{
		AccumulatedTime += DeltaTime;

		while (AccumulatedTime >= FixedDeltaTime)
		{
			for (FHairStrand& Strand : HairStrands)
			{
				UpdateStrand(Strand, FixedDeltaTime);
			}
			AccumulatedTime -= FixedDeltaTime;
		}
	}
	else
	{
		// Variable timestep (less stable but lower latency)
		for (FHairStrand& Strand : HairStrands)
		{
			UpdateStrand(Strand, DeltaTime);
		}
	}
}

void UHairPhysicsComponent::UpdateStrand(FHairStrand& Strand, float DeltaTime)
{
	// Verlet integration: x_new = x + (x - x_prev) * damping + acceleration * dt^2
	float DeltaTimeSq = DeltaTime * DeltaTime;

	for (int32 i = 0; i < Strand.Positions.Num(); ++i)
	{
		FVector& Position = Strand.Positions[i];
		FVector& PrevPosition = Strand.PreviousPositions[i];

		// Calculate velocity (implicit from Verlet)
		FVector Velocity = (Position - PrevPosition) * Damping;

		// Store current position for next iteration
		FVector OldPosition = Position;

		// Apply forces
		FVector Acceleration = FVector(0.0f, 0.0f, -Gravity); // Gravity downward
		Acceleration += WindForce; // Wind force

		// Update position using Verlet integration
		Position = Position + Velocity + Acceleration * DeltaTimeSq;

		// Update previous position
		PrevPosition = OldPosition;
	}

	// Apply constraints (distance, collision)
	ApplyConstraints(Strand);

	// Apply collision
	if (bEnableCollision)
	{
		ApplyCollision(Strand);
	}
}

void UHairPhysicsComponent::ApplyConstraints(FHairStrand& Strand)
{
	if (Strand.Positions.Num() == 0)
	{
		return;
	}

	float SegmentLength = StrandLength / SegmentsPerStrand;

	// Constrain first segment to root
	Strand.Positions[0] = Strand.RootPosition;

	// Apply distance constraints between segments
	for (int32 Iteration = 0; Iteration < 3; ++Iteration) // Multiple iterations for stability
	{
		for (int32 i = 0; i < Strand.Positions.Num() - 1; ++i)
		{
			FVector& P1 = Strand.Positions[i];
			FVector& P2 = Strand.Positions[i + 1];

			FVector Delta = P2 - P1;
			float Distance = Delta.Size();

			if (Distance > SMALL_NUMBER)
			{
				// Calculate correction
				float Difference = (Distance - SegmentLength) / Distance;
				FVector Correction = Delta * Difference * Stiffness;

				// Apply correction (only move P2 since P1 is constrained)
				if (i == 0)
				{
					P2 -= Correction;
				}
				else
				{
					P1 += Correction * 0.5f;
					P2 -= Correction * 0.5f;
				}
			}
		}
	}
}

void UHairPhysicsComponent::ApplyCollision(FHairStrand& Strand)
{
	// Simple sphere collision with collision components
	for (UPrimitiveComponent* CollisionComponent : CollisionComponents)
	{
		if (!CollisionComponent)
		{
			continue;
		}

		// Get collision sphere (simplified as sphere around component)
		FVector CollisionCenter = CollisionComponent->GetComponentLocation();
		float CollisionRadius = 20.0f; // Default radius, could be exposed as property

		for (int32 i = 1; i < Strand.Positions.Num(); ++i) // Skip root (i=0)
		{
			FVector& Position = Strand.Positions[i];
			FVector Delta = Position - CollisionCenter;
			float Distance = Delta.Size();

			if (Distance < CollisionRadius)
			{
				// Push point outside collision sphere
				if (Distance > SMALL_NUMBER)
				{
					Position = CollisionCenter + Delta.GetSafeNormal() * CollisionRadius;
				}
			}
		}
	}
}

