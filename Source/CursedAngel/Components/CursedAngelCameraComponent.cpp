// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/CursedAngelCameraComponent.h"
#include "Characters/CursedAngelCharacter.h"
#include "Components/ActionComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

UCursedAngelCameraComponent::UCursedAngelCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Initialize default camera mode settings
	
	// Free Camera: Standard exploration (Ratchet & Clank style)
	FreeCameraSettings.TargetDistance = 400.0f;
	FreeCameraSettings.TargetHeight = 80.0f;
	FreeCameraSettings.RotationSpeed = 5.0f;
	FreeCameraSettings.TransitionTime = 0.5f;
	FreeCameraSettings.bAutoRotateTowardMovement = true;
	FreeCameraSettings.bEnableCollisionAvoidance = true;
	FreeCameraSettings.FOV = 90.0f;

	// Combat Camera: Tighter framing for combat
	CombatCameraSettings.TargetDistance = 350.0f;
	CombatCameraSettings.TargetHeight = 60.0f;
	CombatCameraSettings.RotationSpeed = 8.0f;
	CombatCameraSettings.TransitionTime = 0.3f;
	CombatCameraSettings.bAutoRotateTowardMovement = false;
	CombatCameraSettings.bEnableCollisionAvoidance = true;
	CombatCameraSettings.FOV = 85.0f;

	// Lock-On Camera: DMC style lock-on
	LockOnCameraSettings.TargetDistance = 400.0f;
	LockOnCameraSettings.TargetHeight = 70.0f;
	LockOnCameraSettings.RotationSpeed = 10.0f;
	LockOnCameraSettings.TransitionTime = 0.2f;
	LockOnCameraSettings.bAutoRotateTowardMovement = false;
	LockOnCameraSettings.bEnableCollisionAvoidance = true;
	LockOnCameraSettings.FOV = 80.0f;

	// Platforming Camera: Pulled back for better spatial awareness
	PlatformingCameraSettings.TargetDistance = 500.0f;
	PlatformingCameraSettings.TargetHeight = 100.0f;
	PlatformingCameraSettings.RotationSpeed = 3.0f;
	PlatformingCameraSettings.TransitionTime = 0.4f;
	PlatformingCameraSettings.bAutoRotateTowardMovement = true;
	PlatformingCameraSettings.bEnableCollisionAvoidance = true;
	PlatformingCameraSettings.FOV = 95.0f;

	// Initialize state
	CurrentCameraMode = ECameraMode::Free;
	PreviousCameraMode = ECameraMode::Free;
	LockedTarget = nullptr;
	TransitionAlpha = 1.0f;
	CurrentDistance = FreeCameraSettings.TargetDistance;
	CurrentHeight = FreeCameraSettings.TargetHeight;
}

void UCursedAngelCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Cache owner character if needed
	if (!OwnerCharacter)
	{
		OwnerCharacter = Cast<ACursedAngelCharacter>(GetOwner());
		if (!OwnerCharacter)
		{
			return;
		}
	}

	// Update camera mode based on gameplay context
	UpdateCameraMode(DeltaTime);

	// Update camera based on current mode
	switch (CurrentCameraMode)
	{
	case ECameraMode::Free:
		UpdateFreeCamera(DeltaTime);
		break;
	case ECameraMode::Combat:
		UpdateCombatCamera(DeltaTime);
		break;
	case ECameraMode::LockOn:
		UpdateLockOnCamera(DeltaTime);
		break;
	case ECameraMode::Platforming:
		UpdatePlatformingCamera(DeltaTime);
		break;
	}

	// Handle collision avoidance
	const FCameraModeSettings* CurrentSettings = nullptr;
	switch (CurrentCameraMode)
	{
	case ECameraMode::Free: CurrentSettings = &FreeCameraSettings; break;
	case ECameraMode::Combat: CurrentSettings = &CombatCameraSettings; break;
	case ECameraMode::LockOn: CurrentSettings = &LockOnCameraSettings; break;
	case ECameraMode::Platforming: CurrentSettings = &PlatformingCameraSettings; break;
	}

	if (CurrentSettings && CurrentSettings->bEnableCollisionAvoidance)
	{
		HandleCollisionAvoidance(DeltaTime);
	}

	// Apply camera transform (camera position is relative to SpringArm)
	SetRelativeLocation(FVector::ZeroVector);
	SetFieldOfView(CurrentSettings ? CurrentSettings->FOV : 90.0f);
}

