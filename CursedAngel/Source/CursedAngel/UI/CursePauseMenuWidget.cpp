// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/CursePauseMenuWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UCursePauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind button click events
	if (BTN_Resume)
	{
		BTN_Resume->OnClicked.AddDynamic(this, &UCursePauseMenuWidget::OnResumeClicked);
	}

	if (BTN_Settings)
	{
		BTN_Settings->OnClicked.AddDynamic(this, &UCursePauseMenuWidget::OnSettingsClicked);
	}

	if (BTN_Restart)
	{
		BTN_Restart->OnClicked.AddDynamic(this, &UCursePauseMenuWidget::OnRestartClicked);
	}

	if (BTN_MainMenu)
	{
		BTN_MainMenu->OnClicked.AddDynamic(this, &UCursePauseMenuWidget::OnMainMenuClicked);
	}

	if (BTN_Quit)
	{
		BTN_Quit->OnClicked.AddDynamic(this, &UCursePauseMenuWidget::OnQuitClicked);
	}

	// Pause the game when this widget is constructed
	UGameplayStatics::SetGamePaused(this, true);

	// Notify Blueprint that the menu is open
	OnPauseMenuOpened();
}

void UCursePauseMenuWidget::OnPauseMenuOpened_Implementation()
{
	// Default: no-op. Override in Blueprint to play open animation.
}

void UCursePauseMenuWidget::OnResumeClicked()
{
	UGameplayStatics::SetGamePaused(this, false);
	RemoveFromParent();
}

void UCursePauseMenuWidget::OnSettingsClicked()
{
	// Notify any listeners (e.g., Game Mode) to show a settings panel
	OnSettingsRequested.Broadcast();
}

void UCursePauseMenuWidget::OnRestartClicked()
{
	UGameplayStatics::SetGamePaused(this, false);

	// Reload the current level by name
	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::OpenLevel(this, FName(*World->GetMapName()));
	}
}

void UCursePauseMenuWidget::OnMainMenuClicked()
{
	UGameplayStatics::SetGamePaused(this, false);
	UGameplayStatics::OpenLevel(this, MainMenuLevelName);
}

void UCursePauseMenuWidget::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
