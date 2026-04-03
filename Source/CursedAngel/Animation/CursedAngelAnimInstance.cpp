// Copyright Epic Games, Inc. All Rights Reserved.

#include "Animation/CursedAngelAnimInstance.h"
#include "Characters/CursedAngelCharacter.h"
#include "Components/ActionComponent.h"
#include "Components/CurseWeaponComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UCursedAngelAnimInstance::UCursedAngelAnimInstance()
{
	// Initialize default values
	CurrentForm = EAnimationForm::Normal;
	Speed = 0.0f;
	Direction = 0.0f;
	bIsInAir = false;
	bIsCrouching = false;
	GroundSpeed = 0.0f;
	bIsAttacking = false;
	bIsDodging = false;
	bIsBlocking = false;
	bIsStunned = false;
	bIsAiming = false;
	ComboIndex = 0;
	LeftFootIKOffset = 0.0f;
	RightFootIKOffset = 0.0f;
	HipOffset = 0.0f;
}

void UCursedAngelAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// Cache owner character reference
	OwnerCharacter = Cast<ACursedAngelCharacter>(TryGetPawnOwner());

	if (OwnerCharacter)
	{
		// Initialize to normal form
		SwitchToNormalForm();
	}
}

void UCursedAngelAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (OwnerCharacter)
	{
		UpdateAnimationVariables();
	}
}

void UCursedAngelAnimInstance::SwitchToNormalForm()
{
	// Already in normal form
	if (CurrentForm == EAnimationForm::Normal)
	{
		return;
	}

	// Set current form
	CurrentForm = EAnimationForm::Normal;

	// Link normal form layer class if valid
	if (NormalFormLayerClass)
	{
		LinkAnimationLayer(NormalFormLayerClass, FName("FullBodySlot"));
	}

	// Swap mesh to normal form
	SwapMesh(EAnimationForm::Normal);

	// Call Blueprint event
	OnFormChanged(CurrentForm);

	UE_LOG(LogTemp, Log, TEXT("CursedAngelAnimInstance: Switched to Normal Form"));
}

void UCursedAngelAnimInstance::SwitchToCursedAngelForm()
{
	// Already in cursed angel form
	if (CurrentForm == EAnimationForm::CursedAngel)
	{
		return;
	}

	// Set current form
	CurrentForm = EAnimationForm::CursedAngel;

	// Link cursed angel form layer class if valid
	if (CursedAngelFormLayerClass)
	{
		LinkAnimationLayer(CursedAngelFormLayerClass, FName("FullBodySlot"));
	}

	// Swap mesh to cursed angel form
	SwapMesh(EAnimationForm::CursedAngel);

	// Call Blueprint event
	OnFormChanged(CurrentForm);

	UE_LOG(LogTemp, Log, TEXT("CursedAngelAnimInstance: Switched to Cursed Angel Form"));
}