void UCursedAngelCameraComponent::SetCameraMode(ECameraMode NewMode, bool bForceImmediate)
{
	if (NewMode == CurrentCameraMode)
	{
		return;
	}

	// Store previous mode for reverting
	PreviousCameraMode = CurrentCameraMode;
	CurrentCameraMode = NewMode;

	// Reset transition
	TransitionAlpha = bForceImmediate ? 1.0f : 0.0f;

	// Call Blueprint event
	OnCameraModeChanged(PreviousCameraMode, CurrentCameraMode);
}

void UCursedAngelCameraComponent::UpdateCameraMode(float DeltaTime)
{
	// Skip auto-switching if disabled or locked on
	if (!bEnableAutoModeSwitch || LockedTarget != nullptr)
	{
		return;
	}

	// Check gameplay context and switch modes
	if (IsInAir())
	{
		if (CurrentCameraMode != ECameraMode::Platforming)
		{
			SetCameraMode(ECameraMode::Platforming);
		}
	}
	else if (IsInCombat())
	{
		if (CurrentCameraMode != ECameraMode::Combat)
		{
			SetCameraMode(ECameraMode::Combat);
		}
	}
	else
	{
		if (CurrentCameraMode != ECameraMode::Free)
		{
			SetCameraMode(ECameraMode::Free);
		}
	}
}

void UCursedAngelCameraComponent::UpdateFreeCamera(float DeltaTime)
{
	SmoothTransitionCamera(DeltaTime, FreeCameraSettings);

	if (!OwnerCharacter)
	{
		return;
	}

	// Get ideal camera position
	FVector IdealLocation = GetIdealCameraLocation(FreeCameraSettings);
	FRotator IdealRotation = GetIdealCameraRotation(FreeCameraSettings);

	// Auto-rotate toward movement direction
	if (FreeCameraSettings.bAutoRotateTowardMovement)
	{
		const FVector Velocity = OwnerCharacter->GetVelocity();
		if (Velocity.SizeSquared() > 100.0f) // Moving
		{
			FRotator MovementRotation = Velocity.Rotation();
			MovementRotation.Pitch = IdealRotation.Pitch;
			IdealRotation = FMath::RInterpTo(IdealRotation, MovementRotation, DeltaTime, FreeCameraSettings.RotationSpeed * 0.5f);
		}
	}

	// Smooth interpolate to ideal position
	CurrentCameraLocation = FMath::VInterpTo(CurrentCameraLocation, IdealLocation, DeltaTime, FreeCameraSettings.RotationSpeed);
	CurrentCameraRotation = FMath::RInterpTo(CurrentCameraRotation, IdealRotation, DeltaTime, FreeCameraSettings.RotationSpeed);

	// Apply to SpringArm if available
	if (USpringArmComponent* SpringArm = OwnerCharacter->FindComponentByClass<USpringArmComponent>())
	{
		SpringArm->TargetArmLength = CurrentDistance;
		SpringArm->SocketOffset = FVector(0.0f, 0.0f, CurrentHeight);
	}
}

