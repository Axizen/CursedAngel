// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatTypes.h"
#include "TransformationTypes.h"
#include "Materials/MaterialParameterCollection.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "CursedAngelComponent.generated.h"

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CURSEDANGEL_API UCursedAngelComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCursedAngelComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Angel")
    float DataPoints = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Angel")
    float MaxDataPoints = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Angel")
    float TransformationDuration = 15.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Angel")
    float SpeedMultiplier = 1.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Angel")
    float DamageMultiplier = 2.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Angel")
    float MovementFluidityBonus = 1.3f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Angel")
    EStyleRank MinimumStyleRankForTransform = EStyleRank::A;
    
    // Transformation Visual Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation|Visuals")
    FTransformationVisuals Visuals;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation|VFX")
    FTransformationVFX VFXConfig;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation|Audio")
    FTransformationAudio AudioConfig;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation|Visuals")
    UMaterialParameterCollection* TransformMPC;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Transformation")
    ETransformationState CurrentState = ETransformationState::Normal;
    
    UFUNCTION(BlueprintCallable, Category = "Cursed Angel")
    void AddDataPoints(float Points);
    
    UFUNCTION(BlueprintCallable, Category = "Cursed Angel")
    void OnEnemyKilled(float StyleValue);
    
    UFUNCTION(BlueprintCallable, Category = "Cursed Angel")
    bool CanTransform() const;
    
    UFUNCTION(BlueprintCallable, Category = "Cursed Angel")
    void ActivateTransformation();
    
    UFUNCTION(BlueprintCallable, Category = "Cursed Angel")
    bool IsTransformed() const { return bIsTransformed; }
    
    UFUNCTION(BlueprintCallable, Category = "Cursed Angel")
    float GetTransformationTimeRemaining() const { return TransformationTimeRemaining; }
    
    UFUNCTION(BlueprintCallable, Category = "Cursed Angel")
    float GetDataPointsPercentage() const { return DataPoints / MaxDataPoints; }
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Cursed Angel")
    void OnTransformationStart();
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Cursed Angel")
    void OnTransformationEnd();
    
    // Delegate for transformation state changes
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTransformationStateChanged, bool, bTransformed);
    
    UPROPERTY(BlueprintAssignable, Category = "Cursed Angel")
    FOnTransformationStateChanged OnTransformationStateChangedDelegate;
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Cursed Angel")
    void OnDataPointsChanged(float NewDataPoints, float MaxPoints);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Transformation|Visuals")
    void OnVisualTransformationStart();
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Transformation|Visuals")
    void OnVisualTransformationEnd();
    
    UFUNCTION(BlueprintCallable, Category = "Transformation")
    ETransformationState GetCurrentState() const { return CurrentState; }

private:
    bool bIsTransformed = false;
    float TransformationTimeRemaining = 0.0f;
    
    // VFX Component References
    UPROPERTY()
    UNiagaraComponent* ActiveLoopVFX;
    
    UPROPERTY()
    UNiagaraComponent* ActiveWingVFX;
    
    UPROPERTY()
    UNiagaraComponent* ActiveAuraVFX;
    
    UPROPERTY()
    UNiagaraComponent* ActiveTrailVFX;
    
    // Audio Component References
    UPROPERTY()
    UAudioComponent* ActiveLoopAudio;
    
    UPROPERTY()
    UAudioComponent* ActiveAmbientAudio;
    
    // Material parameter animation
    float MaterialParameterTime = 0.0f;
    
    // Stored original materials for reverting
    UPROPERTY()
    TArray<UMaterialInterface*> StoredOriginalMaterials;
    
    void UpdateTransformation(float DeltaTime);
    void EndTransformation();
    
    // Visual transformation functions
    void ApplyVisualTransformation();
    void RevertVisualTransformation();
    void UpdateMaterialParameters(float DeltaTime);
    void SpawnTransformationVFX();
    void DestroyTransformationVFX();
};
