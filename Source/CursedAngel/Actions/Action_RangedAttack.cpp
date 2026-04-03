// Copyright Epic Games, Inc. All Rights Reserved.

#include "Action_RangedAttack.h"
#include "ActionDataAsset.h"
#include "Components/CurseWeaponComponent.h"
#include "Characters/CursedAngelCharacter.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"

UAction_RangedAttack::UAction_RangedAttack()
	: WeaponType(ECurseWeaponType(0)) // Default to first enum value
	, bSwitchWeapon(false)
{
}

void UAction_RangedAttack::OnActivate()
{
	if (!OwnerActor)
	{
		return;
	}

	// Get CurseWeaponComponent from owner
	UCurseWeaponComponent* CurseWeaponComp = OwnerActor->FindComponentByClass<UCurseWeaponComponent>();
	if (!CurseWeaponComp)
	{
		// Owner doesn't have CurseWeaponComponent
		bIsActive = false;
		return;
	}

	// Switch weapon if requested
	if (bSwitchWeapon)
	{
		CurseWeaponComp->SwitchWeapon(WeaponType);
	}

	// Fire the curse weapon
	CurseWeaponComp->FireCurseWeapon();

	// Play montage if specified
	if (ActionData && ActionData->ActionMontage)
	{
		PlayMontage(ActionData->ActionMontage);
	}

	// Set action as active
	bIsActive = true;

	// Ranged attacks are typically instant actions
	// The montage (if any) will play, but the action itself completes immediately
	// Override OnTick if you need duration-based behavior
}

void UAction_RangedAttack::OnCancel()
{
	// Stop montage if playing
	if (ActionData && ActionData->ActionMontage && OwnerActor)
	{
		ACursedAngelCharacter* Character = GetOwnerCharacter();
		if (Character && Character->GetMesh())
		{
			UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
			if (AnimInstance && AnimInstance->Montage_IsPlaying(ActionData->ActionMontage))
			{
				AnimInstance->Montage_Stop(0.2f, ActionData->ActionMontage);
			}
		}
	}

	bIsActive = false;
}