void UCursedAngelCameraComponent::UpdateCombatCamera(float DeltaTime)
{
	SmoothTransitionCamera(DeltaTime, CombatCameraSettings);

	if (!OwnerCharacter)
	{
		return;
	}

	// Get ideal camera position
	FVector IdealLocation = GetIdealCameraLocation(CombatCameraSettings);
	FRotator IdealRotation = GetIdealCameraRotation(CombatCameraSettings);

	// Find nearest enemy and orient toward it
	AActor* NearestEnemy = FindNearestLockOnTarget(CombatDetectionRadius);
	if (NearestEnemy)
	{
		FVector ToEnemy = NearestEnemy->GetActorLocation() - OwnerCharacter->GetActorLocation();
		ToEnemy.Z = 0.0f; // Keep rotation on horizontal plane
		if (ToEnemy.SizeSquared() > 0.0f)
		{
			FRotator EnemyRotation = ToEnemy.Rotation();
			EnemyRotation.Pitch = IdealRotation.Pitch;
			IdealRotation = FMath::RInterpTo(IdealRotation, EnemyRotation, DeltaTime, CombatCameraSettings.RotationSpeed);
		}
	}

	// Faster interpolation for combat
	CurrentCameraLocation = FMath::VInterpTo(CurrentCameraLocation, IdealLocation, DeltaTime, CombatCameraSettings.RotationSpeed);
	CurrentCameraRotation = FMath::RInterpTo(CurrentCameraRotation, IdealRotation, DeltaTime, CombatCameraSettings.RotationSpeed);

	// Apply to SpringArm if available
	if (USpringArmComponent* SpringArm = OwnerCharacter->FindComponentByClass<USpringArmComponent>())
	{
		SpringArm->TargetArmLength = CurrentDistance;
		SpringArm->SocketOffset = FVector(0.0f, 0.0f, CurrentHeight);
	}
}

void UCursedAngelCameraComponent::UpdateLockOnCamera(float DeltaTime)
{
	SmoothTransitionCamera(DeltaTime, LockOnCameraSettings);

	if (!OwnerCharacter)
	{
		return;
	}

	// Validate locked target
	if (!LockedTarget || !IsValid(LockedTarget))
	{
		UnlockTarget();
		return;
	}

	// Calculate camera position to keep player between camera and target (orbit)
	FVector PlayerLocation = OwnerCharacter->GetActorLocation();
	FVector TargetLocation = LockedTarget->GetActorLocation();
	FVector ToTarget = (TargetLocation - PlayerLocation).GetSafeNormal();

	// Camera looks at target
	FVector CameraOffset = -ToTarget * CurrentDistance;
	CameraOffset.Z += CurrentHeight;

	FVector IdealLocation = PlayerLocation + CameraOffset;
	FRotator IdealRotation = (TargetLocation - IdealLocation).Rotation();

	// Handle right stick input for orbit control
	if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
	{
		float RightX = 0.0f, RightY = 0.0f;
		PC->GetInputAnalogStickState(EControllerAnalogStick::CAS_RightStick, RightX, RightY);

		if (FMath::Abs(RightX) > 0.1f)
		{
			// Rotate camera around player horizontally
			FRotator OrbitRotation = FRotator(0.0f, RightX * 90.0f * DeltaTime, 0.0f);
			CameraOffset = OrbitRotation.RotateVector(CameraOffset);
		}
	}

	// Smooth interpolate
	CurrentCameraLocation = FMath::VInterpTo(CurrentCameraLocation, IdealLocation, DeltaTime, LockOnCameraSettings.RotationSpeed);
	CurrentCameraRotation = FMath::RInterpTo(CurrentCameraRotation, IdealRotation, DeltaTime, LockOnCameraSettings.RotationSpeed);

	// Apply to SpringArm if available
	if (USpringArmComponent* SpringArm = OwnerCharacter->FindComponentByClass<USpringArmComponent>())
	{
		SpringArm->TargetArmLength = CurrentDistance;
		SpringArm->SocketOffset = FVector(0.0f, 0.0f, CurrentHeight);
		
		// Force SpringArm to look at target
		FVector SpringArmLocation = SpringArm->GetComponentLocation();
		FRotator LookAtRotation = (TargetLocation - SpringArmLocation).Rotation();
		SpringArm->SetWorldRotation(LookAtRotation);
	}
}

