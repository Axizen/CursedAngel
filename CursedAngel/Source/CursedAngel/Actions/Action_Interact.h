// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ActionBase.h"
#include "Action_Interact.generated.h"

/**
 * UAction_Interact
 * Generic interaction action for interacting with objects in the world.
 * Uses sphere trace to find interactable actors with "Interactable" tag.
 * Blueprint logic per interactable type via BP_OnInteract event.
 */
UCLASS(Blueprintable)
class CURSEDANGEL_API UAction_Interact : public UActionBase
{
	GENERATED_BODY()

public:
	/** Maximum distance to detect interactable actors */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractDistance = 200.0f;

	/** Radius of sphere trace for detecting interactables */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractRadius = 50.0f;

	/** Called when activation occurs */
	virtual void OnActivate() override;

	/** Blueprint event called when an interactable actor is found */
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction", meta = (DisplayName = "On Interact"))
	void BP_OnInteract(AActor* InteractableActor);

protected:
	/** Find the closest interactable actor within range */
	AActor* FindInteractable();
};