void UCursedAngelAnimInstance::SwapMesh(EAnimationForm Form)
{
	if (!OwnerCharacter)
	{
		return;
	}

	USkeletalMeshComponent* MeshComponent = OwnerCharacter->GetMesh();
	if (!MeshComponent)
	{
		return;
	}

	// Select mesh based on form
	USkeletalMesh* TargetMesh = nullptr;
	if (Form == EAnimationForm::Normal)
	{
		TargetMesh = NormalFormMesh;
	}
	else if (Form == EAnimationForm::CursedAngel)
	{
		TargetMesh = CursedAngelFormMesh;
	}

	// Swap the mesh
	if (TargetMesh)
	{
		MeshComponent->SetSkeletalMesh(TargetMesh);
		UE_LOG(LogTemp, Log, TEXT("CursedAngelAnimInstance: Swapped mesh to %s"), *TargetMesh->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CursedAngelAnimInstance: Target mesh is null for form %d"), static_cast<int32>(Form));
	}
}

void UCursedAngelAnimInstance::UpdateAnimationVariables()
{
	UpdateLocomotionVariables();
	UpdateCombatVariables();
	UpdateFootIK();
}

void UCursedAngelAnimInstance::UpdateLocomotionVariables()
{
	if (!OwnerCharacter)
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	// Get velocity and calculate speed
	Velocity = MovementComponent->Velocity;
	Speed = Velocity.Size();

	// Calculate direction relative to actor rotation
	if (Speed > 0.0f)
	{
		Direction = CalculateDirection(Velocity, OwnerCharacter->GetActorRotation());
	}
	else
	{
		Direction = 0.0f;
	}

	// Get ground speed (2D velocity)
	FVector Velocity2D = Velocity;
	Velocity2D.Z = 0.0f;
	GroundSpeed = Velocity2D.Size();

	// Check if in air
	bIsInAir = MovementComponent->IsFalling();

	// Check if crouching
	bIsCrouching = MovementComponent->IsCrouching();
}

void UCursedAngelAnimInstance::UpdateCombatVariables()
{
	if (!OwnerCharacter)
	{
		return;
	}

	// Get action component
	UActionComponent* ActionComponent = OwnerCharacter->FindComponentByClass<UActionComponent>();
	if (ActionComponent)
	{
		// Query gameplay tags for combat states
		bIsAttacking = ActionComponent->HasTag(FGameplayTag::RequestGameplayTag(FName("State.Attacking")));
		bIsDodging = ActionComponent->HasTag(FGameplayTag::RequestGameplayTag(FName("State.Dodging")));
		bIsBlocking = ActionComponent->HasTag(FGameplayTag::RequestGameplayTag(FName("State.Blocking")));
		bIsStunned = ActionComponent->HasTag(FGameplayTag::RequestGameplayTag(FName("State.Stunned")));
	}

	// Get weapon component for aiming state
	UCurseWeaponComponent* WeaponComponent = OwnerCharacter->FindComponentByClass<UCurseWeaponComponent>();
	if (WeaponComponent)
	{
		// Check if aiming (weapon component would have this state)
		bIsAiming = ActionComponent ? ActionComponent->HasTag(FGameplayTag::RequestGameplayTag(FName("State.Aiming"))) : false;
	}

	// Combo index would be set externally by combat system
	// This is just reading the value set by other systems
}

void UCursedAngelAnimInstance::UpdateFootIK()
{
	if (!OwnerCharacter)
	{
		return;
	}

	USkeletalMeshComponent* MeshComponent = OwnerCharacter->GetMesh();
	if (!MeshComponent)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Get foot socket locations
	FVector LeftFootLocation = MeshComponent->GetSocketLocation(FName("foot_l"));
	FVector RightFootLocation = MeshComponent->GetSocketLocation(FName("foot_r"));

	// Trace distance
	const float TraceDistance = 100.0f;
	const float IKInterpSpeed = 10.0f;

	// Trace for left foot
	FHitResult LeftHitResult;
	FVector LeftTraceStart = LeftFootLocation + FVector(0.0f, 0.0f, 50.0f);
	FVector LeftTraceEnd = LeftFootLocation - FVector(0.0f, 0.0f, TraceDistance);
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);

	bool bLeftHit = World->LineTraceSingleByChannel(
		LeftHitResult,
		LeftTraceStart,
		LeftTraceEnd,
		ECC_Visibility,
		QueryParams
	);

	// Trace for right foot
	FHitResult RightHitResult;
	FVector RightTraceStart = RightFootLocation + FVector(0.0f, 0.0f, 50.0f);
	FVector RightTraceEnd = RightFootLocation - FVector(0.0f, 0.0f, TraceDistance);

	bool bRightHit = World->LineTraceSingleByChannel(
		RightHitResult,
		RightTraceStart,
		RightTraceEnd,
		ECC_Visibility,
		QueryParams
	);

	// Calculate IK offsets
	if (bLeftHit)
	{
		float TargetOffset = (LeftHitResult.ImpactPoint.Z - LeftFootLocation.Z);
		LeftFootIKOffset = FMath::FInterpTo(LeftFootIKOffset, TargetOffset, World->GetDeltaSeconds(), IKInterpSpeed);

		// Calculate foot rotation from normal
		FRotator TargetRotation = UKismetMathLibrary::MakeRotFromX(LeftHitResult.ImpactNormal);
		LeftFootRotation = FMath::RInterpTo(LeftFootRotation, TargetRotation, World->GetDeltaSeconds(), IKInterpSpeed);
	}
	else
	{
		LeftFootIKOffset = FMath::FInterpTo(LeftFootIKOffset, 0.0f, World->GetDeltaSeconds(), IKInterpSpeed);
		LeftFootRotation = FMath::RInterpTo(LeftFootRotation, FRotator::ZeroRotator, World->GetDeltaSeconds(), IKInterpSpeed);
	}

	if (bRightHit)
	{
		float TargetOffset = (RightHitResult.ImpactPoint.Z - RightFootLocation.Z);
		RightFootIKOffset = FMath::FInterpTo(RightFootIKOffset, TargetOffset, World->GetDeltaSeconds(), IKInterpSpeed);

		// Calculate foot rotation from normal
		FRotator TargetRotation = UKismetMathLibrary::MakeRotFromX(RightHitResult.ImpactNormal);
		RightFootRotation = FMath::RInterpTo(RightFootRotation, TargetRotation, World->GetDeltaSeconds(), IKInterpSpeed);
	}
	else
	{
		RightFootIKOffset = FMath::FInterpTo(RightFootIKOffset, 0.0f, World->GetDeltaSeconds(), IKInterpSpeed);
		RightFootRotation = FMath::RInterpTo(RightFootRotation, FRotator::ZeroRotator, World->GetDeltaSeconds(), IKInterpSpeed);
	}

	// Calculate hip offset (minimum of both feet to keep character grounded)
	HipOffset = FMath::Min(LeftFootIKOffset, RightFootIKOffset);
	if (HipOffset < 0.0f)
	{
		HipOffset = 0.0f;
	}
}

void UCursedAngelAnimInstance::LinkAnimationLayer(TSubclassOf<UAnimInstance> LayerClass, FName SlotName)
{
	if (!LayerClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("CursedAngelAnimInstance: Cannot link null layer class"));
		return;
	}

	// Link the animation class layers
	LinkAnimClassLayers(LayerClass);

	UE_LOG(LogTemp, Log, TEXT("CursedAngelAnimInstance: Linked animation layer %s"), *LayerClass->GetName());
}

void UCursedAngelAnimInstance::PlayMontageWithBlend(UAnimMontage* Montage, float BlendInTime)
{
	if (!Montage)
	{
		UE_LOG(LogTemp, Warning, TEXT("CursedAngelAnimInstance: Cannot play null montage"));
		return;
	}

	// Play montage with custom blend time
	Montage_Play(Montage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
}

void UCursedAngelAnimInstance::StopMontageWithBlend(UAnimMontage* Montage, float BlendOutTime)
{
	if (!Montage)
	{
		UE_LOG(LogTemp, Warning, TEXT("CursedAngelAnimInstance: Cannot stop null montage"));
		return;
	}

	// Stop montage with custom blend time
	Montage_Stop(BlendOutTime, Montage);
}
