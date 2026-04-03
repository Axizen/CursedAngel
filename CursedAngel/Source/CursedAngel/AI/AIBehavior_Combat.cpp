// Copyright Epic Games, Inc. All Rights Reserved.

#include "AIBehavior_Combat.h"
#include "Components/ActionComponent.h"
#include "Characters/CursedAngelCharacter.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"

UAIBehavior_Combat::UAIBehavior_Combat()
{
	BehaviorName = FName("Combat");
	Priority = 10; // Higher priority than follow/idle
}

bool UAIBehavior_Combat::CanActivate()
{
	// Check if there are enemies nearby
	AActor* NearestEnemy = FindNearestEnemy();
	return NearestEnemy != nullptr;
}

void UAIBehavior_Combat::OnActivate()
{
	// Find the nearest enemy target
	CurrentTarget = FindNearestEnemy();
	
	if (CurrentTarget)
	{
		// Move to combat range
		MoveToActor(CurrentTarget);
	}
	
	// Reset cooldown on activation
	CurrentCooldown = 0.0f;
	bIsActive = true;
}

void UAIBehavior_Combat::OnTick(float DeltaTime)
{
	if (!bIsActive)
	{
		return;
	}

	// Update cooldown
	if (CurrentCooldown > 0.0f)
	{
		CurrentCooldown -= DeltaTime;
	}

	// Check if current target is still valid
	if (!CurrentTarget || !IsValid(CurrentTarget))
	{
		// Try to find a new target
		CurrentTarget = FindNearestEnemy();
		
		if (!CurrentTarget)
		{
			// No more enemies, deactivate
			OnDeactivate();
			return;
		}
	}

	// Update movement to target
	if (CurrentTarget)
	{
		MoveToActor(CurrentTarget);
		
		// Check if we're in range and cooldown is ready
		float DistanceToTarget = FVector::Dist(OwnerActor->GetActorLocation(), CurrentTarget->GetActorLocation());
		
		if (DistanceToTarget <= CombatRange && CurrentCooldown <= 0.0f)
		{
			// Execute attack action
			ACursedAngelCharacter* OwnerCharacter = GetOwnerCharacter();
			if (OwnerCharacter)
			{
				UActionComponent* ActionComp = OwnerCharacter->FindComponentByClass<UActionComponent>();
				if (ActionComp)
				{
					// Execute the attack action
					if (ActionComp->ExecuteAction(AttackActionName))
					{
						// Reset cooldown
						CurrentCooldown = AttackCooldown;
					}
				}
			}
		}
	}
}

void UAIBehavior_Combat::OnDeactivate()
{
	CurrentTarget = nullptr;
	CurrentCooldown = 0.0f;
	bIsActive = false;
}

AActor* UAIBehavior_Combat::FindNearestEnemy()
{
	if (!OwnerActor)
	{
		return nullptr;
	}

	UWorld* World = OwnerActor->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// Perform sphere overlap to find enemies within combat range
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerActor);
	
	FVector OwnerLocation = OwnerActor->GetActorLocation();
	
	bool bHasOverlap = World->OverlapMultiByChannel(
		OverlapResults,
		OwnerLocation,
		FQuat::Identity,
		ECollisionChannel::ECC_Pawn,
		FCollisionShape::MakeSphere(CombatRange),
		QueryParams
	);

	if (!bHasOverlap)
	{
		return nullptr;
	}

	// Find the nearest enemy
	AActor* NearestEnemy = nullptr;
	float NearestDistance = FLT_MAX;

	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* OverlappedActor = Result.GetActor();
		if (!OverlappedActor || !IsValid(OverlappedActor))
		{
			continue;
		}

		// Check if actor has "Enemy" tag
		if (OverlappedActor->ActorHasTag(FName("Enemy")))
		{
			float Distance = FVector::Dist(OwnerLocation, OverlappedActor->GetActorLocation());
			if (Distance < NearestDistance)
			{
				NearestDistance = Distance;
				NearestEnemy = OverlappedActor;
			}
		}
	}

	return NearestEnemy;
}
