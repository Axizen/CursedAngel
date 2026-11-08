// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/EnemyAIController.h"
#include "AI/EnemyBase.h"
#include "Characters/CursedAngelCharacter.h"
#include "Components/HealthComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

AEnemyAIController::AEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;
	FollowDistance = 300.0f; AttackDistance = 200.0f; PatrolRadius = 500.0f; bUsePatrol = true; PatrolWaitTime = 3.0f; AcceptanceRadius = 50.0f;
	ControlledEnemy = nullptr; TargetPlayer = nullptr; LastPatrolTime = 0.0f; PatrolOrigin = FVector::ZeroVector; CurrentPatrolPoint = FVector::ZeroVector; CurrentAdaptiveMoveTarget = FVector::ZeroVector;
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
}
void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn); ControlledEnemy = Cast<AEnemyBase>(InPawn); if (!ControlledEnemy) return;
	PatrolOrigin = ControlledEnemy->GetActorLocation(); TargetPlayer = Cast<ACursedAngelCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	if (ControlledEnemy->bUseBehaviorTree && ControlledEnemy->BehaviorTreeAsset) StartBehaviorTree(ControlledEnemy->BehaviorTreeAsset, ControlledEnemy->BlackboardAsset);
}
void AEnemyAIController::Tick(float DeltaTime) { Super::Tick(DeltaTime); UpdateAI(DeltaTime); }
EEnemyAdaptiveTactic AEnemyAIController::GetActiveAdaptiveTactic() const { return ControlledEnemy ? ControlledEnemy->AdaptiveTactic : EEnemyAdaptiveTactic::Default; }

