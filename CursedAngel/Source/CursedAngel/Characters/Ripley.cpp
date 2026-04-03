// Copyright Epic Games, Inc. All Rights Reserved.

#include "Characters/Ripley.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "CursedAngelComponent.h"
#include "AirDashComponent.h"

//DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ARipley

ARipley::ARipley()
{
	// Call base class constructor
	// Base class initializes: CameraBoom, FollowCamera, CurseWeaponComponent, CursedAngelComponent, StyleComponent
	
	// Create Ripley-specific components
	
	// Create air dash component - Ripley's signature mobility ability
	AirDashComponent = CreateDefaultSubobject<UAirDashComponent>(TEXT("AirDashComponent"));

	// Initialize original movement values for transformation
	OriginalMaxWalkSpeed = 500.f;
	OriginalJumpZVelocity = 700.f;
	OriginalAirControl = 0.35f;
	OriginalCameraFOV = 90.0f;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named BP_RipleyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Ripley-Specific Functions
// Base class functions (Move, Look, FireCurseWeapon, etc.) are inherited from ACursedAngelCharacter

void ARipley::ApplyTransformationMovementModifiers()
{
	UCharacterMovementComponent* MovementComp = GetCharacterMovement();
	UCursedAngelComponent* CursedComp = GetCursedAngelComponent();
	if (!MovementComp || !CursedComp)
	{
		return;
	}

	// Store original values
	OriginalMaxWalkSpeed = MovementComp->MaxWalkSpeed;
	OriginalJumpZVelocity = MovementComp->JumpZVelocity;
	OriginalAirControl = MovementComp->AirControl;

	// Get movement config from CursedAngelComponent
	// Note: Using legacy properties for now, will be replaced with MovementConfig struct
	float SpeedMult = CursedComp->SpeedMultiplier;
	float JumpMult = 1.3f; // Default jump multiplier
	float AirControlMult = 1.5f; // Default air control multiplier

	// Apply multipliers
	MovementComp->MaxWalkSpeed = OriginalMaxWalkSpeed * SpeedMult;
	MovementComp->JumpZVelocity = OriginalJumpZVelocity * JumpMult;
	MovementComp->AirControl = OriginalAirControl * AirControlMult;

	// Enable air dash component - Ripley's signature ability
	if (AirDashComponent)
	{
		AirDashComponent->SetAirDashEnabled(true);
	}

	// Update camera FOV for speed sensation
	UCameraComponent* Camera = GetFollowCamera();
	if (Camera)
	{
		OriginalCameraFOV = Camera->FieldOfView;
		Camera->SetFieldOfView(OriginalCameraFOV + 10.0f); // Slight FOV increase
	}
}

void ARipley::RevertTransformationMovementModifiers()
{
	UCharacterMovementComponent* MovementComp = GetCharacterMovement();
	if (!MovementComp)
	{
		return;
	}

	// Restore original movement values
	MovementComp->MaxWalkSpeed = OriginalMaxWalkSpeed;
	MovementComp->JumpZVelocity = OriginalJumpZVelocity;
	MovementComp->AirControl = OriginalAirControl;

	// Disable air dash component
	if (AirDashComponent)
	{
		AirDashComponent->SetAirDashEnabled(false);
	}

	// Reset camera FOV
	UCameraComponent* Camera = GetFollowCamera();
	if (Camera)
	{
		Camera->SetFieldOfView(OriginalCameraFOV);
	}
}

void ARipley::OnTransformationStateChanged(bool bTransformed)
{
	if (bTransformed)
	{
		ApplyTransformationMovementModifiers();
	}
	else
	{
		RevertTransformationMovementModifiers();
	}
}

//////////////////////////////////////////////////////////////////////////
// Virtual Function Overrides

void ARipley::OnCombatAction_Implementation()
{
	Super::OnCombatAction_Implementation();
	// Ripley-specific combat action logic can be added here
}

void ARipley::OnStyleRankChanged_Implementation(EStyleRank NewRank)
{
	Super::OnStyleRankChanged_Implementation(NewRank);
	// Ripley-specific style rank change logic can be added here
}

void ARipley::OnCursedAngelActivated_Implementation()
{
	Super::OnCursedAngelActivated_Implementation();
	
	// Bind to transformation events if not already bound
	UCursedAngelComponent* CursedComp = GetCursedAngelComponent();
	if (CursedComp && !CursedComp->OnTransformationStateChangedDelegate.IsBound())
	{
		CursedComp->OnTransformationStateChangedDelegate.AddDynamic(this, &ARipley::OnTransformationStateChanged);
	}
}

void ARipley::OnCursedAngelDeactivated_Implementation()
{
	Super::OnCursedAngelDeactivated_Implementation();
	// Ripley-specific cursed angel deactivation logic can be added here
}