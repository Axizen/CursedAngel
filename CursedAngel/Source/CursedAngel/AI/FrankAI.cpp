// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/FrankAI.h"
#include "Characters/CursedAngelCharacter.h"
#include "AI/FrankAIController.h"
#include "UtilityActor.h"
#include "Components/CurseWeaponComponent.h"
#include "Components/DialogueComponent.h"
#include "AI/EnemyBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

AFrankAI::AFrankAI()
{
    PrimaryActorTick.bCanEverTick = true;

    // Character base class already has CapsuleComponent and Mesh
    // Configure mesh for Frank (fox model)
    if (GetMesh())
    {
        GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -40.0f));
        GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    }

    // Create utility detection radius
    UtilityRadius = CreateDefaultSubobject<USphereComponent>(TEXT("UtilityRadius"));
    UtilityRadius->SetupAttachment(RootComponent);
    UtilityRadius->SetSphereRadius(UtilityDetectionRadius);
    UtilityRadius->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    UtilityRadius->SetCollisionResponseToAllChannels(ECR_Ignore);
    UtilityRadius->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;
    
    // Create dialogue component
    DialogueComponent = CreateDefaultSubobject<UDialogueComponent>(TEXT("DialogueComponent"));

    // Set default AI controller
    AIControllerClass = AFrankAIController::StaticClass();
}

void AFrankAI::BeginPlay()
{
    Super::BeginPlay();
    
    // Update utility radius size
    if (UtilityRadius)
    {
        UtilityRadius->SetSphereRadius(UtilityDetectionRadius);
    }
    
    // Initialize dialogue tracking
    if (IsValid(PlayerReference))
    {
        LastPlayerPosition = PlayerReference->GetActorLocation();
    }
}

void AFrankAI::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update nearby utilities and reactive elements
    UpdateNearbyUtilities();
    UpdateNearbyReactiveElements();
    
    // Update dialogue context detection
    UpdateDialogueContext(DeltaTime);
    
    // Check for combat state
    CheckForCombat();
    
    // Check for idle state
    CheckForIdleState(DeltaTime);
}

void AFrankAI::SetAIState(EFrankAIState NewState)
{
    if (CurrentState != NewState)
    {
        EFrankAIState OldState = CurrentState;
        CurrentState = NewState;
        OnStateChanged(NewState, OldState);
        
        // Notify AI Controller
        if (AFrankAIController* FrankController = Cast<AFrankAIController>(GetController()))
        {
            FrankController->OnAIStateChanged(NewState, OldState);
        }
    }
}

void AFrankAI::ActivateNearestUtility()
{
    if (!bCanActivateUtilities || NearbyUtilities.Num() == 0)
    {
        return;
    }
    
    // Find the nearest valid utility
    AUtilityActor* NearestUtility = nullptr;
    float NearestDistance = FLT_MAX;
    
    for (AUtilityActor* Utility : NearbyUtilities)
    {
        if (IsValid(Utility) && Utility->CanActivate(PlayerReference, this))
        {
            float Distance = FVector::Dist(GetActorLocation(), Utility->GetActorLocation());
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestUtility = Utility;
            }
        }
    }
    
    if (NearestUtility)
    {
        CurrentUtilityTarget = NearestUtility;
        SetAIState(EFrankAIState::ActivatingUtility);
        OnUtilityActivated(NearestUtility);
        
        // Notify AI Controller to handle movement to utility
        if (AFrankAIController* FrankController = Cast<AFrankAIController>(GetController()))
        {
            FrankController->SetUtilityTarget(NearestUtility);
        }
        
        // Check if already in range to activate immediately
        if (NearestDistance <= UtilityActivationRange)
        {
            NearestUtility->Activate(this);
            OnUtilityActivationComplete(NearestUtility);
            CurrentUtilityTarget = nullptr;
            SetAIState(EFrankAIState::Following);
            
            // Trigger puzzle dialogue
            TriggerContextDialogue(EDialogueContext::Puzzle);
        }
    }
}

void AFrankAI::HighlightReactiveElements()
{
    if (NearbyReactiveElements.Num() > 0)
    {
        OnReactiveElementsHighlighted(NearbyReactiveElements);
    }
}

void AFrankAI::SetPlayerReference(ACursedAngelCharacter* NewPlayerReference)
{
    PlayerReference = NewPlayerReference;
    
    // Notify AI Controller
    if (AFrankAIController* FrankController = Cast<AFrankAIController>(GetController()))
    {
        FrankController->SetFollowTarget(PlayerReference);
    }
}

float AFrankAI::GetDistanceToPlayer() const
{
    if (IsValid(PlayerReference))
    {
        return FVector::Dist(GetActorLocation(), PlayerReference->GetActorLocation());
    }
    return -1.0f;
}

