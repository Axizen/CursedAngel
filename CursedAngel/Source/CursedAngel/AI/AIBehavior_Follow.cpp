// Copyright Epic Games, Inc. All Rights Reserved.

#include "AIBehavior_Follow.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UAIBehavior_Follow::UAIBehavior_Follow()
{
	BehaviorName = TEXT("Follow");
	Priority = 50; // Medium priority - higher than idle, lower than combat
	FollowDistance = 300.0f;
	MaxFollowDistance = 1000.0f;
	bTeleportIfTooFar = true;
	TargetActor = nullptr;
}

bool UAIBehavior_Follow::CanActivate()
{
	// Check if player exists
	if (!TargetActor)
	{
		TargetActor = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	}

	if (!TargetActor)
	{
		return false;
	}

	// Check if we're far enough away to need following
	float Distance = GetDistanceToTarget();
	return Distance > FollowDistance;
}

void UAIBehavior_Follow::OnActivate()
{
	Super::OnActivate();

	// Cache player reference
	if (!TargetActor)
	{
		TargetActor = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	}

	if (!TargetActor)
	{
		return;
	}

	// Start moving to player
	MoveToActor(TargetActor);
}

void UAIBehavior_Follow::OnTick(float DeltaTime)
{
	Super::OnTick(DeltaTime);

	if (!TargetActor || !OwnerActor)
	{
		return;
	}

	float Distance = GetDistanceToTarget();

	// Check if we need to teleport
	if (bTeleportIfTooFar && Distance > MaxFollowDistance)
	{
		// Teleport to player
		FVector TargetLocation = TargetActor->GetActorLocation();
		FVector Offset = TargetActor->GetActorForwardVector() * -FollowDistance;
		FVector TeleportLocation = TargetLocation + Offset;

		OwnerActor->SetActorLocation(TeleportLocation);

		// Stop movement after teleport
		if (AAIController* AIController = Cast<AAIController>(OwnerActor->GetInstigatorController()))
		{
			AIController->StopMovement();
		}
	}
	// Check if we're close enough to stop following
	else if (Distance <= FollowDistance)
	{
		// Stop movement - we're close enough
		if (AAIController* AIController = Cast<AAIController>(OwnerActor->GetInstigatorController()))
		{
			AIController->StopMovement();
		}
	}
	else
	{
		// Continue moving to player
		MoveToActor(TargetActor);
	}
}

void UAIBehavior_Follow::OnDeactivate()
{
	Super::OnDeactivate();

	// Stop movement
	if (OwnerActor)
	{
		if (AAIController* AIController = Cast<AAIController>(OwnerActor->GetInstigatorController()))
		{
			AIController->StopMovement();
		}
	}
}

float UAIBehavior_Follow::GetDistanceToTarget() const
{
	if (!TargetActor || !OwnerActor)
	{
		return 0.0f;
	}

	return FVector::Distance(OwnerActor->GetActorLocation(), TargetActor->GetActorLocation());
}
