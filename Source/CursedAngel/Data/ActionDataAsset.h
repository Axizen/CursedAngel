// Copyright Cursed Angel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ActionDataAsset.generated.h"

// Forward declarations
class UActionBase;
class UAnimMontage;

/**
 * UActionDataAsset
 * Data-driven configuration for actions in the hybrid action system.
 * Stores action metadata, requirements, and granted/blocked tags for combo chains.
 */
UCLASS(BlueprintType)
class CURSEDANGEL_API UActionDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Unique name identifier for this action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	FName ActionName;

	/** The action class to instantiate when executing this action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action", meta = (MustImplement = "ActionBase"))
	TSubclassOf<UActionBase> ActionClass;

	/** Optional animation montage to play when this action activates */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* ActionMontage;

	/** Cooldown duration in seconds before this action can be executed again */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	float Cooldown = 0.0f;

	/** Tags the character must have to execute this action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags")
	FGameplayTagContainer RequiredTags;

	/** Tags granted to the character when this action activates (e.g., Attacking, Dodging) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags")
	FGameplayTagContainer GrantedTags;

	/** Tags that prevent this action from executing (e.g., Stunned, Dodging blocks attacks) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags")
	FGameplayTagContainer BlockedTags;

	/** Override to provide unique asset ID for asset manager */
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
