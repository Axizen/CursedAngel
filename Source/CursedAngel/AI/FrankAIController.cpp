// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/FrankAIController.h"
#include "AI/FrankAI.h"
#include "Characters/CursedAngelCharacter.h"
#include "UtilityActor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"

AFrankAIController::AFrankAIController()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AFrankAIController::BeginPlay()
{
    Super::BeginPlay();
    
    StateChangeTime = GetWorld()->GetTimeSeconds();
}

void AFrankAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!FrankPawn)
    {
        return;
    }
    
    // Update AI behavior based on current state
    switch (CurrentState)
    {
        case EFrankAIState::Following:
            UpdateFollowingBehavior(DeltaTime);
            break;
            
        case EFrankAIState::Investigating:
            UpdateInvestigatingBehavior(DeltaTime);
            break;
            
        case EFrankAIState::ActivatingUtility:
            UpdateActivatingUtilityBehavior(DeltaTime);
            break;
            
        case EFrankAIState::Idle:
            UpdateIdleBehavior(DeltaTime);
            break;
    }
    
    HandleStateTransition();
}

void AFrankAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    FrankPawn = Cast<AFrankAI>(InPawn);
    if (FrankPawn)
    {
        // Set initial properties from pawn
        FollowDistance = FrankPawn->FollowDistance;
        CatchUpDistance = FrankPawn->CatchUpDistance;
        MovementSpeed = FrankPawn->MovementSpeed;
        
        // Set follow target to player reference
        if (FrankPawn->PlayerReference)
        {
            SetFollowTarget(FrankPawn->PlayerReference);
        }
    }
}

void AFrankAIController::SetAIState(EFrankAIState NewState)
{
    if (CurrentState != NewState)
    {
        EFrankAIState OldState = CurrentState;
        CurrentState = NewState;
        StateChangeTime = GetWorld()->GetTimeSeconds();
        
        // Update pawn state
        if (FrankPawn)
        {
            FrankPawn->SetAIState(NewState);
        }
    }
}

void AFrankAIController::SetFollowTarget(AActor* Target)
{
    FollowTarget = Target;
}

void AFrankAIController::SetUtilityTarget(AActor* Target)
{
    UtilityTarget = Target;
    if (Target)
    {
        SetAIState(EFrankAIState::ActivatingUtility);
    }
}

void AFrankAIController::SetInvestigationTarget(AActor* Target)
{
    InvestigationTarget = Target;
    if (Target)
    {
        SetAIState(EFrankAIState::Investigating);
    }
}

void AFrankAIController::EnableScanning(bool bEnable)
{
    bScanningEnabled = bEnable;
}

void AFrankAIController::OnAIStateChanged(EFrankAIState NewState, EFrankAIState OldState)
{
    CurrentState = NewState;
    StateChangeTime = GetWorld()->GetTimeSeconds();
}

void AFrankAIController::UpdateFollowingBehavior(float DeltaTime)
{
    if (!IsValidTarget(FollowTarget))
    {
        SetAIState(EFrankAIState::Idle);
        return;
    }
    
    float DistanceToTarget = GetDistanceToTarget(FollowTarget);
    
    if (DistanceToTarget > CatchUpDistance)
    {
        // Need to catch up quickly
        MoveTowardsTarget(FollowTarget, FollowDistance);
    }
    else if (DistanceToTarget > FollowDistance)
    {
        // Normal following
        MoveTowardsTarget(FollowTarget, FollowDistance);
    }
    else
    {
        // Close enough, stop moving
        StopMovement();
    }
}

void AFrankAIController::UpdateInvestigatingBehavior(float DeltaTime)
{
    if (!IsValidTarget(InvestigationTarget))
    {
        SetAIState(EFrankAIState::Following);
        return;
    }
    
    InvestigationTimer += DeltaTime;
    
    if (!IsAtTarget(InvestigationTarget, InvestigationDistance))
    {
        MoveTowardsTarget(InvestigationTarget, InvestigationDistance);
    }
    else
    {
        StopMovement();
        
        // Investigation complete after 3 seconds
        if (InvestigationTimer >= 3.0f)
        {
            CompleteInvestigation();
        }
    }
}

