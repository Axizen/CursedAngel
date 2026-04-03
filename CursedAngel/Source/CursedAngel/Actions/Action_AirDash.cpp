// Copyright Epic Games, Inc. All Rights Reserved.

#include "Action_AirDash.h"
#include "Data/ActionDataAsset.h"
#include "Characters/CursedAngelCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"

UAction_AirDash::UAction_AirDash()
	: DashSpeed(2000.0f)
	, DashDuration(0.3f)
	, MaxAirDashes(1)
	, CurrentAirDashes(0)
	, ElapsedTime(0.0f)
{
}

bool UAction_AirDash::CanActivate() const
{
	// Get the owner as a character
	ACharacter* Character = Cast<ACharacter>(OwnerActor);
	if (!Character || !Character->GetCharacterMovement())
	{
		return false;
	}

	// Check if character is airborne
	if (!Character->GetCharacterMovement()->IsFalling())
	{
		return false;
	}

	// Check if we have air dashes remaining
	if (CurrentAirDashes >= MaxAirDashes)
	{
		return false;
	}

	return true;
}

void UAction_AirDash::OnActivate()
{
	// Get the owner as a character
	ACharacter* Character = Cast<ACharacter>(OwnerActor);
	if (!Character)
	{
		return;
	}

	// Increment air dash count
	CurrentAirDashes++;

	// Get camera forward direction for dash
	FVector DashDirection = FVector::ForwardVector;
	
	// Try to get camera direction from player controller
	APlayerController* PC = Cast<APlayerController>(Character->GetController());
	if (PC)
	{
		DashDirection = PC->GetControlRotation().Vector();
		// Keep dash horizontal (no vertical component)
		DashDirection.Z = 0.0f;
		DashDirection.Normalize();
	}

	// Apply launch velocity in camera forward direction
	FVector LaunchVelocity = DashDirection * DashSpeed;
	Character->LaunchCharacter(LaunchVelocity, true, true);

	// Play montage if set in action data
	if (ActionData && ActionData->ActionMontage)
	{
		PlayMontage(ActionData->ActionMontage);
	}

	// Reset elapsed time
	ElapsedTime = 0.0f;

	// Set action as active
	bIsActive = true;
}

void UAction_AirDash::OnTick(float DeltaTime)
{
	if (!bIsActive)
	{
		return;
	}

	// Increment elapsed time
	ElapsedTime += DeltaTime;

	// Check if dash duration has elapsed
	if (ElapsedTime >= DashDuration)
	{
		OnCancel();
	}
}

void UAction_AirDash::OnCancel()
{
	// Set action as inactive
	bIsActive = false;
}

void UAction_AirDash::ResetAirDashes()
{
	// Reset air dash count when character lands
	CurrentAirDashes = 0;
}
