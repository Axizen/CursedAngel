// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/AIBehaviorBase.h"
#include "Characters/CursedAngelCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"

void UAIBehaviorBase::OnActivate()
{
	// Default implementation - override in derived classes
	bIsActive = true;
}

void UAIBehaviorBase::OnTick(float DeltaTime)
{
	// Default implementation - override in derived classes
}

void UAIBehaviorBase::OnDeactivate()
{
	// Default implementation - override in derived classes
	bIsActive = false;
}

bool UAIBehaviorBase::CanActivate()
{
	// Default implementation - override in derived classes
	return true;
}

bool UAIBehaviorBase::BP_CanActivate_Implementation()
{
	// Default implementation for Blueprint native event
	return CanActivate();
}

ACursedAngelCharacter* UAIBehaviorBase::GetOwnerCharacter()
{
	if (OwnerActor)
	{
		return Cast<ACursedAngelCharacter>(OwnerActor);
	}
	return nullptr;
}

ACursedAngelCharacter* UAIBehaviorBase::GetPlayerCharacter()
{
	if (OwnerActor)
	{
		UWorld* World = OwnerActor->GetWorld();
		if (World)
		{
			APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);
			if (PlayerController)
			{
				return Cast<ACursedAngelCharacter>(PlayerController->GetPawn());
			}
		}
	}
	return nullptr;
}

void UAIBehaviorBase::MoveToLocation(FVector Location)
{
	ACursedAngelCharacter* OwnerCharacter = GetOwnerCharacter();
	if (OwnerCharacter)
	{
		AAIController* AIController = Cast<AAIController>(OwnerCharacter->GetController());
		if (AIController)
		{
			AIController->MoveToLocation(Location, -1.0f, true, true, false, true);
		}
	}
}

void UAIBehaviorBase::MoveToActor(AActor* Target)
{
	if (!Target)
	{
		return;
	}

	ACursedAngelCharacter* OwnerCharacter = GetOwnerCharacter();
	if (OwnerCharacter)
	{
		AAIController* AIController = Cast<AAIController>(OwnerCharacter->GetController());
		if (AIController)
		{
			AIController->MoveToActor(Target, -1.0f, true, true, false, nullptr, true);
		}
	}
}