void UCursedAngelCameraComponent::UpdatePlatformingCamera(float DeltaTime)
{
	SmoothTransitionCamera(DeltaTime, PlatformingCameraSettings);

	if (!OwnerCharacter)
	{
		return;
	}

	// Get ideal camera position (pulled back)
	FVector IdealLocation = GetIdealCameraLocation(PlatformingCameraSettings);
	FRotator IdealRotation = GetIdealCameraRotation(PlatformingCameraSettings);

	// Look slightly down to see landing zone
	IdealRotation.Pitch = FMath::Clamp(IdealRotation.Pitch - 10.0f, -45.0f, 0.0f);

	// Smooth out movement for platforming stability
	CurrentCameraLocation = FMath::VInterpTo(CurrentCameraLocation, IdealLocation, DeltaTime, PlatformingCameraSettings.RotationSpeed);
	CurrentCameraRotation = FMath::RInterpTo(CurrentCameraRotation, IdealRotation, DeltaTime, PlatformingCameraSettings.RotationSpeed);

	// Apply to SpringArm if available
	if (USpringArmComponent* SpringArm = OwnerCharacter->FindComponentByClass<USpringArmComponent>())
	{
		SpringArm->TargetArmLength = CurrentDistance;
		SpringArm->SocketOffset = FVector(0.0f, 0.0f, CurrentHeight);
	}
}

void UCursedAngelCameraComponent::SmoothTransitionCamera(float DeltaTime, const FCameraModeSettings& TargetSettings)
{
	// Update transition alpha
	if (TransitionAlpha < 1.0f)
	{
		TransitionAlpha = FMath::Clamp(TransitionAlpha + (DeltaTime / TargetSettings.TransitionTime), 0.0f, 1.0f);
	}

	// Interpolate distance and height
	CurrentDistance = FMath::FInterpTo(CurrentDistance, TargetSettings.TargetDistance, DeltaTime, TargetSettings.RotationSpeed);
	CurrentHeight = FMath::FInterpTo(CurrentHeight, TargetSettings.TargetHeight, DeltaTime, TargetSettings.RotationSpeed);
}

void UCursedAngelCameraComponent::HandleCollisionAvoidance(float DeltaTime)
{
	if (!OwnerCharacter)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	USpringArmComponent* SpringArm = OwnerCharacter->FindComponentByClass<USpringArmComponent>();
	if (!SpringArm)
	{
		return;
	}

	// Get ideal camera location (where camera wants to be)
	FVector PlayerLocation = OwnerCharacter->GetActorLocation();
	FVector IdealCameraLocation = SpringArm->GetComponentLocation() - SpringArm->GetForwardVector() * CurrentDistance;
	IdealCameraLocation.Z += CurrentHeight;

	// Sweep sphere from player to ideal camera location
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);
	QueryParams.bTraceComplex = false;

	FHitResult HitResult;
	bool bHit = World->SweepSingleByChannel(
		HitResult,
		PlayerLocation,
		IdealCameraLocation,
		FQuat::Identity,
		ECC_Camera,
		FCollisionShape::MakeSphere(CollisionProbeSize),
		QueryParams
	);

	if (bHit)
	{
		// Move camera to hit point with offset
		float AdjustedDistance = FMath::Max((HitResult.Location - PlayerLocation).Size() - CollisionProbeSize, 50.0f);
		SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, AdjustedDistance, DeltaTime, 10.0f);
	}
	else
	{
		// Smooth return to ideal distance
		SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, CurrentDistance, DeltaTime, 5.0f);
	}
}

void UCursedAngelCameraComponent::LockOnToTarget(AActor* Target)
{
	if (!Target || !IsValid(Target))
	{
		return;
	}

	LockedTarget = Target;
	SetCameraMode(ECameraMode::LockOn);
	OnTargetLocked(Target);
}

void UCursedAngelCameraComponent::UnlockTarget()
{
	if (!LockedTarget)
	{
		return;
	}

	LockedTarget = nullptr;
	SetCameraMode(PreviousCameraMode);
	OnTargetUnlocked();
}

