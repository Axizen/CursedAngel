// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/EnemyAIController.h"
#include "AI/EnemyBase.h"
#include "Characters/CursedAngelCharacter.h"
#include "Components/HealthComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

AEnemyAIController::AEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	// Default values
	FollowDistance = 300.0f;
	AttackDistance = 200.0f;
	PatrolRadius = 500.0f;
	bUsePatrol = true;
	PatrolWaitTime = 3.0f;
	AcceptanceRadius = 50.0f;

	ControlledEnemy = nullptr;
	TargetPlayer = nullptr;
	LastPatrolTime = 0.0f;
	PatrolOrigin = FVector::ZeroVector;
	CurrentPatrolPoint = FVector::ZeroVector;

	// Create Blackboard component
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Cache controlled enemy reference
	ControlledEnemy = Cast<AEnemyBase>(InPawn);
	if (ControlledEnemy)
	{
		// Store patrol origin
		PatrolOrigin = ControlledEnemy->GetActorLocation();
		
		// Find player target
		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		if (PlayerPawn)
		{
			TargetPlayer = Cast<ACursedAngelCharacter>(PlayerPawn);
		}

		// Check if enemy should use Behavior Tree
		if (ControlledEnemy->bUseBehaviorTree && ControlledEnemy->BehaviorTreeAsset)
		{
			StartBehaviorTree(ControlledEnemy->BehaviorTreeAsset, ControlledEnemy->BlackboardAsset);
		}
	}
}

void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateAI(DeltaTime);
}

void AEnemyAIController::UpdateAI(float DeltaTime)
{
	// Skip if using Behavior Tree - BT handles AI logic
	if (IsUsingBehaviorTree())
	{
		return;
	}

	// Early exit if no controlled enemy or enemy is dead
	if (!ControlledEnemy || !ControlledEnemy->HealthComponent || !ControlledEnemy->HealthComponent->IsAlive())
	{
		return;
	}

	// Try to find player if we don't have a target
	if (!TargetPlayer)
	{
		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		if (PlayerPawn)
		{
			TargetPlayer = Cast<ACursedAngelCharacter>(PlayerPawn);
		}
		
		// If still no player, just patrol
		if (!TargetPlayer)
		{
			if (bUsePatrol)
			{
				Patrol();
			}
			return;
		}
	}

	// Get distance to player
	float DistanceToPlayer = ControlledEnemy->GetDistanceToPlayer();
	
	// If player is out of range, patrol or idle
	if (DistanceToPlayer < 0.0f || DistanceToPlayer > ControlledEnemy->AggroRange)
	{
		if (bUsePatrol)
		{
			Patrol();
		}
		return;
	}

	// Player is in aggro range - set as focus
	SetFocus(TargetPlayer);

	// Check if in attack range
	if (DistanceToPlayer <= AttackDistance)
	{
		AttackPlayer();
	}
	// Check if in follow range
	else if (DistanceToPlayer <= ControlledEnemy->AggroRange)
	{
		MoveToPlayer();
	}
}

void AEnemyAIController::MoveToPlayer()
{
	if (!TargetPlayer || !ControlledEnemy)
	{
		return;
	}

	// Move towards player
	MoveToActor(TargetPlayer, FollowDistance, true, true, true, nullptr, true);
}

void AEnemyAIController::AttackPlayer()
{
	if (!ControlledEnemy || !TargetPlayer)
	{
		return;
	}

	// Stop movement
	StopMovement();

	// Face the player
	FVector DirectionToPlayer = TargetPlayer->GetActorLocation() - ControlledEnemy->GetActorLocation();
	DirectionToPlayer.Z = 0.0f;
	DirectionToPlayer.Normalize();
	
	FRotator TargetRotation = DirectionToPlayer.Rotation();
	ControlledEnemy->SetActorRotation(TargetRotation);

	// Perform attack if cooldown is ready
	if (ControlledEnemy->CanAttack())
	{
		ControlledEnemy->PerformAttack();
	}
}

void AEnemyAIController::Patrol()
{
	if (!ControlledEnemy)
	{
		return;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();

	// Check if we need a new patrol point
	bool bNeedNewPoint = false;
	
	// If we don't have a patrol point yet
	if (CurrentPatrolPoint.IsZero())
	{
		bNeedNewPoint = true;
	}
	// If we've reached the current patrol point
	else if (FVector::Dist(ControlledEnemy->GetActorLocation(), CurrentPatrolPoint) <= AcceptanceRadius)
	{
		// Wait at patrol point
		if (CurrentTime - LastPatrolTime >= PatrolWaitTime)
		{
			bNeedNewPoint = true;
		}
	}

	// Generate new patrol point if needed
	if (bNeedNewPoint)
	{
		CurrentPatrolPoint = GetRandomPatrolPoint();
		LastPatrolTime = CurrentTime;
		
		// Move to new patrol point
		MoveToLocation(CurrentPatrolPoint, AcceptanceRadius, true, true, false, true, nullptr, true);
	}
}

FVector AEnemyAIController::GetRandomPatrolPoint() const
{
	// Generate random point within patrol radius
	FVector RandomDirection = FVector(
		FMath::FRandRange(-1.0f, 1.0f),
		FMath::FRandRange(-1.0f, 1.0f),
		0.0f
	);
	RandomDirection.Normalize();

	float RandomDistance = FMath::FRandRange(PatrolRadius * 0.3f, PatrolRadius);
	FVector PatrolPoint = PatrolOrigin + (RandomDirection * RandomDistance);

	return PatrolPoint;
}

void AEnemyAIController::StartBehaviorTree(UBehaviorTree* BehaviorTree, UBlackboardData* BlackboardAsset)
{
	if (!BehaviorTree)
	{
		UE_LOG(LogTemp, Warning, TEXT("AEnemyAIController::StartBehaviorTree - BehaviorTree is null"));
		return;
	}

	// Initialize Blackboard if provided
	if (BlackboardAsset && BlackboardComponent)
	{
		UseBlackboard(BlackboardAsset, BlackboardComponent);
	}

	// Run Behavior Tree
	bool bSuccess = RunBehaviorTree(BehaviorTree);
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("AEnemyAIController::StartBehaviorTree - Successfully started BehaviorTree: %s"), *BehaviorTree->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AEnemyAIController::StartBehaviorTree - Failed to start BehaviorTree: %s"), *BehaviorTree->GetName());
	}
}

void AEnemyAIController::StopBehaviorTree()
{
	UBrainComponent* BrainComp = GetBrainComponent();
	if (BrainComp)
	{
		UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(BrainComp);
		if (BTComp)
		{
			BTComp->StopTree();
			UE_LOG(LogTemp, Log, TEXT("AEnemyAIController::StopBehaviorTree - Behavior Tree stopped"));
		}
	}
}

bool AEnemyAIController::IsUsingBehaviorTree() const
{
	UBrainComponent* BrainComp = GetBrainComponent();
	if (BrainComp)
	{
		return BrainComp->IsRunning();
	}
	return false;
}
