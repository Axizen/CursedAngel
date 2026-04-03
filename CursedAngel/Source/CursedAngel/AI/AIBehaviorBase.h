// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "AIBehaviorBase.generated.h"

class AActor;
class ACursedAngelCharacter;

/**
 * Abstract base class for AI behaviors
 * Mirrors ActionBase for consistency, provides Blueprint extensibility for designer-friendly AI
 * Priority system allows behavior selection (higher priority executes first)
 */
UCLASS(Abstract, Blueprintable, EditInlineNew)
class CURSEDANGEL_API UAIBehaviorBase : public UObject
{
	GENERATED_BODY()

public:
	/** Name of this behavior */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
	FName BehaviorName;

	/** The actor that owns this behavior */
	UPROPERTY(BlueprintReadOnly, Category = "AI Behavior")
	TObjectPtr<AActor> OwnerActor;

	/** Whether this behavior is currently active */
	UPROPERTY(BlueprintReadOnly, Category = "AI Behavior")
	bool bIsActive = false;

	/** Higher priority behaviors execute first */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior", meta = (ToolTip = "Higher priority behaviors execute first"))
	int32 Priority = 0;

public:
	// Core virtual functions
	
	/** Called when the behavior is activated */
	virtual void OnActivate();

	/** Called every frame while the behavior is active */
	virtual void OnTick(float DeltaTime);

	/** Called when the behavior is deactivated */
	virtual void OnDeactivate();

	/** Check if this behavior can be activated */
	virtual bool CanActivate();

	// Blueprint events

	/** Blueprint event called when behavior is activated */
	UFUNCTION(BlueprintImplementableEvent, Category = "AI Behavior", meta = (DisplayName = "On Activate"))
	void BP_OnActivate();

	/** Blueprint event called every frame while behavior is active */
	UFUNCTION(BlueprintImplementableEvent, Category = "AI Behavior", meta = (DisplayName = "On Tick"))
	void BP_OnTick(float DeltaTime);

	/** Blueprint event called when behavior is deactivated */
	UFUNCTION(BlueprintImplementableEvent, Category = "AI Behavior", meta = (DisplayName = "On Deactivate"))
	void BP_OnDeactivate();

	/** Blueprint native event to check if behavior can activate */
	UFUNCTION(BlueprintNativeEvent, Category = "AI Behavior", meta = (DisplayName = "Can Activate"))
	bool BP_CanActivate();

	// Helper functions

	/** Get the owner as a CursedAngelCharacter */
	UFUNCTION(BlueprintCallable, Category = "AI Behavior")
	ACursedAngelCharacter* GetOwnerCharacter();

	/** Get the player character */
	UFUNCTION(BlueprintCallable, Category = "AI Behavior")
	ACursedAngelCharacter* GetPlayerCharacter();

	/** Move owner to a specific location */
	UFUNCTION(BlueprintCallable, Category = "AI Behavior")
	void MoveToLocation(FVector Location);

	/** Move owner to a specific actor */
	UFUNCTION(BlueprintCallable, Category = "AI Behavior")
	void MoveToActor(AActor* Target);
};
