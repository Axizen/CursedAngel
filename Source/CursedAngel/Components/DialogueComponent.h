// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FrankDialogueTypes.h"
#include "DialogueComponent.generated.h"

/**
 * Component that manages Frank's dialogue system
 * Handles context detection, dialogue selection, cooldowns, and queueing
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CURSEDANGEL_API UDialogueComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDialogueComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	UDataTable* DialogueTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FDialogueConfig Config;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	EDialogueContext CurrentContext = EDialogueContext::Exploration;

	// Core functions
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void TriggerDialogue(EDialogueContext Context, bool bImmediate = false);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	FDialogueLine GetRandomDialogueForContext(EDialogueContext Context);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void UpdateContext(EDialogueContext NewContext);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool CanTriggerDialogue() const;

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void ClearDialogueQueue();

	// Blueprint events
	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void OnDialogueTriggered(const FDialogueLine& Line);

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void OnDialogueComplete();

protected:
	// Internal state
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	TArray<FDialogueLine> DialogueQueue;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	float LastDialogueTime = -999.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	bool bIsPlayingDialogue = false;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	float CurrentDialogueTimeRemaining = 0.0f;

	// Internal functions
	void ProcessDialogueQueue(float DeltaTime);
	void PlayDialogueLine(const FDialogueLine& Line);
	void CompleteCurrentDialogue();
	TArray<FDialogueLine*> GetDialogueLinesForContext(EDialogueContext Context);
	bool IsDialogueOnCooldown(const FDialogueLine& Line) const;
};
