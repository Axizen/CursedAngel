// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/DialogueComponent.h"
#include "Engine/DataTable.h"
#include "TimerManager.h"

UDialogueComponent::UDialogueComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UDialogueComponent::BeginPlay()
{
	Super::BeginPlay();
	
	LastDialogueTime = -999.0f;
	bIsPlayingDialogue = false;
	CurrentDialogueTimeRemaining = 0.0f;
}

void UDialogueComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ProcessDialogueQueue(DeltaTime);
}

void UDialogueComponent::TriggerDialogue(EDialogueContext Context, bool bImmediate)
{
	if (!DialogueTable)
	{
		return;
	}

	// Get random dialogue line for context
	FDialogueLine Line = GetRandomDialogueForContext(Context);
	
	if (Line.Text.IsEmpty())
	{
		return;
	}

	// Check if we can trigger dialogue
	if (!bImmediate && !CanTriggerDialogue())
	{
		// Add to queue if not immediate
		if (DialogueQueue.Num() < Config.MaxQueueSize)
		{
			DialogueQueue.Add(Line);
		}
		return;
	}

	// Check line-specific cooldown
	if (IsDialogueOnCooldown(Line))
	{
		return;
	}

	// Play immediately
	PlayDialogueLine(Line);
}

FDialogueLine UDialogueComponent::GetRandomDialogueForContext(EDialogueContext Context)
{
	FDialogueLine EmptyLine;
	
	if (!DialogueTable)
	{
		return EmptyLine;
	}

	// Get all dialogue lines for this context
	TArray<FDialogueLine*> ContextLines = GetDialogueLinesForContext(Context);
	
	if (ContextLines.Num() == 0)
	{
		return EmptyLine;
	}

	// Filter out lines on cooldown
	TArray<FDialogueLine*> AvailableLines;
	for (FDialogueLine* Line : ContextLines)
	{
		if (Line && !IsDialogueOnCooldown(*Line))
		{
			AvailableLines.Add(Line);
		}
	}

	if (AvailableLines.Num() == 0)
	{
		// If all lines are on cooldown, just pick a random one anyway
		AvailableLines = ContextLines;
	}

	// Weight selection by priority
	int32 TotalWeight = 0;
	for (FDialogueLine* Line : AvailableLines)
	{
		if (Line)
		{
			TotalWeight += Line->Priority;
		}
	}

	if (TotalWeight <= 0)
	{
		TotalWeight = AvailableLines.Num();
	}

	// Random weighted selection
	int32 RandomWeight = FMath::RandRange(0, TotalWeight - 1);
	int32 CurrentWeight = 0;
	
	for (FDialogueLine* Line : AvailableLines)
	{
		if (Line)
		{
			CurrentWeight += Line->Priority;
			if (CurrentWeight > RandomWeight)
			{
				return *Line;
			}
		}
	}

	// Fallback to first available line
	if (AvailableLines.Num() > 0 && AvailableLines[0])
	{
		return *AvailableLines[0];
	}

	return EmptyLine;
}

void UDialogueComponent::UpdateContext(EDialogueContext NewContext)
{
	if (CurrentContext != NewContext)
	{
		CurrentContext = NewContext;
	}
}

bool UDialogueComponent::CanTriggerDialogue() const
{
	if (bIsPlayingDialogue)
	{
		return false;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();
	float TimeSinceLastDialogue = CurrentTime - LastDialogueTime;
	
	return TimeSinceLastDialogue >= Config.MinTimeBetweenLines;
}

void UDialogueComponent::ClearDialogueQueue()
{
	DialogueQueue.Empty();
}

void UDialogueComponent::ProcessDialogueQueue(float DeltaTime)
{
	// Update current dialogue timer
	if (bIsPlayingDialogue)
	{
		CurrentDialogueTimeRemaining -= DeltaTime;
		
		if (CurrentDialogueTimeRemaining <= 0.0f)
		{
			CompleteCurrentDialogue();
		}
	}

	// Process queue if not playing and can trigger
	if (!bIsPlayingDialogue && CanTriggerDialogue() && DialogueQueue.Num() > 0)
	{
		FDialogueLine NextLine = DialogueQueue[0];
		DialogueQueue.RemoveAt(0);
		
		PlayDialogueLine(NextLine);
	}
}

void UDialogueComponent::PlayDialogueLine(const FDialogueLine& Line)
{
	if (Line.Text.IsEmpty())
	{
		return;
	}

	bIsPlayingDialogue = true;
	CurrentDialogueTimeRemaining = Line.Duration;
	LastDialogueTime = GetWorld()->GetTimeSeconds();

	// Update the line's last triggered time in the data table
	if (DialogueTable)
	{
		TArray<FDialogueLine*> AllLines;
		DialogueTable->GetAllRows<FDialogueLine>(TEXT("GetAllDialogueLines"), AllLines);
		
		for (FDialogueLine* TableLine : AllLines)
		{
			if (TableLine && TableLine->Text.EqualTo(Line.Text) && TableLine->Context == Line.Context)
			{
				TableLine->LastTriggeredTime = LastDialogueTime;
				break;
			}
		}
	}

	// Fire Blueprint event
	OnDialogueTriggered(Line);
}

void UDialogueComponent::CompleteCurrentDialogue()
{
	bIsPlayingDialogue = false;
	CurrentDialogueTimeRemaining = 0.0f;
	
	OnDialogueComplete();
}

TArray<FDialogueLine*> UDialogueComponent::GetDialogueLinesForContext(EDialogueContext Context)
{
	TArray<FDialogueLine*> ContextLines;
	
	if (!DialogueTable)
	{
		return ContextLines;
	}

	TArray<FDialogueLine*> AllLines;
	DialogueTable->GetAllRows<FDialogueLine>(TEXT("GetDialogueLinesForContext"), AllLines);
	
	for (FDialogueLine* Line : AllLines)
	{
		if (Line && Line->Context == Context)
		{
			ContextLines.Add(Line);
		}
	}

	return ContextLines;
}

bool UDialogueComponent::IsDialogueOnCooldown(const FDialogueLine& Line) const
{
	if (Line.Cooldown <= 0.0f)
	{
		return false;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();
	float TimeSinceTriggered = CurrentTime - Line.LastTriggeredTime;
	
	return TimeSinceTriggered < Line.Cooldown;
}
