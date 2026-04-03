// Copyright Epic Games, Inc. All Rights Reserved.

#include "Actions/Action_CursedAngel.h"
#include "Data/ActionDataAsset.h"
#include "Components/CursedAngelComponent.h"
#include "Characters/CursedAngelCharacter.h"
#include "Animation/AnimMontage.h"

bool UAction_CursedAngel::CanActivate() const
{
	if (!Super::CanActivate())
	{
		return false;
	}

	ACursedAngelCharacter* Character = GetOwnerCharacter();
	if (!Character)
	{
		return false;
	}

	UCursedAngelComponent* CursedAngelComp = Character->FindComponentByClass<UCursedAngelComponent>();
	if (!CursedAngelComp)
	{
		return false;
	}

	// Check if we can activate or deactivate based on current state
	if (bActivateTransformation)
	{
		// Want to activate - check if component allows activation
		return CursedAngelComp->CanTransform();
	}
	else
	{
		// Want to deactivate - check if currently transformed
		return CursedAngelComp->IsTransformed();
	}
}

void UAction_CursedAngel::OnActivate()
{
	Super::OnActivate();

	ACursedAngelCharacter* Character = GetOwnerCharacter();
	if (!Character)
	{
		return;
	}

	UCursedAngelComponent* CursedAngelComp = Character->FindComponentByClass<UCursedAngelComponent>();
	if (!CursedAngelComp)
	{
		return;
	}

	// Play transformation montage if set
	if (ActionData && ActionData->ActionMontage)
	{
		PlayMontage(ActionData->ActionMontage);
	}

	// Toggle transformation state
	if (bActivateTransformation)
	{
		CursedAngelComp->ActivateTransformation();
	}
	// Note: Deactivation happens automatically after TransformationDuration
	// There is no manual deactivate method in CursedAngelComponent

	// Instant action - activate then immediately deactivate
	// The transformation state change is handled by CursedAngelComponent
	bIsActive = true;
	bIsActive = false;
}
