// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/StyleRankDataAsset.h"

FStyleRankData UStyleRankDataAsset::GetRankData(EStyleRank Rank) const
{
	if (const FStyleRankData* Data = RankConfigs.Find(Rank))
	{
		return *Data;
	}

	// Return default data if not found
	return FStyleRankData();
}

bool UStyleRankDataAsset::HasRankConfig(EStyleRank Rank) const
{
	return RankConfigs.Contains(Rank);
}

EStyleRank UStyleRankDataAsset::GetRankForPoints(float Points) const
{
	// Start from highest rank and work down
	EStyleRank CurrentRank = EStyleRank::D;

	// Check each rank in descending order
	if (const FStyleRankData* SRank = RankConfigs.Find(EStyleRank::S))
	{
		if (Points >= SRank->PointsRequired)
		{
			return EStyleRank::S;
		}
	}

	if (const FStyleRankData* ARank = RankConfigs.Find(EStyleRank::A))
	{
		if (Points >= ARank->PointsRequired)
		{
			return EStyleRank::A;
		}
	}

	if (const FStyleRankData* BRank = RankConfigs.Find(EStyleRank::B))
	{
		if (Points >= BRank->PointsRequired)
		{
			return EStyleRank::B;
		}
	}

	if (const FStyleRankData* CRank = RankConfigs.Find(EStyleRank::C))
	{
		if (Points >= CRank->PointsRequired)
		{
			return EStyleRank::C;
		}
	}

	// Default to D rank
	return EStyleRank::D;
}
