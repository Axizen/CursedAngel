// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CursedAngelCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "FrankDialogueTypes.h"
#include "FrankAI.generated.h"

class ACursedAngelCharacter;
class AFrankAIController;
class AUtilityActor;
class UDialogueComponent;

UENUM(BlueprintType)
enum class EFrankAIState : uint8
{
    Following,
    Investigating,
    ActivatingUtility,
    Idle
};

/**
 * AFrankAI - Frank the Fox companion character
 * Inherits combat capabilities from ACursedAngelCharacter base class
 * Provides utility activation, dialogue system, and AI companion behavior
 */
UCLASS(BlueprintType, Blueprintable)
class CURSEDANGEL_API AFrankAI : public ACursedAngelCharacter
{
    GENERATED_BODY()

public:
    AFrankAI();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Frank-specific components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* UtilityRadius;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AudioComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UDialogueComponent* DialogueComponent;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    ACursedAngelCharacter* PlayerReference;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float FollowDistance = 300.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float CatchUpDistance = 600.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float MovementSpeed = 400.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    bool bCanActivateUtilities = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float UtilityDetectionRadius = 500.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float UtilityActivationRange = 150.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float IdleTimeThreshold = 30.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float StuckTimeThreshold = 10.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float EnemyDetectionRadius = 1000.0f;
    
    UFUNCTION(BlueprintCallable, Category = "Frank AI")
    void SetAIState(EFrankAIState NewState);
    
    UFUNCTION(BlueprintCallable, Category = "Frank AI")
    EFrankAIState GetCurrentAIState() const { return CurrentState; }
    
    UFUNCTION(BlueprintCallable, Category = "Frank AI")
    void ActivateNearestUtility();
    
    UFUNCTION(BlueprintCallable, Category = "Frank AI")
    void HighlightReactiveElements();
    
    UFUNCTION(BlueprintCallable, Category = "Frank AI")
    void HighlightNearbyUtilities();
    
    UFUNCTION(BlueprintCallable, Category = "Frank AI")
    void SetPlayerReference(ACursedAngelCharacter* NewPlayerReference);
    
    UFUNCTION(BlueprintCallable, Category = "Frank AI")
    float GetDistanceToPlayer() const;
    
    UFUNCTION(BlueprintCallable, Category = "Frank AI")
    void TriggerContextDialogue(EDialogueContext Context);
    
    UFUNCTION(BlueprintCallable, Category = "Frank AI")
    void OnCombatEvent();
    
    UFUNCTION(BlueprintCallable, Category = "Frank AI")
    void OnPuzzleEvent();
    
    UFUNCTION(BlueprintCallable, Category = "Frank AI")
    void OnExplorationEvent();
    
    UFUNCTION(BlueprintCallable, Category = "Frank AI")
    void OnIdleTimeout();
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Frank AI")
    void OnUtilityActivated(AActor* UtilityActor);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Frank AI")
    void OnUtilityActivationComplete(AUtilityActor* Utility);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Frank AI")
    void OnStateChanged(EFrankAIState NewState, EFrankAIState OldState);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Frank AI")
    void OnReactiveElementsHighlighted(const TArray<AActor*>& ReactiveElements);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "AI")
    EFrankAIState CurrentState = EFrankAIState::Following;
    
    UPROPERTY(BlueprintReadOnly, Category = "AI")
    TArray<AUtilityActor*> NearbyUtilities;
    
    UPROPERTY(BlueprintReadOnly, Category = "AI")
    AUtilityActor* CurrentUtilityTarget = nullptr;
    
    UPROPERTY(BlueprintReadOnly, Category = "AI")
    TArray<AActor*> NearbyReactiveElements;
    
    // Dialogue tracking
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    float TimeSincePlayerMovement = 0.0f;
    
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    float TimeSincePlayerAction = 0.0f;
    
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FVector LastPlayerPosition;
    
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    bool bPlayerInCombat = false;
    
    // Internal functions that can be called by AI Controller
    friend class AFrankAIController;
    
    void UpdateNearbyUtilities();
    void UpdateNearbyReactiveElements();
    void UpdateDialogueContext(float DeltaTime);
    void CheckForCombat();
    void CheckForIdleState(float DeltaTime);
};
