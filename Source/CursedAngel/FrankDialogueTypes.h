// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FrankDialogueTypes.generated.h"

/**
 * Enum representing different dialogue contexts for Frank's barks
 */
UENUM(BlueprintType)
enum class EDialogueContext : uint8
{
	Combat UMETA(DisplayName = "Combat"),
	Puzzle UMETA(DisplayName = "Puzzle"),
	Exploration UMETA(DisplayName = "Exploration"),
	Transformation UMETA(DisplayName = "Transformation"),
	Victory UMETA(DisplayName = "Victory"),
	Idle UMETA(DisplayName = "Idle"),
	Hint UMETA(DisplayName = "Hint")
};

/**
 * Struct representing a single dialogue line
 * Used as row structure for dialogue data table
 */
USTRUCT(BlueprintType)
struct FDialogueLine : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	EDialogueContext Context = EDialogueContext::Exploration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	float Duration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	int32 Priority = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	float Cooldown = 10.0f;

	// Runtime tracking
	float LastTriggeredTime = -999.0f;

	FDialogueLine()
		: Context(EDialogueContext::Exploration)
		, Duration(3.0f)
		, Priority(1)
		, Cooldown(10.0f)
		, LastTriggeredTime(-999.0f)
	{
	}
};

/**
 * Configuration struct for dialogue system behavior
 */
USTRUCT(BlueprintType)
struct FDialogueConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	float MinTimeBetweenLines = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	int32 MaxQueueSize = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	bool bEnableHints = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	float HintFrequency = 30.0f;

	FDialogueConfig()
		: MinTimeBetweenLines(5.0f)
		, MaxQueueSize(3)
		, bEnableHints(true)
		, HintFrequency(30.0f)
	{
	}
};
