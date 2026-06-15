// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/CurseVictoryWidget.h"

#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "Animation/WidgetAnimation.h"
#include "Math/Color.h"

void UCurseVictoryWidget::ShowVictoryScreen(
	int32 EnemiesKilled,
	float TimeTaken,
	int32 DataFragments,
	int32 CurseEssence,
	EStyleRank HighestRank)
{
	// Store all result data so helper functions can reference it
	StoredEnemiesKilled   = EnemiesKilled;
	StoredTimeTaken       = TimeTaken;
	StoredDataFragments   = DataFragments;
	StoredCurseEssence    = CurseEssence;
	StoredHighestRank     = HighestRank;

	// --- Populate text blocks ---

	if (RankValueText)
	{
		RankValueText->SetText(GetRankString(HighestRank));
		RankValueText->SetColorAndOpacity(FSlateColor(GetRankColor(HighestRank)));
	}

	if (EnemiesValueText)
	{
		EnemiesValueText->SetText(FText::AsNumber(EnemiesKilled));
	}

	if (DataFragmentsValueText)
	{
		DataFragmentsValueText->SetText(FText::AsNumber(DataFragments));
	}

	if (CurseEssenceValueText)
	{
		CurseEssenceValueText->SetText(FText::AsNumber(CurseEssence));
	}

	if (TimeValueText)
	{
		TimeValueText->SetText(FormatTime(TimeTaken));
	}

	if (ScoreValueText)
	{
		ScoreValueText->SetText(FText::AsNumber(CalculateFinalScore()));
	}

	// --- Play sequenced animations (null-checked; designers may omit some) ---

	if (FadeInOverlay)
	{
		PlayAnimation(FadeInOverlay);
	}

	if (ScaleInTitle)
	{
		PlayAnimation(ScaleInTitle);
	}

	if (SlideInResults)
	{
		PlayAnimation(SlideInResults);
	}

	if (BlinkContinue)
	{
		PlayAnimation(BlinkContinue);
	}
}

int32 UCurseVictoryWidget::CalculateFinalScore() const
{
	return FMath::RoundToInt(static_cast<float>(StoredEnemiesKilled) * ScoreMultiplierPerEnemy)
		+ StoredDataFragments
		+ StoredCurseEssence;
}

FText UCurseVictoryWidget::FormatTime(float Seconds) const
{
	const int32 TotalSeconds = FMath::FloorToInt(Seconds);
	const int32 Minutes      = TotalSeconds / 60;
	const int32 Secs         = TotalSeconds % 60;

	return FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Secs));
}

FText UCurseVictoryWidget::GetRankString(EStyleRank Rank) const
{
	switch (Rank)
	{
	case EStyleRank::D:   return FText::FromString(TEXT("D"));
	case EStyleRank::C:   return FText::FromString(TEXT("C"));
	case EStyleRank::B:   return FText::FromString(TEXT("B"));
	case EStyleRank::A:   return FText::FromString(TEXT("A"));
	case EStyleRank::S:   return FText::FromString(TEXT("S"));
	case EStyleRank::SS:  return FText::FromString(TEXT("SS"));
	case EStyleRank::SSS: return FText::FromString(TEXT("SSS"));
	default:              return FText::FromString(TEXT("?"));
	}
}

FLinearColor UCurseVictoryWidget::GetRankColor(EStyleRank Rank) const
{
	if (const FLinearColor* FoundColor = RankColors.Find(Rank))
	{
		return *FoundColor;
	}

	// Fallback: white if no color is configured for this rank
	return FLinearColor::White;
}