void AFrankAIController::UpdateActivatingUtilityBehavior(float DeltaTime)
{
    if (!IsValidTarget(UtilityTarget))
    {
        SetAIState(EFrankAIState::Following);
        return;
    }
    
    UtilityActivationTimer += DeltaTime;
    
    if (!IsAtTarget(UtilityTarget, UtilityActivationDistance))
    {
        MoveTowardsTarget(UtilityTarget, UtilityActivationDistance);
    }
    else
    {
        StopMovement();
        
        // Utility activation complete after 2 seconds
        if (UtilityActivationTimer >= 2.0f)
        {
            // Activate the utility if it's a valid AUtilityActor
            if (AUtilityActor* Utility = Cast<AUtilityActor>(UtilityTarget))
            {
                if (FrankPawn)
                {
                    Utility->Activate(FrankPawn);
                    FrankPawn->OnUtilityActivationComplete(Utility);
                    FrankPawn->CurrentUtilityTarget = nullptr;
                }
            }
            CompleteUtilityActivation();
        }
    }
}

void AFrankAIController::UpdateIdleBehavior(float DeltaTime)
{
    StopMovement();
    
    // Return to following if we have a follow target
    if (IsValidTarget(FollowTarget))
    {
        float TimeSinceStateChange = GetWorld()->GetTimeSeconds() - StateChangeTime;
        if (TimeSinceStateChange > 2.0f) // Idle for 2 seconds before returning to follow
        {
            SetAIState(EFrankAIState::Following);
        }
    }
}

void AFrankAIController::MoveTowardsTarget(AActor* Target, float AcceptanceRadius)
{
    if (!IsValidTarget(Target) || !FrankPawn)
    {
        return;
    }
    
    FVector TargetLocation = Target->GetActorLocation();
    FVector CurrentLocation = FrankPawn->GetActorLocation();
    FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
    
    // Calculate desired location (stay at acceptance radius distance)
    FVector DesiredLocation = TargetLocation - (Direction * AcceptanceRadius);
    
    // Move towards desired location
    MoveToLocation(DesiredLocation, AcceptanceRadius * 0.5f);
}

void AFrankAIController::StopMovement()
{
    StopMovement();
}

bool AFrankAIController::IsAtTarget(AActor* Target, float AcceptanceRadius) const
{
    if (!IsValidTarget(Target) || !FrankPawn)
    {
        return false;
    }
    
    float Distance = GetDistanceToTarget(Target);
    return Distance <= AcceptanceRadius;
}

float AFrankAIController::GetDistanceToTarget(AActor* Target) const
{
    if (!IsValidTarget(Target) || !FrankPawn)
    {
        return -1.0f;
    }
    
    return FVector::Dist(FrankPawn->GetActorLocation(), Target->GetActorLocation());
}

bool AFrankAIController::IsValidTarget(AActor* Target) const
{
    return IsValid(Target);
}

void AFrankAIController::HandleStateTransition()
{
    // Handle automatic state transitions based on conditions
    float TimeSinceStateChange = GetWorld()->GetTimeSeconds() - StateChangeTime;
    
    switch (CurrentState)
    {
        case EFrankAIState::Following:
            // Check if we should investigate something
            if (bScanningEnabled && FrankPawn && FrankPawn->NearbyReactiveElements.Num() > 0)
            {
                // Occasionally investigate reactive elements
                if (TimeSinceStateChange > 5.0f && FMath::RandRange(0.0f, 1.0f) < 0.3f)
                {
                    if (FrankPawn->NearbyReactiveElements.Num() > 0)
                    {
                        SetInvestigationTarget(FrankPawn->NearbyReactiveElements[0]);
                    }
                }
            }
            break;
            
        case EFrankAIState::Investigating:
            // Investigation timeout
            if (TimeSinceStateChange > 10.0f)
            {
                CompleteInvestigation();
            }
            break;
            
        case EFrankAIState::ActivatingUtility:
            // Utility activation timeout
            if (TimeSinceStateChange > 5.0f)
            {
                CompleteUtilityActivation();
            }
            break;
    }
}

void AFrankAIController::StartInvestigation(AActor* Target)
{
    InvestigationTarget = Target;
    InvestigationTimer = 0.0f;
    SetAIState(EFrankAIState::Investigating);
}

void AFrankAIController::CompleteInvestigation()
{
    InvestigationTarget = nullptr;
    InvestigationTimer = 0.0f;
    SetAIState(EFrankAIState::Following);
}

void AFrankAIController::StartUtilityActivation(AActor* Target)
{
    UtilityTarget = Target;
    UtilityActivationTimer = 0.0f;
    SetAIState(EFrankAIState::ActivatingUtility);
}

void AFrankAIController::CompleteUtilityActivation()
{
    UtilityTarget = nullptr;
    UtilityActivationTimer = 0.0f;
    SetAIState(EFrankAIState::Following);
}
