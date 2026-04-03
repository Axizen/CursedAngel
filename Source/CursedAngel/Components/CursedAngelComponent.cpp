// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/CursedAngelComponent.h"
#include "Engine/World.h"
#include "Characters/CursedAngelCharacter.h"
#include "Components/StyleComponent.h"
#include "FrankAI.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Materials/MaterialParameterCollectionInstance.h"

UCursedAngelComponent::UCursedAngelComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UCursedAngelComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UCursedAngelComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsTransformed)
    {
        UpdateTransformation(DeltaTime);
        UpdateMaterialParameters(DeltaTime);
    }
}

void UCursedAngelComponent::AddDataPoints(float Points)
{
    if (Points <= 0.0f)
    {
        return;
    }
    
    float OldDataPoints = DataPoints;
    DataPoints = FMath::Clamp(DataPoints + Points, 0.0f, MaxDataPoints);
    
    if (DataPoints != OldDataPoints)
    {
        OnDataPointsChanged(DataPoints, MaxDataPoints);
    }
}

bool UCursedAngelComponent::CanTransform() const
{
    if (bIsTransformed)
    {
        return false;
    }
    
    if (DataPoints < MaxDataPoints)
    {
        return false;
    }
    
    // Check if owner has StyleComponent and meets minimum rank requirement
    ACursedAngelCharacter* OwnerCharacter = Cast<ACursedAngelCharacter>(GetOwner());
    if (OwnerCharacter)
    {
        UStyleComponent* StyleComp = OwnerCharacter->GetStyleComponent();
        if (StyleComp)
        {
            EStyleRank CurrentRank = StyleComp->GetCurrentRank();
            return CurrentRank >= MinimumStyleRankForTransform;
        }
    }
    
    // If no StyleComponent found, only check data points (fallback)
    return true;
}

void UCursedAngelComponent::ActivateTransformation()
{
    if (!CanTransform())
    {
        return;
    }
    
    // Set state to transforming
    CurrentState = ETransformationState::Transforming;
    
    bIsTransformed = true;
    TransformationTimeRemaining = TransformationDuration;
    DataPoints = 0.0f;
    
    // Apply visual transformation
    ApplyVisualTransformation();
    
    // Spawn VFX
    SpawnTransformationVFX();
    
    // Play activation audio
    if (AudioConfig.ActivationSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, AudioConfig.ActivationSound, GetOwner()->GetActorLocation());
    }
    
    // Start loop audio
    if (AudioConfig.LoopSound)
    {
        ActiveLoopAudio = UGameplayStatics::SpawnSoundAttached(AudioConfig.LoopSound, GetOwner()->GetRootComponent());
    }
    
    // Start ambient audio
    if (AudioConfig.AmbientSound)
    {
        ActiveAmbientAudio = UGameplayStatics::SpawnSoundAttached(AudioConfig.AmbientSound, GetOwner()->GetRootComponent());
    }
    
    // Set state to transformed
    CurrentState = ETransformationState::Transformed;
    
    OnTransformationStart();
    OnVisualTransformationStart();
    OnDataPointsChanged(DataPoints, MaxDataPoints);
    OnTransformationStateChangedDelegate.Broadcast(true);
    
    // Notify Frank of transformation event
    ACursedAngelCharacter* OwnerCharacter = Cast<ACursedAngelCharacter>(GetOwner());
    if (OwnerCharacter && IsValid(OwnerCharacter->FrankCompanion))
    {
        OwnerCharacter->FrankCompanion->TriggerContextDialogue(EDialogueContext::Transformation);
    }
}

void UCursedAngelComponent::UpdateTransformation(float DeltaTime)
{
    if (!bIsTransformed)
    {
        return;
    }
    
    TransformationTimeRemaining -= DeltaTime;
    
    if (TransformationTimeRemaining <= 0.0f)
    {
        EndTransformation();
    }
}

void UCursedAngelComponent::EndTransformation()
{
    if (!bIsTransformed)
    {
        return;
    }
    
    // Set state to reverting
    CurrentState = ETransformationState::Reverting;
    
    bIsTransformed = false;
    TransformationTimeRemaining = 0.0f;
    
    // Revert visual transformation
    RevertVisualTransformation();
    
    // Destroy VFX
    DestroyTransformationVFX();
    
    // Play deactivation audio
    if (AudioConfig.DeactivationSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, AudioConfig.DeactivationSound, GetOwner()->GetActorLocation());
    }
    
    // Stop loop audio
    if (ActiveLoopAudio)
    {
        ActiveLoopAudio->Stop();
        ActiveLoopAudio = nullptr;
    }
    
    // Stop ambient audio
    if (ActiveAmbientAudio)
    {
        ActiveAmbientAudio->Stop();
        ActiveAmbientAudio = nullptr;
    }
    
    // Set state to normal
    CurrentState = ETransformationState::Normal;
    
    OnTransformationEnd();
    OnVisualTransformationEnd();
    OnTransformationStateChangedDelegate.Broadcast(false);
}

void UCursedAngelComponent::OnEnemyKilled(float StyleValue)
{
    if (StyleValue <= 0.0f)
    {
        return;
    }
    
    // Get style rank multiplier from owner's StyleComponent
    float RankMultiplier = 1.0f;
    ACursedAngelCharacter* OwnerCharacter = Cast<ACursedAngelCharacter>(GetOwner());
    if (OwnerCharacter)
    {
        UStyleComponent* StyleComp = OwnerCharacter->GetStyleComponent();
        if (StyleComp)
        {
            RankMultiplier = StyleComp->GetDamageMultiplier();
        }
    }
    
    // Scale data points by style rank multiplier
    float ScaledPoints = StyleValue * RankMultiplier;
    AddDataPoints(ScaledPoints);
}

