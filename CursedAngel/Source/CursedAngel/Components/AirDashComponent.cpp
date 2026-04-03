// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/AirDashComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

UAirDashComponent::UAirDashComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Default values
	AirDashSpeed = 2000.0f;
	AirDashDuration = 0.3f;
	AirDashCooldown = 1.0f;
	MaxAirDashes = 2;
	
	CurrentAirDashes = MaxAirDashes;
	LastDashTime = -999.0f;
	bIsDashing = false;
	DashDirection = FVector::ZeroVector;
	DashTimeRemaining = 0.0f;
	bAirDashEnabled = false;
	
	OwnerCharacter = nullptr;
	MovementComponent = nullptr;
}

void UAirDashComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache owner character and movement component
	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		MovementComponent = OwnerCharacter->GetCharacterMovement();
		
		// Bind to landed event to reset air dashes
		OwnerCharacter->LandedDelegate.AddDynamic(this, &UAirDashComponent::OnCharacterLanded);
	}
}

void UAirDashComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update dash if currently dashing
	if (bIsDashing)
	{
		UpdateDash(DeltaTime);
	}
}

bool UAirDashComponent::CanAirDash() const
{
	if (!bAirDashEnabled || !OwnerCharacter || !MovementComponent)
	{
		return false;
	}

	// Must be in air
	if (MovementComponent->IsMovingOnGround())
	{
		return false;
	}

	// Must have dashes remaining
	if (CurrentAirDashes <= 0)
	{
		return false;
	}

	// Check cooldown
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastDashTime < AirDashCooldown)
	{
		return false;
	}

	// Cannot dash while already dashing
	if (bIsDashing)
	{
		return false;
	}

	return true;
}

void UAirDashComponent::PerformAirDash(FVector Direction)
{
	if (!CanAirDash())
	{
		return;
	}

	// Normalize direction
	Direction.Normalize();
	if (Direction.IsNearlyZero())
	{
		// Default to forward direction if no direction provided
		Direction = OwnerCharacter->GetActorForwardVector();
	}

	// Store dash state
	DashDirection = Direction;
	bIsDashing = true;
	DashTimeRemaining = AirDashDuration;
	LastDashTime = GetWorld()->GetTimeSeconds();
	CurrentAirDashes--;

	// Apply initial impulse
	if (MovementComponent)
	{
		// Stop current velocity and apply dash velocity
		MovementComponent->StopMovementImmediately();
		MovementComponent->Velocity = DashDirection * AirDashSpeed;
		
		// Disable gravity during dash for more control
		MovementComponent->GravityScale = 0.0f;
	}

	// Fire Blueprint event
	OnAirDashStart(DashDirection);
}

void UAirDashComponent::UpdateDash(float DeltaTime)
{
	DashTimeRemaining -= DeltaTime;

	if (DashTimeRemaining <= 0.0f)
	{
		EndDash();
		return;
	}

	// Maintain dash velocity during dash
	if (MovementComponent)
	{
		MovementComponent->Velocity = DashDirection * AirDashSpeed;
	}
}

void UAirDashComponent::EndDash()
{
	bIsDashing = false;
	DashTimeRemaining = 0.0f;

	// Re-enable gravity
	if (MovementComponent)
	{
		MovementComponent->GravityScale = 1.0f;
	}

	// Fire Blueprint event
	OnAirDashEnd(DashDirection);
	
	DashDirection = FVector::ZeroVector;
}

void UAirDashComponent::ResetAirDashes()
{
	CurrentAirDashes = MaxAirDashes;
}

void UAirDashComponent::SetAirDashEnabled(bool bEnabled)
{
	bAirDashEnabled = bEnabled;
	
	if (!bEnabled && bIsDashing)
	{
		// End dash if disabled while dashing
		EndDash();
	}
	
	if (bEnabled)
	{
		// Reset dashes when enabled
		ResetAirDashes();
	}
}

void UAirDashComponent::OnCharacterLanded(const FHitResult& Hit)
{
	// Reset air dashes when landing
	ResetAirDashes();
	
	// End dash if landing while dashing
	if (bIsDashing)
	{
		EndDash();
	}
}
