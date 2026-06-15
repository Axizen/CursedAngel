// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/CurseConfirmDialogWidget.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"

void UCurseConfirmDialogWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BTN_Yes)
	{
		BTN_Yes->OnClicked.AddDynamic(this, &UCurseConfirmDialogWidget::OnYesClicked);
	}

	if (BTN_No)
	{
		BTN_No->OnClicked.AddDynamic(this, &UCurseConfirmDialogWidget::OnNoClicked);
	}
}

void UCurseConfirmDialogWidget::SetMessage(const FText& Message)
{
	if (TXT_Message)
	{
		TXT_Message->SetText(Message);
	}
}

void UCurseConfirmDialogWidget::ShowDialog()
{
	SetVisibility(ESlateVisibility::Visible);
}

void UCurseConfirmDialogWidget::HideDialog()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UCurseConfirmDialogWidget::OnYesClicked()
{
	OnConfirmed.Broadcast();
	HideDialog();
}

void UCurseConfirmDialogWidget::OnNoClicked()
{
	OnCancelled.Broadcast();
	HideDialog();
}