void UCursedAngelComponent::ApplyVisualTransformation()
{
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        return;
    }
    
    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (!MeshComp)
    {
        return;
    }
    
    // Store original materials for reverting
    StoredOriginalMaterials.Empty();
    for (int32 i = 0; i < MeshComp->GetNumMaterials(); ++i)
    {
        StoredOriginalMaterials.Add(MeshComp->GetMaterial(i));
    }
    
    // Swap mesh if configured
    if (Visuals.TransformedMesh)
    {
        MeshComp->SetSkeletalMesh(Visuals.TransformedMesh);
    }
    
    // Apply transformed materials
    if (Visuals.TransformedMaterials.Num() > 0)
    {
        for (int32 i = 0; i < Visuals.TransformedMaterials.Num() && i < MeshComp->GetNumMaterials(); ++i)
        {
            if (Visuals.TransformedMaterials[i])
            {
                MeshComp->SetMaterial(i, Visuals.TransformedMaterials[i]);
            }
        }
    }
    
    // Reset material parameter time for pulsing effect
    MaterialParameterTime = 0.0f;
}

void UCursedAngelComponent::RevertVisualTransformation()
{
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        return;
    }
    
    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (!MeshComp)
    {
        return;
    }
    
    // Revert mesh if configured
    if (Visuals.NormalMesh)
    {
        MeshComp->SetSkeletalMesh(Visuals.NormalMesh);
    }
    
    // Revert to original materials
    if (StoredOriginalMaterials.Num() > 0)
    {
        for (int32 i = 0; i < StoredOriginalMaterials.Num() && i < MeshComp->GetNumMaterials(); ++i)
        {
            if (StoredOriginalMaterials[i])
            {
                MeshComp->SetMaterial(i, StoredOriginalMaterials[i]);
            }
        }
    }
    else if (Visuals.NormalMaterials.Num() > 0)
    {
        // Fallback to configured normal materials
        for (int32 i = 0; i < Visuals.NormalMaterials.Num() && i < MeshComp->GetNumMaterials(); ++i)
        {
            if (Visuals.NormalMaterials[i])
            {
                MeshComp->SetMaterial(i, Visuals.NormalMaterials[i]);
            }
        }
    }
    
    StoredOriginalMaterials.Empty();
}

void UCursedAngelComponent::UpdateMaterialParameters(float DeltaTime)
{
    if (!TransformMPC)
    {
        return;
    }
    
    MaterialParameterTime += DeltaTime;
    
    // Create pulsing glow effect using sine wave
    float PulseValue = FMath::Sin(MaterialParameterTime * 2.0f) * 0.5f + 0.5f; // 0 to 1
    float EmissiveValue = Visuals.EmissiveIntensity * PulseValue;
    
    UMaterialParameterCollectionInstance* MPCInstance = GetWorld()->GetParameterCollectionInstance(TransformMPC);
    if (MPCInstance)
    {
        MPCInstance->SetScalarParameterValue(FName("TransformEmissiveIntensity"), EmissiveValue);
        MPCInstance->SetVectorParameterValue(FName("TransformGlowColor"), Visuals.GlowColor);
    }
}

void UCursedAngelComponent::SpawnTransformationVFX()
{
    if (!GetOwner())
    {
        return;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FRotator OwnerRotation = GetOwner()->GetActorRotation();
    
    // Spawn activation VFX (one-shot)
    if (VFXConfig.ActivationVFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), VFXConfig.ActivationVFX, OwnerLocation, OwnerRotation);
    }
    
    // Spawn loop VFX (attached)
    if (VFXConfig.LoopVFX)
    {
        ActiveLoopVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
            VFXConfig.LoopVFX,
            GetOwner()->GetRootComponent(),
            NAME_None,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            true
        );
    }
    
    // Spawn wing VFX (attached)
    if (VFXConfig.WingVFX)
    {
        ActiveWingVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
            VFXConfig.WingVFX,
            GetOwner()->GetRootComponent(),
            NAME_None,
            FVector(0.0f, 0.0f, 50.0f), // Offset behind character
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            true
        );
    }
    
    // Spawn aura VFX (attached)
    if (VFXConfig.AuraVFX)
    {
        ActiveAuraVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
            VFXConfig.AuraVFX,
            GetOwner()->GetRootComponent(),
            NAME_None,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            true
        );
    }
    
    // Spawn trail VFX (attached)
    if (VFXConfig.TrailVFX)
    {
        ActiveTrailVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
            VFXConfig.TrailVFX,
            GetOwner()->GetRootComponent(),
            NAME_None,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            true
        );
    }
}

void UCursedAngelComponent::DestroyTransformationVFX()
{
    // Spawn deactivation VFX (one-shot)
    if (VFXConfig.DeactivationVFX && GetOwner())
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            VFXConfig.DeactivationVFX,
            GetOwner()->GetActorLocation(),
            GetOwner()->GetActorRotation()
        );
    }
    
    // Destroy loop VFX
    if (ActiveLoopVFX)
    {
        ActiveLoopVFX->DestroyComponent();
        ActiveLoopVFX = nullptr;
    }
    
    // Destroy wing VFX
    if (ActiveWingVFX)
    {
        ActiveWingVFX->DestroyComponent();
        ActiveWingVFX = nullptr;
    }
    
    // Destroy aura VFX
    if (ActiveAuraVFX)
    {
        ActiveAuraVFX->DestroyComponent();
        ActiveAuraVFX = nullptr;
    }
    
    // Destroy trail VFX
    if (ActiveTrailVFX)
    {
        ActiveTrailVFX->DestroyComponent();
        ActiveTrailVFX = nullptr;
    }
}
