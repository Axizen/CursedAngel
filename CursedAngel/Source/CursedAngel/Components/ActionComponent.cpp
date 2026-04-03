// Copyright Cursed Angel. All Rights Reserved.

#include "ActionComponent.h"
#include "CursedAngel/Data/ActionDataAsset.h"
#include "CursedAngel/Actions/ActionBase.h"

UActionComponent::UActionComponent()
{
	// Enable ticking for cooldown updates and action processing
	PrimaryComponentTick.bCanEverTick = true;
}

void UActionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update action cooldowns
	TArray<FName> CooldownKeys;
	ActionCooldowns.GetKeys(CooldownKeys);
	for (const FName& ActionName : CooldownKeys)
	{
		float& Cooldown = ActionCooldowns[ActionName];
		Cooldown -= DeltaTime;
		if (Cooldown <= 0.0f)
		{
			ActionCooldowns.Remove(ActionName);
		}
	}

	// Tick the current action if active
	if (CurrentAction && CurrentAction->bIsActive)
	{
		CurrentAction->OnTick(DeltaTime);
		CurrentAction->BP_OnTick(DeltaTime);
	}
	// If current action finished, process the queue
	else if (CurrentAction && !CurrentAction->bIsActive)
	{
		ClearCurrentAction();
		ProcessActionQueue();
	}
}

bool UActionComponent::ExecuteAction(FName ActionName)
{
	// Find the action data asset
	UActionDataAsset* ActionData = GetAction(ActionName);
	if (!ActionData)
	{
		UE_LOG(LogTemp, Warning, TEXT("ActionComponent::ExecuteAction - Action not found: %s"), *ActionName.ToString());
		return false;
	}

	// Check if action can be executed
	if (!CanExecuteAction(ActionData))
	{
		return false;
	}

	// Cancel current action if one is active
	if (CurrentAction)
	{
		CancelCurrentAction();
	}

	// Instantiate the action from the action class
	if (!ActionData->ActionClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ActionComponent::ExecuteAction - Action class is null for: %s"), *ActionName.ToString());
		return false;
	}

	CurrentAction = NewObject<UActionBase>(this, ActionData->ActionClass);
	if (!CurrentAction)
	{
		UE_LOG(LogTemp, Error, TEXT("ActionComponent::ExecuteAction - Failed to instantiate action: %s"), *ActionName.ToString());
		return false;
	}

	// Initialize the action
	CurrentAction->ActionData = ActionData;
	CurrentAction->OwnerActor = GetOwner();
	CurrentAction->bIsActive = true;

	// Add granted tags
	for (const FGameplayTag& Tag : ActionData->GrantedTags)
	{
		AddTag(Tag);
	}

	// Set cooldown
	if (ActionData->Cooldown > 0.0f)
	{
		ActionCooldowns.Add(ActionName, ActionData->Cooldown);
	}

	// Activate the action
	CurrentAction->OnActivate();
	CurrentAction->BP_OnActivate();

	return true;
}

void UActionComponent::QueueAction(FName ActionName)
{
	// Add action to queue if not already present
	if (!ActionQueue.Contains(ActionName))
	{
		ActionQueue.Add(ActionName);
	}
}

void UActionComponent::CancelCurrentAction()
{
	if (!CurrentAction)
	{
		return;
	}

	// Call cancel on the action
	if (CurrentAction->bIsActive)
	{
		CurrentAction->OnCancel();
		CurrentAction->BP_OnCancel();
		CurrentAction->bIsActive = false;
	}

	// Clear current action
	ClearCurrentAction();
}

UActionDataAsset* UActionComponent::GetAction(FName ActionName) const
{
	for (UActionDataAsset* Action : AvailableActions)
	{
		if (Action && Action->ActionName == ActionName)
		{
			return Action;
		}
	}
	return nullptr;
}

bool UActionComponent::CanExecuteAction(const UActionDataAsset* ActionData) const
{
	if (!ActionData)
	{
		return false;
	}

	// Check cooldown
	if (ActionCooldowns.Contains(ActionData->ActionName))
	{
		return false;
	}

	// Check required tags - character must have ALL required tags
	if (!ActionData->RequiredTags.IsEmpty())
	{
		if (!ActiveTags.HasAll(ActionData->RequiredTags))
		{
			return false;
		}
	}

	// Check blocked tags - character must have NONE of the blocked tags
	if (!ActionData->BlockedTags.IsEmpty())
	{
		if (ActiveTags.HasAny(ActionData->BlockedTags))
		{
			return false;
		}
	}

	// Additional check via action's CanActivate
	// Note: We can't instantiate the action here just to check, so we rely on data tags
	// Individual actions can override CanActivate for runtime checks

	return true;
}

void UActionComponent::AddTag(FGameplayTag Tag)
{
	if (Tag.IsValid())
	{
		ActiveTags.AddTag(Tag);
	}
}

void UActionComponent::RemoveTag(FGameplayTag Tag)
{
	if (Tag.IsValid())
	{
		ActiveTags.RemoveTag(Tag);
	}
}

bool UActionComponent::HasTag(FGameplayTag Tag) const
{
	return ActiveTags.HasTag(Tag);
}

void UActionComponent::ProcessActionQueue()
{
	// Execute the next queued action if any
	if (ActionQueue.Num() > 0)
	{
		FName NextAction = ActionQueue[0];
		ActionQueue.RemoveAt(0);
		ExecuteAction(NextAction);
	}
}

void UActionComponent::ClearCurrentAction()
{
	if (!CurrentAction)
	{
		return;
	}

	// Remove granted tags
	if (CurrentAction->ActionData)
	{
		for (const FGameplayTag& Tag : CurrentAction->ActionData->GrantedTags)
		{
			RemoveTag(Tag);
		}
	}

	// Clear the current action reference
	CurrentAction = nullptr;
}
