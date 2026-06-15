// Copyright Cursed Angel. All Rights Reserved.

#include "ActionComponent.h"
#include "CursedAngel/Data/ActionDataAsset.h"
#include "CursedAngel/Actions/ActionBase.h"

static TAutoConsoleVariable<int32> CVarActionDebug(
	TEXT("ca.ActionDebug"),
	0,
	TEXT("Enable CursedAngel action system debug logging (0=off, 1=on)"),
	ECVF_Default
);

#define CA_ACTION_LOG(Fmt, ...) \
	if (CVarActionDebug.GetValueOnGameThread()) \
		UE_LOG(LogTemp, Log, TEXT("[CAActionDebug] " Fmt), ##__VA_ARGS__)

UActionComponent::UActionComponent()
{
	// Enable ticking for cooldown updates, combo buffer pruning, and action tick
	PrimaryComponentTick.bCanEverTick = true;
}

void UActionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Programmer: No CharacterDataAsset reference on this component — ComboWindowDuration
	// is initialized to the same default as FCombatFeelConfig (0.4s). Characters that have
	// a data asset should set ComboWindowDuration explicitly after BeginPlay via
	// ActionComponent->ComboWindowDuration = DataAsset->CombatFeel.ComboWindowDuration.
}

void UActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ===== Critical Per-Frame Checks (keep minimal) =====
	// 1. Decrement action cooldowns — remove entries when they expire.
	// 2. Prune expired combo buffer entries — stale inputs are discarded silently.
	// 3. Tick the active action OR process the combo buffer when the action finishes.

	// --- 1. Update action cooldowns ---
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

	// --- 2. Prune expired combo buffer entries ---
	// Entries whose ExpiryTime has passed are removed to prevent stale inputs from firing.
	// Programmer: RemoveAll preserves order for the remaining entries (oldest-first execution).
	if (GetWorld())
	{
		const float Now = GetWorld()->GetTimeSeconds();
		ComboBuffer.RemoveAll([Now](const FBufferedAction& Entry)
		{
			return Entry.ExpiryTime < Now;
		});
	}

	// --- 3. Tick current action or process combo buffer ---
	if (CurrentAction && CurrentAction->bIsActive)
	{
		// Action is still running — tick it
		CurrentAction->OnTick(DeltaTime);
		CurrentAction->BP_OnTick(DeltaTime);
	}
	else if (CurrentAction && !CurrentAction->bIsActive)
	{
		// Action just finished — clean up and try to chain from the combo buffer
		ClearCurrentAction();
		ProcessComboBuffer();
	}
}

bool UActionComponent::ExecuteAction(FName ActionName)
{
	// Programmer: Main action execution entry point.
	// Validates data, checks gating conditions (cooldown + tags), cancels any running action,
	// instantiates the new action, applies GrantedTags + StateFlags, then activates.

	// Find the action data asset
	UActionDataAsset* ActionData = GetAction(ActionName);
	if (!ActionData)
	{
		UE_LOG(LogTemp, Warning, TEXT("ActionComponent::ExecuteAction - Action not found: %s"), *ActionName.ToString());
		return false;
	}

	// Gate: check cooldown and blocking tags
	if (!CanExecuteAction(ActionData))
	{
		CA_ACTION_LOG("ExecuteAction: '%s' BLOCKED (cooldown or tag gate) on %s", *ActionName.ToString(), *GetOwner()->GetName());
		return false;
	}

	// Cancel any running action before starting the new one
	if (CurrentAction)
	{
		CancelCurrentAction();
	}

	// Validate action class
	if (!ActionData->ActionClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ActionComponent::ExecuteAction - Action class is null for: %s"), *ActionName.ToString());
		return false;
	}

	// Instantiate the action object
	CurrentAction = NewObject<UActionBase>(this, ActionData->ActionClass);
	if (!CurrentAction)
	{
		UE_LOG(LogTemp, Error, TEXT("ActionComponent::ExecuteAction - Failed to instantiate action: %s"), *ActionName.ToString());
		return false;
	}

	// Initialize base fields
	CurrentAction->ActionData = ActionData;
	CurrentAction->OwnerActor = GetOwner();
	CurrentAction->bIsActive = true;

	// Add granted gameplay tags
	for (const FGameplayTag& Tag : ActionData->GrantedTags)
	{
		AddTag(Tag);
	}

	// Set ECharacterStateFlags bits corresponding to the granted tags.
	// Programmer: This runs AFTER AddTag so both systems stay in sync.
	ApplyStateFlagsForTags(ActionData->GrantedTags);

	// Register cooldown if applicable
	if (ActionData->Cooldown > 0.0f)
	{
		ActionCooldowns.Add(ActionName, ActionData->Cooldown);
	}

	// Fire activation callbacks (C++ first, then Blueprint override)
	CurrentAction->OnActivate();
	CurrentAction->BP_OnActivate();

	CA_ACTION_LOG("ExecuteAction: '%s' STARTED on %s", *ActionName.ToString(), *GetOwner()->GetName());

	return true;
}

