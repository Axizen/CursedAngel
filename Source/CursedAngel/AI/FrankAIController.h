// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "AI/FrankAI.h"
#include "FrankAIController.generated.h"

class ACursedAngelCharacter;
class AFrankAI;

UCLASS(BlueprintType, Blueprintable)
class CURSEDANGEL_API AFrankAIController : public AAIController
{
    GENERATED_BODY()

public:
    AFrankAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

public:
    // AI State Management
    UFUNCTION(BlueprintCallable, Category = "Frank AI Controller")
    void SetAIState(EFrankAIState NewState);
    
    UFUNCTION(BlueprintCallable, Category = "Frank AI Controller")
    EFrankAIState GetCurrentAIState() const { return CurrentState; }
    
    UFUNCTION(BlueprintCallable, Category = "Frank AI Controller")
    void SetFollowTarget(AActor* Target);
    
    UFUNCTION(BlueprintCallable, Category = "Frank AI Controller")
    void SetUtilityTarget(AActor* Target);
    
    UFUNCTION(BlueprintCallable, Category = "Frank AI Controller")
    void SetInvestigationTarget(AActor* Target);
    
    UFUNCTION(BlueprintCallable, Category = "Frank AI Controller")
    void EnableScanning(bool bEnable);
    
    // Called by FrankAI when state changes
    void OnAIStateChanged(EFrankAIState NewState, EFrankAIState OldState);

protected:
    // AI Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float FollowDistance = 300.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float CatchUpDistance = 600.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float MovementSpeed = 400.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float UtilityActivationDistance = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float InvestigationDistance = 150.0f;
    
    UPROPERTY(BlueprintReadOnly, Category = "AI")
    bool bScanningEnabled = true;

private:
    // Current AI state
    EFrankAIState CurrentState = EFrankAIState::Following;
    
    // Target references
    UPROPERTY()
    AActor* FollowTarget = nullptr;
    
    UPROPERTY()
    AActor* UtilityTarget = nullptr;
    
    UPROPERTY()
    AActor* InvestigationTarget = nullptr;
    
    UPROPERTY()
    AFrankAI* FrankPawn = nullptr;
    
    // State timers
    float StateChangeTime = 0.0f;
    float UtilityActivationTimer = 0.0f;
    float InvestigationTimer = 0.0f;
    
    // AI Behavior Functions
    void UpdateFollowingBehavior(float DeltaTime);
    void UpdateInvestigatingBehavior(float DeltaTime);
    void UpdateActivatingUtilityBehavior(float DeltaTime);
    void UpdateIdleBehavior(float DeltaTime);
    
    // Movement Functions
    void MoveTowardsTarget(AActor* Target, float AcceptanceRadius = 100.0f);
    void StopMovement();
    bool IsAtTarget(AActor* Target, float AcceptanceRadius = 100.0f) const;
    
    // Utility Functions
    float GetDistanceToTarget(AActor* Target) const;
    bool IsValidTarget(AActor* Target) const;
    void HandleStateTransition();
    
    // Investigation Functions
    void StartInvestigation(AActor* Target);
    void CompleteInvestigation();
    
    // Utility Activation Functions
    void StartUtilityActivation(AActor* Target);
    void CompleteUtilityActivation();
};
