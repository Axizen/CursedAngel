// Fill out your copyright notice in the Description page of Project Settings.


#include "Physics/ClothPhysicsComponent.h"

UClothPhysicsComponent::UClothPhysicsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UClothPhysicsComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize cloth grid
	InitializeClothGrid();

	// Calculate fixed delta time from update rate
	if (UpdateRate > 0.0f)
	{
		FixedDeltaTime = 1.0f / UpdateRate;
	}
}

void UClothPhysicsComponent::InitializeClothGrid()
{
	ClothGrid.Empty();
	ClothGrid.Reserve(GridWidth);

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	FVector OwnerLocation = Owner->GetActorLocation();
	FRotator OwnerRotation = Owner->GetActorRotation();

	// Calculate cell size
	float CellWidth = ClothWidth / (GridWidth - 1);
	float CellHeight = ClothHeight / (GridHeight - 1);

	// Create grid
	for (int32 x = 0; x < GridWidth; ++x)
	{
		TArray<FClothVertex> Column;
		Column.Reserve(GridHeight);

		for (int32 y = 0; y < GridHeight; ++y)
		{
			FClothVertex Vertex;

			// Calculate local position
			FVector LocalPos = FVector(
				x * CellWidth - ClothWidth * 0.5f,
				0.0f,
				y * CellHeight - ClothHeight * 0.5f
			);

			// Transform to world space
			Vertex.Position = OwnerLocation + OwnerRotation.RotateVector(LocalPos);
			Vertex.PreviousPosition = Vertex.Position;
			Vertex.bIsAnchored = false;
			Vertex.bIsDestroyed = false;

			// Check if this vertex is an anchor point
			FVector2D NormalizedPos = FVector2D(
				static_cast<float>(x) / (GridWidth - 1),
				static_cast<float>(y) / (GridHeight - 1)
			);

			for (const FVector2D& AnchorPoint : AnchorPoints)
			{
				if (FVector2D::Distance(NormalizedPos, AnchorPoint) < 0.1f)
				{
					Vertex.bIsAnchored = true;
					break;
				}
			}

			Column.Add(Vertex);
		}

		ClothGrid.Add(Column);
	}
}

void UClothPhysicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ClothGrid.Num() == 0)
	{
		return;
	}

	// Fixed timestep simulation for stability
	if (bUseFixedTimeStep)
	{
		AccumulatedTime += DeltaTime;

		while (AccumulatedTime >= FixedDeltaTime)
		{
			UpdateClothGrid(FixedDeltaTime);
			AccumulatedTime -= FixedDeltaTime;
		}
	}
	else
	{
		// Variable timestep (less stable but lower latency)
		UpdateClothGrid(DeltaTime);
	}
}

void UClothPhysicsComponent::UpdateClothGrid(float DeltaTime)
{
	// Verlet integration for all non-anchored vertices
	float DeltaTimeSq = DeltaTime * DeltaTime;

	for (int32 x = 0; x < ClothGrid.Num(); ++x)
	{
		for (int32 y = 0; y < ClothGrid[x].Num(); ++y)
		{
			FClothVertex& Vertex = ClothGrid[x][y];

			// Skip anchored or destroyed vertices
			if (Vertex.bIsAnchored || Vertex.bIsDestroyed)
			{
				continue;
			}

			// Calculate velocity (implicit from Verlet)
			FVector Velocity = (Vertex.Position - Vertex.PreviousPosition) * Damping;

			// Store current position for next iteration
			FVector OldPosition = Vertex.Position;

			// Apply forces
			FVector Acceleration = FVector(0.0f, 0.0f, -Gravity); // Gravity downward
			Acceleration += WindForce; // Wind force

			// Update position using Verlet integration
			Vertex.Position = Vertex.Position + Velocity + Acceleration * DeltaTimeSq;

			// Update previous position
			Vertex.PreviousPosition = OldPosition;
		}
	}

	// Apply constraints (distance between connected vertices)
	ApplyConstraints();

	// Apply collision
	if (bEnableCollision)
	{
		ApplyCollision();
	}
}

void UClothPhysicsComponent::ApplyConstraints()
{
	if (ClothGrid.Num() == 0)
	{
		return;
	}

	float CellWidth = ClothWidth / (GridWidth - 1);
	float CellHeight = ClothHeight / (GridHeight - 1);

	// Multiple iterations for stability
	for (int32 Iteration = 0; Iteration < 3; ++Iteration)
	{
		// Horizontal constraints
		for (int32 x = 0; x < ClothGrid.Num() - 1; ++x)
		{
			for (int32 y = 0; y < ClothGrid[x].Num(); ++y)
			{
				FClothVertex& V1 = ClothGrid[x][y];
				FClothVertex& V2 = ClothGrid[x + 1][y];

				// Skip if either vertex is destroyed
				if (V1.bIsDestroyed || V2.bIsDestroyed)
				{
					continue;
				}

				FVector Delta = V2.Position - V1.Position;
				float Distance = Delta.Size();

				if (Distance > SMALL_NUMBER)
				{
					// Calculate correction
					float Difference = (Distance - CellWidth) / Distance;
					FVector Correction = Delta * Difference * Stiffness;

					// Apply correction (respect anchored vertices)
					if (!V1.bIsAnchored && !V2.bIsAnchored)
					{
						V1.Position += Correction * 0.5f;
						V2.Position -= Correction * 0.5f;
					}
					else if (!V1.bIsAnchored)
					{
						V1.Position += Correction;
					}
					else if (!V2.bIsAnchored)
					{
						V2.Position -= Correction;
					}
				}
			}
		}

		// Vertical constraints
		for (int32 x = 0; x < ClothGrid.Num(); ++x)
		{
			for (int32 y = 0; y < ClothGrid[x].Num() - 1; ++y)
			{
				FClothVertex& V1 = ClothGrid[x][y];
				FClothVertex& V2 = ClothGrid[x][y + 1];

				// Skip if either vertex is destroyed
				if (V1.bIsDestroyed || V2.bIsDestroyed)
				{
					continue;
				}

				FVector Delta = V2.Position - V1.Position;
				float Distance = Delta.Size();

				if (Distance > SMALL_NUMBER)
				{
					// Calculate correction
					float Difference = (Distance - CellHeight) / Distance;
					FVector Correction = Delta * Difference * Stiffness;

					// Apply correction (respect anchored vertices)
					if (!V1.bIsAnchored && !V2.bIsAnchored)
					{
						V1.Position += Correction * 0.5f;
						V2.Position -= Correction * 0.5f;
					}
					else if (!V1.bIsAnchored)
					{
						V1.Position += Correction;
					}
					else if (!V2.bIsAnchored)
					{
						V2.Position -= Correction;
					}
				}
			}
		}
	}
}

void UClothPhysicsComponent::ApplyCollision()
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
		float CollisionRadius = 30.0f; // Default radius, could be exposed as property

		for (int32 x = 0; x < ClothGrid.Num(); ++x)
		{
			for (int32 y = 0; y < ClothGrid[x].Num(); ++y)
			{
				FClothVertex& Vertex = ClothGrid[x][y];

				// Skip anchored or destroyed vertices
				if (Vertex.bIsAnchored || Vertex.bIsDestroyed)
				{
					continue;
				}

				FVector Delta = Vertex.Position - CollisionCenter;
				float Distance = Delta.Size();

				if (Distance < CollisionRadius)
				{
					// Push point outside collision sphere
					if (Distance > SMALL_NUMBER)
					{
						Vertex.Position = CollisionCenter + Delta.GetSafeNormal() * CollisionRadius;
					}
				}
			}
		}
	}
}