void UActionComponent::BufferAction(FName ActionName)
{
	// Programmer: Creates a timed FBufferedAction entry.
	// ExpiryTime = now + ComboWindowDuration. The entry is consumed when the current action
	// ends (ProcessComboBuffer), or discarded if TickComponent prunes it first.
	// Deduplication: only one entry per action name to prevent duplicate firing.

	if (!GetWorld())
	{
		return;
	}

	// Deduplicate: if this action is already in the buffer, refresh its expiry time instead
	const float ExpiryTime = GetWorld()->GetTimeSeconds() + ComboWindowDuration;
	for (FBufferedAction& Existing : ComboBuffer)
	{
		if (Existing.ActionName == ActionName)
		{
			// Refresh expiry so the latest input press extends the window
			Existing.ExpiryTime = ExpiryTime;
			return;
		}
	}

	ComboBuffer.Add(FBufferedAction(ActionName, ExpiryTime));
}

void UActionComponent::QueueAction(FName ActionName)
{
	// [Legacy] Delegates to BufferAction for timed combo buffering.
	// Programmer: Also updates the old ActionQueue for any code still reading it.
	BufferAction(ActionName);

	// Keep legacy ActionQueue in sync for Blueprint backwards compatibility
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

	// Call cancel callbacks (C++ first, then Blueprint override)
	if (CurrentAction->bIsActive)
	{
		CurrentAction->OnCancel();
		CurrentAction->BP_OnCancel();
		CurrentAction->bIsActive = false;
	}

	// Clear action reference, remove tags and state flags
	CA_ACTION_LOG("CancelCurrentAction: '%s' CANCELLED on %s",
		(CurrentAction->ActionData ? *CurrentAction->ActionData->ActionName.ToString() : TEXT("Unknown")),
		*GetOwner()->GetName());
	ClearCurrentAction();
}

