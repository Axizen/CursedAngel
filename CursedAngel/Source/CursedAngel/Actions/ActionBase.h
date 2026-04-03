// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "ActionBase.generated.h"

class UActionDataAsset;
class ACursedAngelCharacter;
class UAnimMontage;

/**
 * Abstract base class for all actions in the action system.
 * Provides Blueprint extensibility and common functionality.
 */
UCLASS(Abstract, Blueprintable, EditInlineNew)
class CURSEDANGEL_API UActionBase : public UObject
{
	GENERATED_BODY()

public:
	UActionBase();

	// ===== Properties =====

	/** Reference to the action data asset that created this action */
	UPROPERTY(BlueprintReadOnly, Category = "Action")
	TObjectPtr<UActionDataAsset> ActionData;

	/** The actor that owns and is executing this action */
	UPROPERTY(BlueprintReadOnly, Category = "Action")
	TObjectPtr<AActor> OwnerActor;

	/** Whether this action is currently active */
	UPROPERTY(BlueprintReadOnly, Category = "Action")
	bool bIsActive;

	/** Current cooldown remaining for this action */
	UPROPERTY(BlueprintReadOnly, Category = "Action")
	float CurrentCooldown;

	// ===== Virtual Functions =====

	/** Called when the action is activated */
	virtual void OnActivate();

	/** Called every frame while the action is active */
	virtual void OnTick(float DeltaTime);

	/** Called when the action is cancelled or interrupted */
	virtual void OnCancel();

	/** Check if the action can be activated */
	virtual bool CanActivate() const;

	// ===== Blueprint Events =====

	/** Blueprint event called when the action is activated */
	UFUNCTION(BlueprintImplementableEvent, Category = "Action", meta = (DisplayName = "On Activate"))
	void BP_OnActivate();

	/** Blueprint event called every frame while the action is active */
	UFUNCTION(BlueprintImplementableEvent, Category = "Action", meta = (DisplayName = "On Tick"))
	void BP_OnTick(float DeltaTime);

	/** Blueprint event called when the action is cancelled */
	UFUNCTION(BlueprintImplementableEvent, Category = "Action", meta = (DisplayName = "On Cancel"))
	void BP_OnCancel();

	/** Blueprint native event to check if action can be activated */
	UFUNCTION(BlueprintNativeEvent, Category = "Action", meta = (DisplayName = "Can Activate"))
	bool BP_CanActivate() const;

	// ===== Helper Functions =====

	/** Play an animation montage on the owner's mesh */
	UFUNCTION(BlueprintCallable, Category = "Action")
	void PlayMontage(UAnimMontage* Montage);

	/** Apply damage to a target actor */
	UFUNCTION(BlueprintCallable, Category = "Action")
	void ApplyDamageToTarget(AActor* Target, float Damage);

	/** Get the owner as a CursedAngelCharacter */
	UFUNCTION(BlueprintCallable, Category = "Action")
	ACursedAngelCharacter* GetOwnerCharacter() const;
};
