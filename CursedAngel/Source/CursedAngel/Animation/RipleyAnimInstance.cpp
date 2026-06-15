// Copyright Epic Games, Inc. All Rights Reserved.

#include "Animation/RipleyAnimInstance.h"
#include "Characters/Ripley.h"
#include "Components/CursedAngelComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Physics/SoftBodyPhysicsComponent.h"

void URipleyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// Cache the owning ARipley character
	RipleyOwner = Cast<ARipley>(TryGetPawnOwner());

	if (RipleyOwner)
	{
		// Cache all 6 SoftBody component references from ARipley
		SoftBody_BreastL = RipleyOwner->GetSoftBody_BreastL();
		SoftBody_BreastR = RipleyOwner->GetSoftBody_BreastR();
		SoftBody_ThighL  = RipleyOwner->GetSoftBody_ThighL();
		SoftBody_ThighR  = RipleyOwner->GetSoftBody_ThighR();
		SoftBody_ButtL   = RipleyOwner->GetSoftBody_ButtL();
		SoftBody_ButtR   = RipleyOwner->GetSoftBody_ButtR();
	}
}

void URipleyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// Lazy-init: TryGetPawnOwner() may return null during NativeInitializeAnimation
	if (!RipleyOwner)
	{
		RipleyOwner = Cast<ARipley>(TryGetPawnOwner());
		if (RipleyOwner)
		{
			SoftBody_BreastL = RipleyOwner->GetSoftBody_BreastL();
			SoftBody_BreastR = RipleyOwner->GetSoftBody_BreastR();
			SoftBody_ThighL  = RipleyOwner->GetSoftBody_ThighL();
			SoftBody_ThighR  = RipleyOwner->GetSoftBody_ThighR();
			SoftBody_ButtL   = RipleyOwner->GetSoftBody_ButtL();
			SoftBody_ButtR   = RipleyOwner->GetSoftBody_ButtR();
		}
	}

	if (!RipleyOwner)
	{
		return;
	}

	// Update transformation state from the CursedAngelComponent
	UCursedAngelComponent* CursedAngelComp = RipleyOwner->GetCursedAngelComponent();
	if (CursedAngelComp)
	{
		bIsTransformed = CursedAngelComp->IsTransformed();
	}

	// Convert world-space SoftBody positions to component space on the game thread.
	// Modify Bone nodes in the AnimGraph use BCS_ComponentSpace + BMM_Replace,
	// so we must provide component-space coordinates (not world-space).
	if (USkeletalMeshComponent* Mesh = GetOwningComponent())
	{
		const FTransform CompInvTransform = Mesh->GetComponentTransform().Inverse();

		if (SoftBody_BreastL) SoftBodyCS_BreastL = CompInvTransform.TransformPosition(SoftBody_BreastL->GetCurrentPosition());
		if (SoftBody_BreastR) SoftBodyCS_BreastR = CompInvTransform.TransformPosition(SoftBody_BreastR->GetCurrentPosition());
		if (SoftBody_ThighL)  SoftBodyCS_ThighL  = CompInvTransform.TransformPosition(SoftBody_ThighL->GetCurrentPosition());
		if (SoftBody_ThighR)  SoftBodyCS_ThighR  = CompInvTransform.TransformPosition(SoftBody_ThighR->GetCurrentPosition());
		if (SoftBody_ButtL)   SoftBodyCS_ButtL   = CompInvTransform.TransformPosition(SoftBody_ButtL->GetCurrentPosition());
		if (SoftBody_ButtR)   SoftBodyCS_ButtR   = CompInvTransform.TransformPosition(SoftBody_ButtR->GetCurrentPosition());
	}
}