UActionDataAsset* UActionComponent::GetAction(FName ActionName) const
{
	// Programmer: Linear search through AvailableActions by name.
	// For large action sets, consider a TMap cache — fine for typical game action counts (<20).
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
	// Programmer: Gate checks run in order of cheapness.
	// 1. Null check (free)
	// 2. Cooldown check (TMap lookup)
	// 3. Required tags (FGameplayTagContainer::HasAll)
	// 4. Blocked tags (FGameplayTagContainer::HasAny)

	if (!ActionData)
	{
		return false;
	}

	// Check cooldown — action cannot fire while on cooldown
	if (ActionCooldowns.Contains(ActionData->ActionName))
	{
		return false;
	}

	// Check required tags — character must have ALL required tags
	if (!ActionData->RequiredTags.IsEmpty())
	{
		if (!ActiveTags.HasAll(ActionData->RequiredTags))
		{
			return false;
		}
	}

	// Check blocked tags — character must have NONE of the blocked tags
	if (!ActionData->BlockedTags.IsEmpty())
	{
		if (ActiveTags.HasAny(ActionData->BlockedTags))
		{
			return false;
		}
	}

	// Note: runtime checks beyond tags are handled by individual action CanActivate overrides.
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

void UActionComponent::ProcessComboBuffer()
{
	// Programmer: Called from TickComponent when CurrentAction becomes inactive.
	// Walks the buffer from oldest to newest, skipping expired entries.
	// The first non-expired entry is executed; all preceding expired entries are removed.
	// This preserves correct combo ordering (oldest buffered input fires first).

	if (!GetWorld())
	{
		return;
	}

	const float Now = GetWorld()->GetTimeSeconds();

	while (ComboBuffer.Num() > 0)
	{
		const FBufferedAction& Next = ComboBuffer[0];

		if (Next.ExpiryTime < Now)
		{
			// Entry has expired — discard and try the next one
			ComboBuffer.RemoveAt(0);
			continue;
		}

		// Valid entry — remove from buffer and execute
		const FName ActionToExecute = Next.ActionName;
		ComboBuffer.RemoveAt(0);
		ExecuteAction(ActionToExecute);
		return; // Only fire one action per frame on action end
	}
}

void UActionComponent::ApplyStateFlagsForTags(const FGameplayTagContainer& Tags)
{
	// Programmer: Maps GrantedTags to ECharacterStateFlags bits.
	// Tag naming convention: 'Action.Attacking', 'Action.Dodging', etc.
	// Both the tag system (ActiveTags) and the bitmask (StateFlags) are kept in sync
	// so consumers can use whichever is most convenient.

	for (const FGameplayTag& Tag : Tags)
	{
		const FName TagName = Tag.GetTagName();

		if (TagName == FName("Action.Attacking"))
		{
			SetStateFlag(ECharacterStateFlags::Attacking);
		}
		else if (TagName == FName("Action.Dodging"))
		{
			SetStateFlag(ECharacterStateFlags::Dodging);
		}
		else if (TagName == FName("Action.AirDashing"))
		{
			SetStateFlag(ECharacterStateFlags::AirDashing);
		}
		else if (TagName == FName("Action.Airborne"))
		{
			SetStateFlag(ECharacterStateFlags::Airborne);
		}
		else if (TagName == FName("Action.Invulnerable"))
		{
			SetStateFlag(ECharacterStateFlags::Invulnerable);
		}
		else if (TagName == FName("Action.Stunned"))
		{
			SetStateFlag(ECharacterStateFlags::Stunned);
		}
		else if (TagName == FName("Action.Transformed"))
		{
			SetStateFlag(ECharacterStateFlags::Transformed);
		}
	}
}

void UActionComponent::ClearStateFlagsForTags(const FGameplayTagContainer& Tags)
{
	// Programmer: Inverse of ApplyStateFlagsForTags.
	// Called from ClearCurrentAction to remove state flag bits when a action's tags are ungranted.

	for (const FGameplayTag& Tag : Tags)
	{
		const FName TagName = Tag.GetTagName();

		if (TagName == FName("Action.Attacking"))
		{
			ClearStateFlag(ECharacterStateFlags::Attacking);
		}
		else if (TagName == FName("Action.Dodging"))
		{
			ClearStateFlag(ECharacterStateFlags::Dodging);
		}
		else if (TagName == FName("Action.AirDashing"))
		{
			ClearStateFlag(ECharacterStateFlags::AirDashing);
		}
		else if (TagName == FName("Action.Airborne"))
		{
			ClearStateFlag(ECharacterStateFlags::Airborne);
		}
		else if (TagName == FName("Action.Invulnerable"))
		{
			ClearStateFlag(ECharacterStateFlags::Invulnerable);
		}
		else if (TagName == FName("Action.Stunned"))
		{
			ClearStateFlag(ECharacterStateFlags::Stunned);
		}
		else if (TagName == FName("Action.Transformed"))
		{
			ClearStateFlag(ECharacterStateFlags::Transformed);
		}
	}
}

void UActionComponent::ClearCurrentAction()
{
	// Programmer: Removes all effects of the current action:
	// 1. Removes gameplay tags that were granted by the action
	// 2. Clears the corresponding ECharacterStateFlags bits
	// 3. Nulls out the CurrentAction reference
	// Always safe to call even if CurrentAction is null (guard at top).

	if (!CurrentAction)
	{
		return;
	}

	// Remove granted tags and clear corresponding state flags
	if (CurrentAction->ActionData)
	{
		const FGameplayTagContainer& GrantedTags = CurrentAction->ActionData->GrantedTags;

		for (const FGameplayTag& Tag : GrantedTags)
		{
			RemoveTag(Tag);
		}

		// Clear ECharacterStateFlags bits for the removed tags
		ClearStateFlagsForTags(GrantedTags);
	}

	// Null the action reference — GC will clean it up
	CurrentAction = nullptr;
}
