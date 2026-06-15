// Copyright Epic Games, Inc. All Rights Reserved.

#include "Animation/RipleyLinkedAnimInstance.h"
#include "Animation/RipleyAnimInstance.h"
#include "Components/SkeletalMeshComponent.h"

#define CA_ANIM_LOG(Fmt, ...) \
	{ static const auto* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("ca.AnimDebug")); \
	  if (CVar && CVar->GetInt() != 0) \
		UE_LOG(LogTemp, Log, TEXT("[CAAnimDebug] " Fmt), ##__VA_ARGS__); }

void URipleyLinkedAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// Obtain the main animation instance from the owning skeletal mesh component.
	// For linked animation layers, GetOwningComponent()->GetAnimInstance() returns the primary anim instance.
	if (USkeletalMeshComponent* MeshComp = GetOwningComponent())
	{
		MainInstance = Cast<URipleyAnimInstance>(MeshComp->GetAnimInstance());
	}

	if (MainInstance)
	{
		CA_ANIM_LOG("RipleyLinkedAnim NativeInitializeAnimation: MainInstance acquired -> %s", *MainInstance->GetName());
	}
	else
	{
		CA_ANIM_LOG("RipleyLinkedAnim NativeInitializeAnimation: MainInstance is NULL (will retry in Update)");
	}
}

void URipleyLinkedAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!MainInstance)
	{
		if (USkeletalMeshComponent* MeshComp = GetOwningComponent())
		{
			MainInstance = Cast<URipleyAnimInstance>(MeshComp->GetAnimInstance());
			if (MainInstance)
			{
				CA_ANIM_LOG("RipleyLinkedAnim NativeUpdateAnimation: MainInstance acquired lazily -> %s", *MainInstance->GetName());
			}
			else
			{
				CA_ANIM_LOG("RipleyLinkedAnim NativeUpdateAnimation: MainInstance still NULL");
			}
		}
	}

	if (!MainInstance)
	{
		return;
	}

	// Mirror locomotion state from the main instance so AnimGraph transitions work correctly
	GroundSpeed = MainInstance->GroundSpeed;
	bIsInAir    = MainInstance->bIsInAir;

	// Mirror pre-computed component-space SoftBody positions from the main instance.
	// These are safe to read here (game thread) and safe to access in the AnimGraph (plain FVectors).
	SoftBodyCS_BreastL = MainInstance->SoftBodyCS_BreastL;
	SoftBodyCS_BreastR = MainInstance->SoftBodyCS_BreastR;
	SoftBodyCS_ThighL  = MainInstance->SoftBodyCS_ThighL;
	SoftBodyCS_ThighR  = MainInstance->SoftBodyCS_ThighR;
	SoftBodyCS_ButtL   = MainInstance->SoftBodyCS_ButtL;
	SoftBodyCS_ButtR   = MainInstance->SoftBodyCS_ButtR;
}
