// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/EnemyAIController.h"
#include "AI/EnemyBase.h"
#include "Characters/CursedAngelCharacter.h"
#include "Components/HealthComponent.h"
#include "Components/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

AEnemyAIController::AEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;
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
	CurrentAdaptiveMoveTarget = FVector::ZeroVector;
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ControlledEnemy = Cast<AEnemyBase>(InPawn);
	if (!ControlledEnemy) return;
	PatrolOrigin = ControlledEnemy->GetActorLocation();
	TargetPlayer = Cast<ACursedAngelCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	if (ControlledEnemy->bUseBehaviorTree && ControlledEnemy->BehaviorTreeAsset)
	{
		StartBehaviorTree(ControlledEnemy->BehaviorTreeAsset, ControlledEnemy->BlackboardAsset);
	}
}

void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateAI(DeltaTime);
}

EEnemyAdaptiveTactic AEnemyAIController::GetActiveAdaptiveTactic() const
{
	return ControlledEnemy ? ControlledEnemy->AdaptiveTactic : EEnemyAdaptiveTactic::Default;
}

void AEnemyAIController::UpdateAI(float DeltaTime)
{
	// Behavior Trees may read AdaptiveTactic and CurrentAttackPattern from the pawn/blackboard;
	// the built-in state machine must not compete with a running tree.
	if (IsUsingBehaviorTree()) return;
	if (!ControlledEnemy || !ControlledEnemy->HealthComponent || !ControlledEnemy->HealthComponent->IsAlive()) return;
	if (!TargetPlayer) TargetPlayer = Cast<ACursedAngelCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	if (!TargetPlayer) { if (bUsePatrol) Patrol(); return; }

	const float Distance = ControlledEnemy->GetDistanceToPlayer();
	if (Distance < 0.0f || Distance > ControlledEnemy->AggroRange)
	{
		if (bUsePatrol) Patrol();
		return;
	}
	SetFocus(TargetPlayer);

	const EEnemyAdaptiveTactic Tactic = GetActiveAdaptiveTactic();
	const bool bPlayerAirborne = TargetPlayer->GetCharacterMovement() && TargetPlayer->GetCharacterMovement()->IsFalling();
	if (Tactic == EEnemyAdaptiveTactic::PunishAirborne && bPlayerAirborne)
	{
		// Close the gap while the player is airborne; the configured anti-air pattern is
		// available on the enemy for Blueprint/Behavior Tree attack selection.
		MoveTowardsPlayer(FMath::Max(AcceptanceRadius, AttackDistance * 0.75f));
		if (Distance <= AttackDistance && ControlledEnemy->CanAttack()) AttackPlayer();
		return;
	}
	if (Tactic == EEnemyAdaptiveTactic::StrafePlayer || Tactic == EEnemyAdaptiveTactic::DisplaceFromDestruction)
	{
		if (Distance <= AttackDistance)
		{
			AttackPlayer();
		}
		else
		{
			MoveToAdaptivePosition(Tactic);
		}
		return;
	}

	if (Distance <= AttackDistance) AttackPlayer();
	else if (Tactic == EEnemyAdaptiveTactic::RushPlayer) MoveTowardsPlayer(FMath::Max(AcceptanceRadius, AttackDistance * 0.65f));
	else if (Tactic == EEnemyAdaptiveTactic::MaintainRange)
	{
		const float DesiredRange = FMath::Max(AttackDistance, FollowDistance * ControlledEnemy->AdaptiveEngagementDistanceMultiplier);
		MoveTowardsPlayer(DesiredRange);
	}
	else MoveToPlayer();
}

void AEnemyAIController::MoveTowardsPlayer(float AcceptanceDistance)
{
	if (TargetPlayer) MoveToActor(TargetPlayer, AcceptanceDistance, true, true, true, nullptr, true);
}