void AFrankAI::UpdateNearbyUtilities()
{
    if (!UtilityRadius)
    {
        return;
    }
    
    TArray<AActor*> OverlappingActors;
    UtilityRadius->GetOverlappingActors(OverlappingActors);
    
    NearbyUtilities.Empty();
    
    for (AActor* Actor : OverlappingActors)
    {
        // Use proper class check for AUtilityActor
        if (AUtilityActor* Utility = Cast<AUtilityActor>(Actor))
        {
            // Filter by CanActivate check
            if (Utility->CanActivate(PlayerReference, this))
            {
                NearbyUtilities.Add(Utility);
            }
        }
    }
    
    // Sort by distance to player
    if (IsValid(PlayerReference) && NearbyUtilities.Num() > 1)
    {
        NearbyUtilities.Sort([this](const AUtilityActor& A, const AUtilityActor& B)
        {
            float DistA = FVector::Dist(PlayerReference->GetActorLocation(), A.GetActorLocation());
            float DistB = FVector::Dist(PlayerReference->GetActorLocation(), B.GetActorLocation());
            return DistA < DistB;
        });
    }
    
    // Highlight nearby utilities
    HighlightNearbyUtilities();
}

void AFrankAI::UpdateNearbyReactiveElements()
{
    if (!UtilityRadius)
    {
        return;
    }
    
    TArray<AActor*> OverlappingActors;
    UtilityRadius->GetOverlappingActors(OverlappingActors);
    
    NearbyReactiveElements.Empty();
    
    for (AActor* Actor : OverlappingActors)
    {
        // Check if actor is a reactive element (will be implemented through class check)
        if (IsValid(Actor) && Actor->GetClass()->GetName().Contains(TEXT("Reactive")))
        {
            NearbyReactiveElements.Add(Actor);
        }
    }
}

void AFrankAI::HighlightNearbyUtilities()
{
    // Highlight all nearby utilities
    for (AUtilityActor* Utility : NearbyUtilities)
    {
        if (IsValid(Utility))
        {
            Utility->Highlight(true);
        }
    }
}

void AFrankAI::TriggerContextDialogue(EDialogueContext Context)
{
    if (DialogueComponent)
    {
        DialogueComponent->TriggerDialogue(Context, false);
    }
}

void AFrankAI::OnCombatEvent()
{
    TriggerContextDialogue(EDialogueContext::Combat);
    TimeSincePlayerAction = 0.0f;
}

void AFrankAI::OnPuzzleEvent()
{
    TriggerContextDialogue(EDialogueContext::Puzzle);
    TimeSincePlayerAction = 0.0f;
}

void AFrankAI::OnExplorationEvent()
{
    TriggerContextDialogue(EDialogueContext::Exploration);
    TimeSincePlayerAction = 0.0f;
}

void AFrankAI::OnIdleTimeout()
{
    TriggerContextDialogue(EDialogueContext::Idle);
}

void AFrankAI::UpdateDialogueContext(float DeltaTime)
{
    if (!DialogueComponent || !IsValid(PlayerReference))
    {
        return;
    }
    
    // Update context based on current state
    EDialogueContext NewContext = EDialogueContext::Exploration;
    
    if (bPlayerInCombat)
    {
        NewContext = EDialogueContext::Combat;
    }
    else if (NearbyUtilities.Num() > 0)
    {
        NewContext = EDialogueContext::Puzzle;
    }
    else if (TimeSincePlayerMovement > IdleTimeThreshold)
    {
        NewContext = EDialogueContext::Idle;
    }
    
    DialogueComponent->UpdateContext(NewContext);
}

void AFrankAI::CheckForCombat()
{
    if (!IsValid(PlayerReference))
    {
        bPlayerInCombat = false;
        return;
    }
    
    // Check for nearby enemies
    TArray<AActor*> FoundEnemies;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyBase::StaticClass(), FoundEnemies);
    
    bPlayerInCombat = false;
    FVector PlayerLocation = PlayerReference->GetActorLocation();
    
    for (AActor* EnemyActor : FoundEnemies)
    {
        if (AEnemyBase* Enemy = Cast<AEnemyBase>(EnemyActor))
        {
            float Distance = FVector::Dist(PlayerLocation, Enemy->GetActorLocation());
            if (Distance <= EnemyDetectionRadius && Enemy->bIsAggro)
            {
                bPlayerInCombat = true;
                break;
            }
        }
    }
}

void AFrankAI::CheckForIdleState(float DeltaTime)
{
    if (!IsValid(PlayerReference))
    {
        return;
    }
    
    FVector CurrentPlayerPosition = PlayerReference->GetActorLocation();
    float MovementDistance = FVector::Dist(CurrentPlayerPosition, LastPlayerPosition);
    
    // Check if player has moved significantly
    if (MovementDistance > 10.0f)
    {
        TimeSincePlayerMovement = 0.0f;
        TimeSincePlayerAction = 0.0f;
        LastPlayerPosition = CurrentPlayerPosition;
    }
    else
    {
        TimeSincePlayerMovement += DeltaTime;
        TimeSincePlayerAction += DeltaTime;
        
        // Trigger idle dialogue
        if (TimeSincePlayerMovement >= IdleTimeThreshold)
        {
            OnIdleTimeout();
            TimeSincePlayerMovement = 0.0f; // Reset to avoid spam
        }
        
        // Trigger hint dialogue if player seems stuck
        if (TimeSincePlayerAction >= StuckTimeThreshold && !bPlayerInCombat)
        {
            TriggerContextDialogue(EDialogueContext::Hint);
            TimeSincePlayerAction = 0.0f; // Reset to avoid spam
        }
    }
}
