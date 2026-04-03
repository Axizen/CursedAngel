// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actions/ActionBase.h"
#include "Action_CursedAngel.generated.h"

/**
 * Action for activating/deactivating Cursed Angel transformation
 * Integrates with UCursedAngelComponent for transformation state management
 * Instant action (no duration, just state change)
 */
UCLASS(Blueprintable)
class CURSEDANGEL_API UAction_CursedAngel : public UActionBase
{
	GENERATED_BODY()

public:
	/** True to activate transformation, false to deactivate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CursedAngel")
	bool bActivateTransformation = true;

	//~ Begin UActionBase Interface
	virtual void OnActivate() override;
	virtual bool CanActivate() const override;
	//~ End UActionBase Interface
};
