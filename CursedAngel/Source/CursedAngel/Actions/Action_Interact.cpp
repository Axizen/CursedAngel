// Copyright Epic Games, Inc. All Rights Reserved.

#include "Action_Interact.h"
#include "Data/ActionDataAsset.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"

void UAction_Interact::OnActivate()
{
	// Find the closest interactable actor
	AActor* InteractableActor = FindInteractable();

	if (InteractableActor)
	{
		// Call Blueprint event for custom interaction logic
		BP_OnInteract(InteractableActor);

		// Play montage if one is set in the action data
		if (ActionData && ActionData->ActionMontage)
		{
			PlayMontage(ActionData->ActionMontage);
		}
	}

	// Interaction is an instant action - activate and immediately complete
	bIsActive = true;
	bIsActive = false;
}

AActor* UAction_Interact::FindInteractable()
{
	if (!OwnerActor)
	{
		return nullptr;
	}

	// Get the forward direction from the owner
	FVector StartLocation = OwnerActor->GetActorLocation();
	FVector ForwardVector = OwnerActor->GetActorForwardVector();
	FVector EndLocation = StartLocation + (ForwardVector * InteractDistance);

	// Setup trace parameters
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(OwnerActor);
	TArray<FHitResult> HitResults;

	// Perform sphere trace to find interactable actors
	bool bHit = UKismetSystemLibrary::SphereTraceMulti(
		OwnerActor->GetWorld(),
		StartLocation,
		EndLocation,
		InteractRadius,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false,
		ActorsToIgnore,
		EDrawDebugTrace::None,
		HitResults,
		true
	);

	if (bHit)
	{
		AActor* ClosestInteractable = nullptr;
		float ClosestDistance = FLT_MAX;

		// Find the closest actor with "Interactable" tag
		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor && HitActor->ActorHasTag(FName("Interactable")))
			{
				float Distance = FVector::Distance(StartLocation, Hit.Location);
				if (Distance < ClosestDistance)
				{
					ClosestDistance = Distance;
					ClosestInteractable = HitActor;
				}
			}
		}

		return ClosestInteractable;
	}

	return nullptr;
}
