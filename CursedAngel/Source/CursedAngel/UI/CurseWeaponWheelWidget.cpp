// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/CurseWeaponWheelWidget.h"

#include "Components/Overlay.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

#include "Components/CurseWeaponComponent.h"
#include "Engine/Texture2D.h"

// ============================================================================
// Visibility
// ============================================================================

void UCurseWeaponWheelWidget::ShowWheel()
{
	SetVisibility(ESlateVisibility::Visible);
	bIsWheelVisible = true;

	// Sync the visual highlight to the currently equipped weapon.
	UpdateSelection(CurrentWeaponIndex);
}

void UCurseWeaponWheelWidget::HideWheel()
{
	SetVisibility(ESlateVisibility::Collapsed);
	bIsWheelVisible = false;
}

// ============================================================================
// Selection
// ============================================================================

void UCurseWeaponWheelWidget::UpdateSelection(int32 SlotIndex)
{
	SelectedSlotIndex = SlotIndex;

	TObjectPtr<UBorder> Borders[3] = { Border_0, Border_1, Border_2 };

	for (int32 i = 0; i < 3; ++i)
	{
		if (Borders[i])
		{
			Borders[i]->SetBrushColor(i == SlotIndex ? HighlightColor : NormalColor);
		}
	}
}

void UCurseWeaponWheelWidget::SelectWeapon(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex > 2)
	{
		return;
	}

	SelectedSlotIndex  = SlotIndex;
	CurrentWeaponIndex = SlotIndex;

	// Tell the component to switch weapons.
	if (CachedWeaponComp)
	{
		const ECurseWeaponType NewWeapon = static_cast<ECurseWeaponType>(SlotIndex);
		CachedWeaponComp->SwitchWeapon(NewWeapon);
		OnWeaponSelected(NewWeapon);
	}

	HideWheel();
}

// ============================================================================
// Data Initialisation
// ============================================================================

void UCurseWeaponWheelWidget::InitializeWeaponData(UCurseWeaponComponent* WeaponComp,
                                                    const TArray<FText>& Names,
                                                    const TArray<UTexture2D*>& Icons)
{
	CachedWeaponComp  = WeaponComp;
	CachedWeaponNames = Names;

	CachedWeaponIcons.Reset();
	for (UTexture2D* Tex : Icons)
	{
		CachedWeaponIcons.Add(Tex);
	}

	// Sync current weapon index from the component if available.
	if (WeaponComp)
	{
		CurrentWeaponIndex = static_cast<int32>(WeaponComp->CurrentWeapon);
	}

	UpdateWeaponIcons();
}

void UCurseWeaponWheelWidget::UpdateWeaponIcons()
{
	TObjectPtr<UImage>     Icons[3] = { WeaponIcon_0, WeaponIcon_1, WeaponIcon_2 };
	TObjectPtr<UTextBlock> Names[3] = { WeaponName_0, WeaponName_1, WeaponName_2 };

	for (int32 i = 0; i < 3; ++i)
	{
		if (Names[i] && CachedWeaponNames.IsValidIndex(i))
		{
			Names[i]->SetText(CachedWeaponNames[i]);
		}

		if (Icons[i] && CachedWeaponIcons.IsValidIndex(i) && CachedWeaponIcons[i])
		{
			Icons[i]->SetBrushFromTexture(CachedWeaponIcons[i]);
		}
	}

	// Restore border highlight to the current weapon.
	UpdateSelection(CurrentWeaponIndex);
}

// ============================================================================
// BlueprintNativeEvent
// ============================================================================

void UCurseWeaponWheelWidget::OnWeaponSelected_Implementation(ECurseWeaponType SelectedWeapon)
{
	// Default C++ implementation is intentionally empty.
	// Override in Blueprint for custom transition effects (e.g. slot flash, weapon name pop-in).
}
