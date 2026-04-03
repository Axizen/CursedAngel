// Copyright Epic Games, Inc. All Rights Reserved.

#include "ReactiveArenaElement.h"
#include "Engine/World.h"
#include "TimerManager.h"

AReactiveArenaElement::AReactiveArenaElement()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create base mesh
    BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
    BaseMesh->SetupAttachment(RootComponent);
    BaseMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    BaseMesh->SetCollisionResponseToAllChannels(ECR_Block);

    // Create trigger volume
    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
    TriggerVolume->SetupAttachment(RootComponent);
    TriggerVolume->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
    TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    TriggerVolume->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
}

void AReactiveArenaElement::BeginPlay()
{
    Super::BeginPlay();
    
    // Bind overlap events
    if (TriggerVolume)
    {
        TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AReactiveArenaElement::OnTriggerVolumeBeginOverlap);
    }
}

void AReactiveArenaElement::TriggerReaction(ECurseWeaponType CurseType, FVector ImpactLocation)
{
    if (!CanReact(CurseType))
    {
        return;
    }
    
    bCurrentlyReacting = true;
    bIsActive = true;
    
    // Start reaction timer
    if (ReactionDuration > 0.0f)
    {
        GetWorld()->GetTimerManager().SetTimer(ReactionTimer, this, &AReactiveArenaElement::EndReaction, ReactionDuration, false);
    }
    
    // Trigger Blueprint event
    OnEnvironmentalReaction(CurseType, ImpactLocation);
}

bool AReactiveArenaElement::CanReact(ECurseWeaponType CurseType) const
{
    // Check if we're on cooldown
    if (bOnCooldown)
    {
        return false;
    }
    
    // Check if already reacting and can't reactivate
    if (bCurrentlyReacting && !bCanReactivate)
    {
        return false;
    }
    
    // Check if this element reacts to the specific curse type
    return ReactsToWeapon == CurseType;
}

void AReactiveArenaElement::ForceEndReaction()
{
    if (bCurrentlyReacting)
    {
        // Clear the timer
        GetWorld()->GetTimerManager().ClearTimer(ReactionTimer);
        EndReaction();
    }
}

void AReactiveArenaElement::OnTriggerVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // Check if the overlapping actor is a curse projectile or effect
    if (IsValid(OtherActor))
    {
        // Check if the actor has curse weapon component or is a curse projectile
        // This will be expanded when we create the curse projectiles
        FString ActorName = OtherActor->GetClass()->GetName();
        
        ECurseWeaponType DetectedCurseType = ECurseWeaponType::CorruptionRail;
        
        // Simple name-based detection (will be improved with proper projectile classes)
        if (ActorName.Contains(TEXT("Corruption")) || ActorName.Contains(TEXT("Rail")))
        {
            DetectedCurseType = ECurseWeaponType::CorruptionRail;
        }
        else if (ActorName.Contains(TEXT("Fragment")) || ActorName.Contains(TEXT("Needle")))
        {
            DetectedCurseType = ECurseWeaponType::FragmentNeedles;
        }
        else if (ActorName.Contains(TEXT("Void")) || ActorName.Contains(TEXT("Mine")))
        {
            DetectedCurseType = ECurseWeaponType::VoidMines;
        }
        
        // Trigger reaction if conditions are met
        if (CanReact(DetectedCurseType))
        {
            FVector ImpactLocation = SweepResult.ImpactPoint.IsZero() ? OtherActor->GetActorLocation() : FVector(SweepResult.ImpactPoint);
            TriggerReaction(DetectedCurseType, ImpactLocation);
        }
    }
}

void AReactiveArenaElement::EndReaction()
{
    if (!bCurrentlyReacting)
    {
        return;
    }
    
    bCurrentlyReacting = false;
    bIsActive = false;
    
    // Clear reaction timer
    GetWorld()->GetTimerManager().ClearTimer(ReactionTimer);
    
    // Start cooldown if applicable
    if (ReactivationCooldown > 0.0f)
    {
        bOnCooldown = true;
        GetWorld()->GetTimerManager().SetTimer(CooldownTimer, this, &AReactiveArenaElement::EndCooldown, ReactivationCooldown, false);
    }
    
    // Trigger Blueprint event
    OnReactionEnd();
}

void AReactiveArenaElement::EndCooldown()
{
    bOnCooldown = false;
    GetWorld()->GetTimerManager().ClearTimer(CooldownTimer);
    
    // Trigger Blueprint event
    OnReactionCooldownComplete();
}