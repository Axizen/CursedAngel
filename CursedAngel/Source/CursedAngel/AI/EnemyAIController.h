// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AI/EnemyManagerSubsystem.h"
#include "EnemyAIController.generated.h"

class AEnemyBase;
class ACursedAngelCharacter;
class UBehaviorTree;
class UBlackboardData;
class UBlackboardComponent;

/** AI controller supporting baseline combat plus tactics selected by UEnemyManagerSubsystem. */
UCLASS()
class CURSEDANGEL_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()
public:
	AEnemyAIController();
protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI", meta=(ToolTip="Baseline distance to maintain while following. Adaptive rules scale this value.")) float FollowDistance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI") float AttackDistance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI") float PatrolRadius;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI") bool bUsePatrol;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI") float PatrolWaitTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI") float AcceptanceRadius;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|Adaptive", meta=(ClampMin="0.25", ClampMax="3.0", ToolTip="How far an adaptive strafe/displacement maneuver moves from the player.")) float AdaptiveManeuverDistance = 350.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|Adaptive", meta=(ClampMin="0.25", ClampMax="5.0", ToolTip="Seconds between repositioning moves for strafe/displacement tactics.")) float AdaptiveManeuverInterval = 1.5f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI|Behavior Tree") UBlackboardComponent* BlackboardComponent;
	UFUNCTION(BlueprintPure, Category="AI|Adaptive") EEnemyAdaptiveTactic GetActiveAdaptiveTactic() const;
protected:
	UPROPERTY() AEnemyBase* ControlledEnemy;
	UPROPERTY() ACursedAngelCharacter* TargetPlayer;
	FVector PatrolOrigin;
	float LastPatrolTime;
	FVector CurrentPatrolPoint;
	FVector CurrentAdaptiveMoveTarget;
	float LastAdaptiveMoveTime = -1.0f;
	int32 StrafeDirection = 1;
	void UpdateAI(float DeltaTime);
	void MoveTowardsPlayer(float AcceptanceDistance);
	void MoveToAdaptivePosition(EEnemyAdaptiveTactic Tactic);
	void MoveToPlayer();
	void AttackPlayer();
	void Patrol();
	FVector GetRandomPatrolPoint() const;
public:
	void StartBehaviorTree(UBehaviorTree* BehaviorTree, UBlackboardData* BlackboardAsset);
	void StopBehaviorTree();
	bool IsUsingBehaviorTree() const;
};