void AEnemyAIController::UpdateAI(float DeltaTime)
{
	if (IsUsingBehaviorTree() || !ControlledEnemy || !ControlledEnemy->HealthComponent || !ControlledEnemy->HealthComponent->IsAlive()) return;
	if (!TargetPlayer) TargetPlayer = Cast<ACursedAngelCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	if (!TargetPlayer) { if (bUsePatrol) Patrol(); return; }
	const float Distance = ControlledEnemy->GetDistanceToPlayer();
	if (Distance < 0.0f || Distance > ControlledEnemy->AggroRange) { if (bUsePatrol) Patrol(); return; }
	SetFocus(TargetPlayer);
	if (ControlledEnemy->bHasCoordinatedMoveTarget)
	{
		if (FVector::Dist2D(ControlledEnemy->GetActorLocation(), ControlledEnemy->CoordinatedMoveTarget) > AcceptanceRadius * 1.5f)
			MoveToLocation(ControlledEnemy->CoordinatedMoveTarget, AcceptanceRadius, true, true, false, true, nullptr, true);
		if (Distance <= AttackDistance && ControlledEnemy->CanAttack()) AttackPlayer();
		return;
	}
	const EEnemyAdaptiveTactic Tactic = GetActiveAdaptiveTactic();
	const bool bAirborne = TargetPlayer->GetCharacterMovement() && TargetPlayer->GetCharacterMovement()->IsFalling();
	if (Tactic == EEnemyAdaptiveTactic::PunishAirborne && bAirborne)
	{
		MoveTowardsPlayer(FMath::Max(AcceptanceRadius, AttackDistance * 0.75f));
		if (Distance <= AttackDistance && ControlledEnemy->CanAttack()) AttackPlayer();
		return;
	}
	if (Tactic == EEnemyAdaptiveTactic::StrafePlayer || Tactic == EEnemyAdaptiveTactic::DisplaceFromDestruction || Tactic == EEnemyAdaptiveTactic::FlankGuard)
	{
		if (Distance <= AttackDistance) AttackPlayer(); else MoveToAdaptivePosition(Tactic);
		return;
	}
	if (Distance <= AttackDistance) AttackPlayer();
	else if (Tactic == EEnemyAdaptiveTactic::RushPlayer) MoveTowardsPlayer(FMath::Max(AcceptanceRadius, AttackDistance * 0.65f));
	else if (Tactic == EEnemyAdaptiveTactic::MaintainRange) MoveTowardsPlayer(FMath::Max(AttackDistance, FollowDistance * ControlledEnemy->AdaptiveEngagementDistanceMultiplier));
	else MoveToPlayer();
}
void AEnemyAIController::MoveTowardsPlayer(float AcceptanceDistance) { if (TargetPlayer) MoveToActor(TargetPlayer, AcceptanceDistance, true, true, true, nullptr, true); }
void AEnemyAIController::MoveToAdaptivePosition(EEnemyAdaptiveTactic Tactic)
{
	if (!ControlledEnemy || !TargetPlayer || !GetWorld()) return;
	const float Now = GetWorld()->GetTimeSeconds();
	if (CurrentAdaptiveMoveTarget.IsNearlyZero() || FVector::Dist(ControlledEnemy->GetActorLocation(), CurrentAdaptiveMoveTarget) <= AcceptanceRadius || Now - LastAdaptiveMoveTime >= FMath::Max(0.25f, AdaptiveManeuverInterval))
	{
		const FVector ToPlayer = (TargetPlayer->GetActorLocation() - ControlledEnemy->GetActorLocation()).GetSafeNormal2D();
		const FVector Right = FVector::CrossProduct(FVector::UpVector, ToPlayer).GetSafeNormal();
		const float Distance = AdaptiveManeuverDistance * ControlledEnemy->AdaptiveEngagementDistanceMultiplier;
		CurrentAdaptiveMoveTarget = Tactic == EEnemyAdaptiveTactic::DisplaceFromDestruction ? TargetPlayer->GetActorLocation() - ToPlayer * Distance + Right * Distance * 0.5f * StrafeDirection : TargetPlayer->GetActorLocation() + Right * Distance * StrafeDirection;
		StrafeDirection *= -1; LastAdaptiveMoveTime = Now;
	}
	MoveToLocation(CurrentAdaptiveMoveTarget, AcceptanceRadius, true, true, false, true, nullptr, true);
}
void AEnemyAIController::MoveToPlayer() { if (TargetPlayer && ControlledEnemy) MoveTowardsPlayer(FMath::Max(AcceptanceRadius, FollowDistance * ControlledEnemy->AdaptiveEngagementDistanceMultiplier)); }
void AEnemyAIController::AttackPlayer()
{
	if (!ControlledEnemy || !TargetPlayer) return;
	StopMovement(); FVector Direction = TargetPlayer->GetActorLocation() - ControlledEnemy->GetActorLocation(); Direction.Z = 0.0f;
	if (!Direction.IsNearlyZero()) ControlledEnemy->SetActorRotation(Direction.Rotation());
	if (ControlledEnemy->CanAttack()) ControlledEnemy->PerformAttack();
}
void AEnemyAIController::Patrol()
{
	if (!ControlledEnemy || !GetWorld()) return;
	const float Now = GetWorld()->GetTimeSeconds(); bool bNew = CurrentPatrolPoint.IsZero();
	if (!bNew && FVector::Dist(ControlledEnemy->GetActorLocation(), CurrentPatrolPoint) <= AcceptanceRadius && Now - LastPatrolTime >= PatrolWaitTime) bNew = true;
	if (bNew) { CurrentPatrolPoint = GetRandomPatrolPoint(); LastPatrolTime = Now; MoveToLocation(CurrentPatrolPoint, AcceptanceRadius, true, true, false, true, nullptr, true); }
}
FVector AEnemyAIController::GetRandomPatrolPoint() const
{
	FVector Direction(FMath::FRandRange(-1.0f, 1.0f), FMath::FRandRange(-1.0f, 1.0f), 0.0f);
	return PatrolOrigin + Direction.GetSafeNormal() * FMath::FRandRange(PatrolRadius * 0.3f, PatrolRadius);
}
void AEnemyAIController::StartBehaviorTree(UBehaviorTree* Tree, UBlackboardData* Blackboard)
{
	if (!Tree) return;
	if (Blackboard && BlackboardComponent) UseBlackboard(Blackboard, BlackboardComponent);
	if (!RunBehaviorTree(Tree)) UE_LOG(LogTemp, Warning, TEXT("Failed to start Behavior Tree: %s"), *Tree->GetName());
}
void AEnemyAIController::StopBehaviorTree()
{
	if (UBehaviorTreeComponent* Tree = Cast<UBehaviorTreeComponent>(GetBrainComponent())) Tree->StopTree();
}
bool AEnemyAIController::IsUsingBehaviorTree() const { const UBrainComponent* Brain = GetBrainComponent(); return Brain && Brain->IsRunning(); }
