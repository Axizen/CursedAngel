// Copyright Epic Games, Inc. All Rights Reserved.

#include "Characters/Ripley.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "CursedAngelComponent.h"
#include "Data/CharacterDataAsset.h"
#include "Physics/SoftBodyPhysicsComponent.h"

//DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ARipley

ARipley::ARipley()
{
	// Call base class constructor
	// Base class initializes: CameraBoom, FollowCamera, CurseWeaponComponent, CursedAngelComponent, StyleComponent
	
	// Create Ripley-specific components
	SoftBody_BreastL = CreateDefaultSubobject<USoftBodyPhysicsComponent>(TEXT("SoftBody_Breast_L"));
	SoftBody_BreastR = CreateDefaultSubobject<USoftBodyPhysicsComponent>(TEXT("SoftBody_Breast_R"));
	SoftBody_ThighL  = CreateDefaultSubobject<USoftBodyPhysicsComponent>(TEXT("SoftBody_ThighL"));
	SoftBody_ThighR  = CreateDefaultSubobject<USoftBodyPhysicsComponent>(TEXT("SoftBody_ThighR"));
	SoftBody_ButtL   = CreateDefaultSubobject<USoftBodyPhysicsComponent>(TEXT("SoftBody_Butt_L"));
	SoftBody_ButtR   = CreateDefaultSubobject<USoftBodyPhysicsComponent>(TEXT("SoftBody_Butt_R"));

	// Default stored values — overwritten by ApplyTransformationMovementModifiers() at runtime
	// Aligned with FMovementFeelConfig defaults so the delta calculations are consistent.
	OriginalMaxWalkSpeed = 700.f;
	OriginalJumpHeight = 800.f;
	OriginalAirControl = 0.85f;
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
	OriginalJumpHeight = MovementComp->JumpZVelocity;
	OriginalAirControl = MovementComp->AirControl;

	// Programmer: Applies transformation movement multipliers relative to MovementFeel config baseline.
	// Reads config values from CharacterDataAsset->MovementFeel if available; falls back to stored originals.
	// Air dash is now in the base class (ACursedAngelCharacter) — no component enable/disable needed.
	const FMovementFeelConfig* FeelConfig = nullptr;
	if (CharacterConfig)
	{
		FeelConfig = &CharacterConfig->MovementFeel;
	}

	const float BaseWalkSpeed = (FeelConfig && FeelConfig->MaxWalkSpeed > 0.f) ? FeelConfig->MaxWalkSpeed : OriginalMaxWalkSpeed;
	const float BaseJumpHeight = (FeelConfig && FeelConfig->JumpHeight > 0.f) ? FeelConfig->JumpHeight : OriginalJumpHeight;
	const float BaseAirControl = (FeelConfig && FeelConfig->AirControl > 0.f) ? FeelConfig->AirControl : OriginalAirControl;

	const float SpeedMult = CursedComp->SpeedMultiplier;
	const float JumpMult = 1.3f;       // R&C-style jump boost during transformation
	const float AirControlMult = 1.5f; // Enhanced air control during transformation

	MovementComp->MaxWalkSpeed = BaseWalkSpeed * SpeedMult;
	MovementComp->JumpZVelocity = BaseJumpHeight * JumpMult;
	MovementComp->AirControl = BaseAirControl * AirControlMult;

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
	// Programmer: Restores movement parameters after transformation ends.
	// Uses ApplyMovementFeelConfig() for a clean restore from the data asset rather than
	// raw stored values — ensures config changes made at runtime are respected.
	// Air dash is now in the base class (ACursedAngelCharacter) and requires no
	// component enable/disable here; ResetAirDashes() is called separately by the transformation system.

	// Re-apply the full movement feel config from CharacterDataAsset.
	// This is the authoritative source of truth for movement parameters post-transformation.
	ApplyMovementFeelConfig();

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