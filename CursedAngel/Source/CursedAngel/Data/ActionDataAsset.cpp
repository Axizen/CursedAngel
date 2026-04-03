// Copyright Cursed Angel. All Rights Reserved.

#include "ActionDataAsset.h"

FPrimaryAssetId UActionDataAsset::GetPrimaryAssetId() const
{
	// Return asset type "Action" with this asset's name as the identifier
	return FPrimaryAssetId(TEXT("Action"), GetFName());
}