void AEnemyAIController::MoveToAdaptivePosition(EEnemyAdaptiveTactic Tactic)
{
	if (!ControlledEnemy || !TargetPlayer || !GetWorld()) return;
	const float Now = GetWorld()->GetTimeSeconds();
	if (CurrentAdaptiveMoveTarget.IsNearlyZero() || FVector::Dist(ControlledEnemy->GetActorLocation(), CurrentAdaptiveMoveTarget) <= AcceptanceRadius || Now - LastAdaptiveMoveTime >= FMath::Max(0.25f, AdaptiveManeuverInterval))
	{
		const FVector ToPlayer = (TargetPlayer->GetActorLocation() - ControlledEnemy->GetActorLocation()).GetSafeNormal2D();
		const FVector Right = FVector::CrossProduct(FVector::UpVector, ToPlayer).GetSafeNormal();
		const float Distance = AdaptiveManeuverDistance * (ControlledEnemy->AdaptiveEngagementDistanceMultiplier);
		if (Tactic == EEnemyAdaptiveTactic::DisplaceFromDestruction)
		{
			// Rotate around and bias outward to avoid standing in the player's recently
			// exploited close-range/destruction space; no destructible actor is required.
			CurrentAdaptiveMoveTarget = TargetPlayer->GetActorLocation() - ToPlayer * Distance + Right * (Distance * 0.5f * StrafeDirection);
		}
		else
		{
			CurrentAdaptiveMoveTarget = TargetPlayer->GetActorLocation() + Right * Distance * StrafeDirection;
		}
		StrafeDirection *= -1;
		LastAdaptiveMoveTime = Now;
	}
	MoveToLocation(CurrentAdaptiveMoveTarget, AcceptanceRadius, true, true, false, true, nullptr, true);
}

void AEnemyAIController::MoveToPlayer()
{
	if (!TargetPlayer || !ControlledEnemy) return;
	const float DesiredDistance = FMath::Max(AcceptanceRadius, FollowDistance * ControlledEnemy->AdaptiveEngagementDistanceMultiplier);
	MoveTowardsPlayer(DesiredDistance);
}

void AEnemyAIController::AttackPlayer()
{
	if (!ControlledEnemy || !TargetPlayer) return;
	StopMovement();
	FVector Direction = TargetPlayer->GetActorLocation() - ControlledEnemy->GetActorLocation();
	Direction.Z = 0.0f;
	if (!Direction.IsNearlyZero()) ControlledEnemy->SetActorRotation(Direction.Rotation());
	if (ControlledEnemy->CanAttack()) ControlledEnemy->PerformAttack();
}

void AEnemyAIController::Patrol()
{
	if (!ControlledEnemy || !GetWorld()) return;
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	bool bNeedNewPoint = CurrentPatrolPoint.IsZero();
	if (!bNeedNewPoint && FVector::Dist(ControlledEnemy->GetActorLocation(), CurrentPatrolPoint) <= AcceptanceRadius && CurrentTime - LastPatrolTime >= PatrolWaitTime) bNeedNewPoint = true;
	if (bNeedNewPoint)
	{
		CurrentPatrolPoint = GetRandomPatrolPoint();
		LastPatrolTime = CurrentTime;
		MoveToLocation(CurrentPatrolPoint, AcceptanceRadius, true, true, false, true, nullptr, true);
	}
}

FVector AEnemyAIController::GetRandomPatrolPoint() const
{
	FVector Direction(FMath::FRandRange(-1.0f, 1.0f), FMath::FRandRange(-1.0f, 1.0f), 0.0f);
	Direction = Direction.GetSafeNormal();
	return PatrolOrigin + Direction * FMath::FRandRange(PatrolRadius * 0.3f, PatrolRadius);
}

void AEnemyAIController::StartBehaviorTree(UBehaviorTree* BehaviorTree, UBlackboardData* BlackboardAsset)
{
	if (!BehaviorTree) { UE_LOG(LogTemp, Warning, TEXT("StartBehaviorTree: BehaviorTree is null")); return; }
	if (BlackboardAsset && BlackboardComponent) UseBlackboard(BlackboardAsset, BlackboardComponent);
	if (!RunBehaviorTree(BehaviorTree)) UE_LOG(LogTemp, Warning, TEXT("StartBehaviorTree: Failed to start %s"), *BehaviorTree->GetName());
}

void AEnemyAIController::StopBehaviorTree()
{
	if (UBrainComponent* Brain = GetBrainComponent())
	{
		if (UBehaviorTreeComponent* Tree = Cast<UBehaviorTreeComponent>(Brain)) Tree->StopTree();
	}
}

bool AEnemyAIController::IsUsingBehaviorTree() const
{
	const UBrainComponent* Brain = GetBrainComponent();
	return Brain && Brain->IsRunning();
}