AActor* UCursedAngelCameraComponent::FindNearestLockOnTarget(float MaxDistance)
{
	if (!OwnerCharacter)
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// Find all actors with 'Enemy' tag
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(World, FName("Enemy"), FoundActors);

	AActor* NearestEnemy = nullptr;
	float NearestDistSq = MaxDistance * MaxDistance;
	FVector PlayerLocation = OwnerCharacter->GetActorLocation();

	for (AActor* Actor : FoundActors)
	{
		if (!Actor || !IsValid(Actor))
		{
			continue;
		}

		float DistSq = FVector::DistSquared(PlayerLocation, Actor->GetActorLocation());
		if (DistSq < NearestDistSq)
		{
			// Line-of-sight check
			FHitResult HitResult;
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(OwnerCharacter);
			QueryParams.AddIgnoredActor(Actor);

			bool bHit = World->LineTraceSingleByChannel(
				HitResult,
				PlayerLocation,
				Actor->GetActorLocation(),
				ECC_Visibility,
				QueryParams
			);

			if (!bHit) // No obstruction
			{
				NearestEnemy = Actor;
				NearestDistSq = DistSq;
			}
		}
	}

	return NearestEnemy;
}

void UCursedAngelCameraComponent::SwitchLockOnTarget(bool bNext)
{
	if (!OwnerCharacter)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Get all enemies
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(World, FName("Enemy"), FoundActors);

	if (FoundActors.Num() == 0)
	{
		return;
	}

	// Sort by distance
	FVector PlayerLocation = OwnerCharacter->GetActorLocation();
	FoundActors.Sort([PlayerLocation](const AActor& A, const AActor& B) {
		float DistA = FVector::DistSquared(PlayerLocation, A.GetActorLocation());
		float DistB = FVector::DistSquared(PlayerLocation, B.GetActorLocation());
		return DistA < DistB;
	});

	// Find current target index
	int32 CurrentIndex = FoundActors.IndexOfByKey(LockedTarget);
	
	if (CurrentIndex == INDEX_NONE)
	{
		// No current target, lock to nearest
		LockOnToTarget(FoundActors[0]);
	}
	else
	{
		// Switch to next/previous
		int32 NewIndex = bNext ? (CurrentIndex + 1) % FoundActors.Num() : (CurrentIndex - 1 + FoundActors.Num()) % FoundActors.Num();
		LockOnToTarget(FoundActors[NewIndex]);
	}
}

bool UCursedAngelCameraComponent::IsInCombat() const
{
	if (!OwnerCharacter)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// Check if enemies are within detection radius
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(World, FName("Enemy"), FoundActors);

	FVector PlayerLocation = OwnerCharacter->GetActorLocation();
	float DetectionRadiusSq = CombatDetectionRadius * CombatDetectionRadius;

	for (AActor* Actor : FoundActors)
	{
		if (Actor && IsValid(Actor))
		{
			float DistSq = FVector::DistSquared(PlayerLocation, Actor->GetActorLocation());
			if (DistSq < DetectionRadiusSq)
			{
				return true;
			}
		}
	}

	return false;
}

bool UCursedAngelCameraComponent::IsInAir() const
{
	if (!OwnerCharacter)
	{
		return false;
	}

	UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement();
	if (!Movement)
	{
		return false;
	}

	return Movement->IsFalling();
}

FVector UCursedAngelCameraComponent::GetIdealCameraLocation(const FCameraModeSettings& Settings) const
{
	if (!OwnerCharacter)
	{
		return FVector::ZeroVector;
	}

	// Get player location and rotation
	FVector PlayerLocation = OwnerCharacter->GetActorLocation();
	FRotator PlayerRotation = OwnerCharacter->GetControlRotation();

	// Calculate offset behind player
	FVector ForwardVector = PlayerRotation.Vector();
	FVector Offset = -ForwardVector * Settings.TargetDistance;
	Offset.Z += Settings.TargetHeight;

	return PlayerLocation + Offset;
}

FRotator UCursedAngelCameraComponent::GetIdealCameraRotation(const FCameraModeSettings& Settings) const
{
	if (!OwnerCharacter)
	{
		return FRotator::ZeroRotator;
	}

	// Base rotation on control rotation
	return OwnerCharacter->GetControlRotation();
}
