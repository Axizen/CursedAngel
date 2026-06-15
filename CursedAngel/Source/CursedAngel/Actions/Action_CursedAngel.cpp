// Copyright Epic Games, Inc. All Rights Reserved.

#include "Actions/Action_CursedAngel.h"
#include "Data/ActionDataAsset.h"
#include "Components/CursedAngelComponent.h"
#include "Characters/CursedAngelCharacter.h"
#include "Animation/AnimMontage.h"

#define CA_ACTION_LOG(Fmt, ...) \
    { IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("ca.ActionDebug")); \
      if (CVar && CVar->GetInt()) \
          UE_LOG(LogTemp, Log, TEXT("[CAActionDebug] " Fmt), ##__VA_ARGS__); }

bool UAction_CursedAngel::CanActivate() const
{
	if (!Super::CanActivate())
	{
		return false;
	}

	ACursedAngelCharacter* Character = GetOwnerCharacter();
	if (!Character)
	{
		CA_ACTION_LOG("Action_CursedAngel::CanActivate - BLOCKED: no owner character");
		return false;
	}

	UCursedAngelComponent* CursedAngelComp = Character->FindComponentByClass<UCursedAngelComponent>();
	if (!CursedAngelComp)
	{
		CA_ACTION_LOG("Action_CursedAngel::CanActivate - BLOCKED: no CursedAngelComponent");
		return false;
	}

	// Check if we can activate or deactivate based on current state
	if (bActivateTransformation)
	{
		// Want to activate - check if component allows activation
		CA_ACTION_LOG("Action_CursedAngel::CanActivate - checking CanTransform: %s", CursedAngelComp->CanTransform() ? TEXT("YES") : TEXT("NO"));
		return CursedAngelComp->CanTransform();
	}
	else
	{
		// Want to deactivate - check if currently transformed
		CA_ACTION_LOG("Action_CursedAngel::CanActivate - checking IsTransformed: %s", CursedAngelComp->IsTransformed() ? TEXT("YES") : TEXT("NO"));
		return CursedAngelComp->IsTransformed();
	}
}

void UAction_CursedAngel::OnActivate()
{
	Super::OnActivate();

	CA_ACTION_LOG("Action_CursedAngel::OnActivate - STARTED on %s", OwnerActor ? *OwnerActor->GetName() : TEXT("NULL"));

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
		CA_ACTION_LOG("Action_CursedAngel::OnActivate - Playing montage: %s", *ActionData->ActionMontage->GetName());
	}

	// Toggle transformation state
	if (bActivateTransformation)
	{
		CursedAngelComp->ActivateTransformation();
		CA_ACTION_LOG("Action_CursedAngel::OnActivate - ActivateTransformation called");
	}
	// Note: Deactivation happens automatically after TransformationDuration
	// There is no manual deactivate method in CursedAngelComponent

	// Instant action - activate then immediately deactivate
	// The transformation state change is handled by CursedAngelComponent
	bIsActive = true;
	bIsActive = false;
}
