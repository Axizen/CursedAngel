// Fill out your copyright notice in the Description page of Project Settings.


#include "Physics/SoftBodyPhysicsComponent.h"

USoftBodyPhysicsComponent::USoftBodyPhysicsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void USoftBodyPhysicsComponent::BeginPlay()
{
	Super::BeginPlay();

	// Get owner character
	OwnerCharacter = Cast<ACharacter>(GetOwner());

	if (OwnerCharacter)
	{
		// Initialize rest position relative to character
		FVector OwnerLocation = OwnerCharacter->GetActorLocation();
		CurrentPosition = OwnerLocation + RestPosition;
		PreviousPosition = CurrentPosition;
		Velocity = FVector::ZeroVector;

		// Store initial character state
		PreviousCharacterLocation = OwnerLocation;
		PreviousCharacterRotation = OwnerCharacter->GetActorRotation();
	}

	// Calculate fixed delta time from update rate
	if (UpdateRate > 0.0f)
	{
		FixedDeltaTime = 1.0f / UpdateRate;
	}
}

void USoftBodyPhysicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!OwnerCharacter)
	{
		return;
	}

	// Fixed timestep simulation for stability
	if (bUseFixedTimeStep)
	{
		AccumulatedTime += DeltaTime;

		while (AccumulatedTime >= FixedDeltaTime)
		{
			UpdateSoftBody(FixedDeltaTime);
			AccumulatedTime -= FixedDeltaTime;
		}
	}
	else
	{
		// Variable timestep (less stable but lower latency)
		UpdateSoftBody(DeltaTime);
	}
}

void USoftBodyPhysicsComponent::UpdateSoftBody(float DeltaTime)
{
	// Apply character movement influence
	ApplyCharacterMovement();

	// Apply spring force (Hooke's law: F = -k * x)
	ApplySpringForce();

	// Calculate acceleration from forces
	FVector Acceleration = FVector::ZeroVector;

	// Gravity
	Acceleration.Z -= Gravity;

	// Spring force (already applied to velocity in ApplySpringForce)

	// Semi-implicit Euler integration
	// v = v + a * dt
	Velocity += Acceleration * DeltaTime;

	// Apply damping
	Velocity *= Damping;

	// x = x + v * dt
	CurrentPosition += Velocity * DeltaTime;

	// Clamp displacement from rest position
	FVector OwnerLocation = OwnerCharacter->GetActorLocation();
	FVector TargetRestPosition = OwnerLocation + RestPosition;
	FVector Displacement = CurrentPosition - TargetRestPosition;

	if (Displacement.SizeSquared() > MaxDisplacement * MaxDisplacement)
	{
		CurrentPosition = TargetRestPosition + Displacement.GetClampedToMaxSize(MaxDisplacement);
	}
}

void USoftBodyPhysicsComponent::ApplySpringForce()
{
	// Calculate target rest position in world space
	FVector OwnerLocation = OwnerCharacter->GetActorLocation();
	FRotator OwnerRotation = OwnerCharacter->GetActorRotation();
	FVector WorldRestPosition = OwnerLocation + OwnerRotation.RotateVector(RestPosition);

	// Calculate displacement from rest position
	FVector Displacement = CurrentPosition - WorldRestPosition;

	// Hooke's law: F = -k * x
	FVector SpringForce = -SpringStiffness * Displacement;

	// Apply force to velocity (F = m * a, so a = F / m)
	FVector Acceleration = SpringForce / Mass;
	Velocity += Acceleration * FixedDeltaTime;
}

void USoftBodyPhysicsComponent::ApplyCharacterMovement()
{
	if (!OwnerCharacter)
	{
		return;
	}

	// Get current character state
	FVector CurrentCharacterLocation = OwnerCharacter->GetActorLocation();
	FRotator CurrentCharacterRotation = OwnerCharacter->GetActorRotation();

	// Calculate character movement
	FVector CharacterMovement = CurrentCharacterLocation - PreviousCharacterLocation;
	FRotator CharacterRotation = CurrentCharacterRotation - PreviousCharacterRotation;

	// Apply movement influence to soft-body velocity (inertia effect)
	// When character moves, soft-body lags behind due to inertia
	Velocity -= CharacterMovement * MovementInfluence;

	// Apply rotation influence (centrifugal effect)
	// When character rotates, soft-body experiences outward force
	if (!CharacterRotation.IsNearlyZero())
	{
		FVector OwnerLocation = OwnerCharacter->GetActorLocation();
		FVector Radius = CurrentPosition - OwnerLocation;

		// Simplified centrifugal force (omega^2 * r)
		float AngularVelocity = CharacterRotation.Yaw * RotationInfluence;
		FVector CentrifugalForce = Radius * AngularVelocity;
		Velocity += CentrifugalForce * FixedDeltaTime;
	}

	// Store character state for next frame
	PreviousCharacterLocation = CurrentCharacterLocation;
	PreviousCharacterRotation = CurrentCharacterRotation;
}
