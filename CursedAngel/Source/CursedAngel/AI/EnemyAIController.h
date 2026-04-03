// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

class AEnemyBase;
class ACursedAngelCharacter;
class UBehaviorTree;
class UBlackboardData;
class UBlackboardComponent;

/**
 * AI Controller for enemy characters
 * Implements simple state machine for patrol, chase, and attack behaviors
 */
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
	// ========== Properties ==========

	/** Distance to maintain from player when following */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float FollowDistance;

	/** Distance at which enemy will attack */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float AttackDistance;

	/** Radius for patrol movement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float PatrolRadius;

	/** Whether this AI should patrol when idle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	bool bUsePatrol;

	/** Time between patrol point changes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float PatrolWaitTime;

	/** Acceptance radius for movement goals */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float AcceptanceRadius;

	/** Blackboard component for Behavior Tree data */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Behavior Tree")
	UBlackboardComponent* BlackboardComponent;

protected:
	// ========== Internal State ==========

	/** Cached reference to controlled enemy */
	UPROPERTY()
	AEnemyBase* ControlledEnemy;

	/** Cached reference to target player */
	UPROPERTY()
	ACursedAngelCharacter* TargetPlayer;

	/** Initial spawn location for patrol */
	FVector PatrolOrigin;

	/** Time of last patrol point change */
	float LastPatrolTime;

	/** Current patrol destination */
	FVector CurrentPatrolPoint;

	// ========== Functions ==========

	/**
	 * Update AI behavior based on current state
	 * @param DeltaTime Time since last update
	 */
	void UpdateAI(float DeltaTime);

	/**
	 * Move towards the player
	 */
	void MoveToPlayer();

	/**
	 * Stop and attack the player
	 */
	void AttackPlayer();

	/**
	 * Patrol around spawn point
	 */
	void Patrol();

	/**
	 * Generate a random patrol point within patrol radius
	 * @return Random location for patrol
	 */
	FVector GetRandomPatrolPoint() const;

public:
	// ========== Behavior Tree Functions ==========

	/**
	 * Start a Behavior Tree for this AI controller
	 * @param BehaviorTree The Behavior Tree asset to run
	 * @param BlackboardAsset The Blackboard data asset to use (optional)
	 */
	void StartBehaviorTree(UBehaviorTree* BehaviorTree, UBlackboardData* BlackboardAsset);

	/**
	 * Stop the currently running Behavior Tree
	 */
	void StopBehaviorTree();

	/**
	 * Check if this AI controller is currently using a Behavior Tree
	 * @return True if a Behavior Tree is running, false if using simple AI
	 */
	bool IsUsingBehaviorTree() const;
};
