// Copyright Cursed Angel. All Rights Reserved.

#include "AIBehaviorComponent.h"
#include "CursedAngel/AI/AIBehaviorBase.h"

UAIBehaviorComponent::UAIBehaviorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAIBehaviorComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize behaviors with owner reference
	InitializeBehaviors();
}

void UAIBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Tick the current behavior if active
	if (CurrentBehavior && CurrentBehavior->bIsActive)
	{
		CurrentBehavior->OnTick(DeltaTime);
		CurrentBehavior->BP_OnTick(DeltaTime);
	}

	// Auto-select best behavior if enabled and no current behavior
	if (bAutoSelectBehavior && !CurrentBehavior)
	{
		UAIBehaviorBase* BestBehavior = SelectBestBehavior();
		if (BestBehavior)
		{
			ExecuteBehavior(BestBehavior->BehaviorName);
		}
	}
}

bool UAIBehaviorComponent::ExecuteBehavior(FName BehaviorName)
{
	// Find the behavior by name
	UAIBehaviorBase* Behavior = GetBehavior(BehaviorName);
	if (!Behavior)
	{
		UE_LOG(LogTemp, Warning, TEXT("UAIBehaviorComponent::ExecuteBehavior: Behavior '%s' not found"), *BehaviorName.ToString());
		return false;
	}

	// Check if the behavior can be activated
	if (!Behavior->CanActivate() || !Behavior->BP_CanActivate())
	{
		return false;
	}

	// Stop current behavior if any
	if (CurrentBehavior)
	{
		StopCurrentBehavior();
	}

	// Set as current behavior
	CurrentBehavior = Behavior;
	CurrentBehavior->bIsActive = true;

	// Activate the behavior
	CurrentBehavior->OnActivate();
	CurrentBehavior->BP_OnActivate();

	return true;
}

void UAIBehaviorComponent::StopCurrentBehavior()
{
	if (!CurrentBehavior)
	{
		return;
	}

	// Deactivate the behavior
	CurrentBehavior->OnDeactivate();
	CurrentBehavior->BP_OnDeactivate();
	CurrentBehavior->bIsActive = false;

	// Clear current behavior
	CurrentBehavior = nullptr;
}

UAIBehaviorBase* UAIBehaviorComponent::GetBehavior(FName BehaviorName) const
{
	for (UAIBehaviorBase* Behavior : AvailableBehaviors)
	{
		if (Behavior && Behavior->BehaviorName == BehaviorName)
		{
			return Behavior;
		}
	}

	return nullptr;
}

UAIBehaviorBase* UAIBehaviorComponent::SelectBestBehavior()
{
	UAIBehaviorBase* BestBehavior = nullptr;
	int32 HighestPriority = TNumericLimits<int32>::Min();

	// Iterate through all behaviors to find highest priority valid behavior
	for (UAIBehaviorBase* Behavior : AvailableBehaviors)
	{
		if (!Behavior)
		{
			continue;
		}

		// Check if this behavior can activate
		if (!Behavior->CanActivate() || !Behavior->BP_CanActivate())
		{
			continue;
		}

		// Check if this behavior has higher priority than current best
		if (Behavior->Priority > HighestPriority)
		{
			BestBehavior = Behavior;
			HighestPriority = Behavior->Priority;
		}
	}

	return BestBehavior;
}

void UAIBehaviorComponent::InitializeBehaviors()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Set owner reference for all behaviors
	for (UAIBehaviorBase* Behavior : AvailableBehaviors)
	{
		if (Behavior)
		{
			Behavior->OwnerActor = Owner;
		}
	}
}
