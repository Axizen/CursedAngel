// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/CursedAngelCameraComponent.h"
#include "GameFramework/Character.h"
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

	// Per-mode camera lag defaults
	FreeCameraSettings.bEnableCameraLag = true;
	FreeCameraSettings.CameraLagSpeed = 8.0f;

	CombatCameraSettings.bEnableCameraLag = true;
	CombatCameraSettings.CameraLagSpeed = 12.0f;

	LockOnCameraSettings.bEnableCameraLag = true;
	LockOnCameraSettings.CameraLagSpeed = 15.0f;

	PlatformingCameraSettings.bEnableCameraLag = true;
	PlatformingCameraSettings.CameraLagSpeed = 6.0f;

	// Initialize state
	CurrentCameraMode = ECameraMode::Free;
	PreviousCameraMode = ECameraMode::Free;
	LockedTarget = nullptr;
	TransitionAlpha = 1.0f;
	CurrentDistance = FreeCameraSettings.TargetDistance;
	CurrentHeight = FreeCameraSettings.TargetHeight;
	CurrentFOV = 90.0f;
	EnemyCacheTimer = 0.0f;
}

void UCursedAngelCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Cache owner character if needed
	if (!OwnerCharacter)
	{
		OwnerCharacter = Cast<ACharacter>(GetOwner());
		if (!OwnerCharacter)
		{
			return;
		}
	}

	// Tick enemy cache timer and refresh when expired
	EnemyCacheTimer -= DeltaTime;
	if (EnemyCacheTimer <= 0.0f)
	{
		RefreshEnemyCache();
		EnemyCacheTimer = EnemyCacheInterval;
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

	// Apply SpringArm camera lag settings for current mode
	if (CurrentSettings && OwnerCharacter)
	{
		if (USpringArmComponent* SpringArm = OwnerCharacter->FindComponentByClass<USpringArmComponent>())
		{
			SpringArm->bEnableCameraLag = CurrentSettings->bEnableCameraLag;
			SpringArm->CameraLagSpeed = CurrentSettings->CameraLagSpeed;
		}
	}

	// Interpolate FOV smoothly between modes
	float TargetFOV = CurrentSettings ? CurrentSettings->FOV : 90.0f;
	CurrentFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, 5.0f);

	// Apply camera transform (camera position is relative to SpringArm)
	SetRelativeLocation(FVector::ZeroVector);
	SetFieldOfView(CurrentFOV);
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
	// Skip auto-switching if disabled
	if (!bEnableAutoModeSwitch)
	{
		return;
	}

	// Priority: LockOn > Combat > Platforming (air only, not in combat) > Free
	if (LockedTarget != nullptr)
	{
		if (CurrentCameraMode != ECameraMode::LockOn)
		{
			SetCameraMode(ECameraMode::LockOn);
		}
	}
	else if (IsInCombat())
	{
		if (CurrentCameraMode != ECameraMode::Combat)
		{
			SetCameraMode(ECameraMode::Combat);
		}
	}
	else if (IsInAir())
	{
		if (CurrentCameraMode != ECameraMode::Platforming)
		{
			SetCameraMode(ECameraMode::Platforming);
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

	// Apply to SpringArm if available
	if (USpringArmComponent* SpringArm = OwnerCharacter->FindComponentByClass<USpringArmComponent>())
	{
		SpringArm->TargetArmLength = CurrentDistance;

		// Apply screen-midpoint socket offset to frame player and target (DMC style)
		SpringArm->SocketOffset = GetLockOnSocketOffset();
		
		// Force SpringArm to look at the midpoint between player and target
		FVector MidPoint = FMath::Lerp(PlayerLocation, TargetLocation, LockOnFramingBias);
		FVector SpringArmLocation = SpringArm->GetComponentLocation();
		FRotator LookAtRotation = (MidPoint - SpringArmLocation).Rotation();
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

	// Dynamically increase pitch downward when falling fast (better landing visibility)
	float VelZ = OwnerCharacter->GetVelocity().Z;
	float PitchBoost = FMath::GetMappedRangeValueClamped(FVector2D(-1200.f, 0.f), FVector2D(-20.f, 0.f), VelZ);
	IdealRotation.Pitch = FMath::Clamp(IdealRotation.Pitch + PitchBoost, -45.f, 0.f);

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

	AActor* NearestEnemy = nullptr;
	float NearestDistSq = MaxDistance * MaxDistance;
	FVector PlayerLocation = OwnerCharacter->GetActorLocation();

	for (AActor* Actor : CachedEnemies)
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

	if (CachedEnemies.Num() == 0)
	{
		return;
	}

	// Build a list of valid enemies sorted by angle to screen center (most centered = best)
	FVector CameraLocation = GetComponentLocation();
	FVector CameraForward = GetForwardVector();

	TArray<AActor*> ValidEnemies = CachedEnemies.FilterByPredicate([](AActor* A) {
		return A && IsValid(A);
	});

	if (ValidEnemies.Num() == 0)
	{
		return;
	}

	// Sort by screen-center angle (dot product: higher = more centered)
	ValidEnemies.Sort([CameraLocation, CameraForward](const AActor& A, const AActor& B) {
		FVector DirA = (A.GetActorLocation() - CameraLocation).GetSafeNormal();
		FVector DirB = (B.GetActorLocation() - CameraLocation).GetSafeNormal();
		return FVector::DotProduct(CameraForward, DirA) > FVector::DotProduct(CameraForward, DirB);
	});

	// Find current target index in sorted list
	int32 CurrentIndex = ValidEnemies.IndexOfByKey(LockedTarget);
	
	if (CurrentIndex == INDEX_NONE)
	{
		// No current target, lock to most-centered
		LockOnToTarget(ValidEnemies[0]);
	}
	else
	{
		// Switch to next/previous
		int32 NewIndex = bNext ? (CurrentIndex + 1) % ValidEnemies.Num() : (CurrentIndex - 1 + ValidEnemies.Num()) % ValidEnemies.Num();
		LockOnToTarget(ValidEnemies[NewIndex]);
	}
}

bool UCursedAngelCameraComponent::IsInCombat() const
{
	if (!OwnerCharacter)
	{
		return false;
	}

	// Check if any cached enemies are within detection radius
	FVector PlayerLocation = OwnerCharacter->GetActorLocation();
	float DetectionRadiusSq = CombatDetectionRadius * CombatDetectionRadius;

	for (AActor* Actor : CachedEnemies)
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

void UCursedAngelCameraComponent::RefreshEnemyCache()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Repopulate CachedEnemies from world actors tagged 'Enemy'
	UGameplayStatics::GetAllActorsWithTag(World, FName("Enemy"), CachedEnemies);

	// Remove any invalid/dead actors from the cache
	CachedEnemies.RemoveAll([](AActor* Actor) {
		return !Actor || !IsValid(Actor);
	});
}

FVector UCursedAngelCameraComponent::GetLockOnSocketOffset() const
{
	if (!OwnerCharacter || !LockedTarget)
	{
		return FVector(0.0f, 0.0f, CurrentHeight);
	}

	USpringArmComponent* SpringArm = OwnerCharacter->FindComponentByClass<USpringArmComponent>();
	if (!SpringArm)
	{
		return FVector(0.0f, 0.0f, CurrentHeight);
	}

	FVector PlayerLocation = OwnerCharacter->GetActorLocation();
	FVector TargetLocation = LockedTarget->GetActorLocation();

	// Compute the desired world-space framing point (DMC style: slightly biased toward target)
	FVector FrameCenter = FMath::Lerp(PlayerLocation, TargetLocation, LockOnFramingBias);

	// The SpringArm base is at the character root; compute offset in arm local space
	// We want the socket (camera base) to shift so that FrameCenter is centered on screen.
	// Project the lateral offset into SpringArm's right/up axes.
	FVector ArmRight = SpringArm->GetRightVector();
	FVector ArmUp = SpringArm->GetUpVector();

	FVector WorldOffset = FrameCenter - PlayerLocation;
	float RightOffset = FVector::DotProduct(WorldOffset, ArmRight);
	float UpOffset = FVector::DotProduct(WorldOffset, ArmUp);

	// Clamp offsets to avoid extreme framing
	RightOffset = FMath::Clamp(RightOffset, -200.0f, 200.0f);
	UpOffset = FMath::Clamp(UpOffset + CurrentHeight, -100.0f, 300.0f);

	return FVector(0.0f, RightOffset, UpOffset);
}
