// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "GameplayTagContainer.h"
#include "CursedAngelCameraComponent.generated.h"

class ACursedAngelCharacter;

/**
 * Camera modes for different gameplay contexts
 */
UENUM(BlueprintType)
enum class ECameraMode : uint8
{
	Free UMETA(DisplayName = "Free Camera"),
	Combat UMETA(DisplayName = "Combat Camera"),
	LockOn UMETA(DisplayName = "Lock-On Camera"),
	Platforming UMETA(DisplayName = "Platforming Camera")
};

/**
 * Camera settings per mode
 */
USTRUCT(BlueprintType)
struct FCameraModeSettings
{
	GENERATED_BODY()

	/** Target distance from character */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Mode Settings")
	float TargetDistance = 400.0f;

	/** Target height offset from character */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Mode Settings")
	float TargetHeight = 80.0f;

	/** Camera rotation interpolation speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Mode Settings")
	float RotationSpeed = 5.0f;

	/** Time to transition to this mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Mode Settings")
	float TransitionTime = 0.5f;

	/** Auto-rotate camera toward movement direction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Mode Settings")
	bool bAutoRotateTowardMovement = false;

	/** Enable collision avoidance for this mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Mode Settings")
	bool bEnableCollisionAvoidance = true;

	/** Field of view for this mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Mode Settings")
	float FOV = 90.0f;
};

/**
 * Advanced camera component with multiple modes (Free, Combat, Lock-On, Platforming)
 * Features smooth transitions, lock-on targeting, and collision avoidance
 */
UCLASS(ClassGroup = (Camera), meta = (BlueprintSpawnableComponent))
class CURSEDANGEL_API UCursedAngelCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

public:
	UCursedAngelCameraComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Camera Mode Settings
	
	/** Settings for Free camera mode (Ratchet & Clank style) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	FCameraModeSettings FreeCameraSettings;

	/** Settings for Combat camera mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	FCameraModeSettings CombatCameraSettings;

	/** Settings for Lock-On camera mode (DMC style) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	FCameraModeSettings LockOnCameraSettings;

	/** Settings for Platforming camera mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	FCameraModeSettings PlatformingCameraSettings;

	// Current State

	/** Current active camera mode */
	UPROPERTY(BlueprintReadOnly, Category = "Camera State")
	ECameraMode CurrentCameraMode;

	/** Currently locked target (null if not locked on) */
	UPROPERTY(BlueprintReadWrite, Category = "Camera State")
	AActor* LockedTarget;

	// Configuration

	/** Radius to detect nearby enemies for combat mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Configuration")
	float CombatDetectionRadius = 1500.0f;

	/** Size of collision probe sphere */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Configuration")
	float CollisionProbeSize = 12.0f;

	/** Enable automatic mode switching based on gameplay context */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Configuration")
	bool bEnableAutoModeSwitch = true;

	// Public Functions

	/** Set camera mode manually */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetCameraMode(ECameraMode NewMode, bool bForceImmediate = false);

	/** Get current camera mode */
	UFUNCTION(BlueprintPure, Category = "Camera")
	ECameraMode GetCameraMode() const { return CurrentCameraMode; }

	/** Lock onto a specific target */
	UFUNCTION(BlueprintCallable, Category = "Camera|Lock-On")
	void LockOnToTarget(AActor* Target);

	/** Unlock current target */
	UFUNCTION(BlueprintCallable, Category = "Camera|Lock-On")
	void UnlockTarget();

	/** Find nearest valid lock-on target within range */
	UFUNCTION(BlueprintCallable, Category = "Camera|Lock-On")
	AActor* FindNearestLockOnTarget(float MaxDistance = 2000.0f);

	/** Switch to next/previous lock-on target */
	UFUNCTION(BlueprintCallable, Category = "Camera|Lock-On")
	void SwitchLockOnTarget(bool bNext = true);

	// Blueprint Events

	/** Called when camera mode changes */
	UFUNCTION(BlueprintImplementableEvent, Category = "Camera|Events")
	void OnCameraModeChanged(ECameraMode OldMode, ECameraMode NewMode);

	/** Called when target is locked */
	UFUNCTION(BlueprintImplementableEvent, Category = "Camera|Events")
	void OnTargetLocked(AActor* Target);

	/** Called when target is unlocked */
	UFUNCTION(BlueprintImplementableEvent, Category = "Camera|Events")
	void OnTargetUnlocked();

protected:
	// Cached References

	/** Cached owner character reference */
	UPROPERTY()
	ACursedAngelCharacter* OwnerCharacter;

	// Camera State

	/** Current camera location (smoothed) */
	FVector CurrentCameraLocation;

	/** Current camera rotation (smoothed) */
	FRotator CurrentCameraRotation;

	/** Current interpolated distance */
	float CurrentDistance;

	/** Current interpolated height */
	float CurrentHeight;

	/** Previous camera mode (for reverting from lock-on) */
	ECameraMode PreviousCameraMode;

	/** Transition alpha for smooth mode switching */
	float TransitionAlpha;

	// Update Functions

	/** Update camera mode based on gameplay context */
	void UpdateCameraMode(float DeltaTime);

	/** Update Free camera mode */
	void UpdateFreeCamera(float DeltaTime);

	/** Update Combat camera mode */
	void UpdateCombatCamera(float DeltaTime);

	/** Update Lock-On camera mode */
	void UpdateLockOnCamera(float DeltaTime);

	/** Update Platforming camera mode */
	void UpdatePlatformingCamera(float DeltaTime);

	/** Smooth transition between camera settings */
	void SmoothTransitionCamera(float DeltaTime, const FCameraModeSettings& TargetSettings);

	/** Handle collision avoidance using sweep traces */
	void HandleCollisionAvoidance(float DeltaTime);

	// Query Functions

	/** Check if character is in combat */
	bool IsInCombat() const;

	/** Check if character is in air */
	bool IsInAir() const;

	/** Get ideal camera location for current settings */
	FVector GetIdealCameraLocation(const FCameraModeSettings& Settings) const;

	/** Get ideal camera rotation for current settings */
	FRotator GetIdealCameraRotation(const FCameraModeSettings& Settings) const;
};
