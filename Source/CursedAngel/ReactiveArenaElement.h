// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "CurseWeaponComponent.h"
#include "ReactiveArenaElement.generated.h"

UCLASS(BlueprintType, Blueprintable)
class CURSEDANGEL_API AReactiveArenaElement : public AActor
{
    GENERATED_BODY()

public:
    AReactiveArenaElement();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* BaseMesh;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* TriggerVolume;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reactive")
    ECurseWeaponType ReactsToWeapon = ECurseWeaponType::CorruptionRail;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reactive")
    EReactionType ReactionBehavior = EReactionType::PlatformExtension;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reactive")
    float ReactionDuration = 10.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reactive")
    bool bIsActive = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reactive")
    bool bCanReactivate = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reactive")
    float ReactivationCooldown = 5.0f;
    
    UFUNCTION(BlueprintCallable, Category = "Reactive")
    void TriggerReaction(ECurseWeaponType CurseType, FVector ImpactLocation);
    
    UFUNCTION(BlueprintCallable, Category = "Reactive")
    bool CanReact(ECurseWeaponType CurseType) const;
    
    UFUNCTION(BlueprintCallable, Category = "Reactive")
    void ForceEndReaction();
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Reactive")
    void OnEnvironmentalReaction(ECurseWeaponType CurseType, FVector ImpactLocation);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Reactive")
    void OnReactionEnd();
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Reactive")
    void OnReactionCooldownComplete();

protected:
    UFUNCTION()
    void OnTriggerVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
    bool bCurrentlyReacting = false;
    bool bOnCooldown = false;
    FTimerHandle ReactionTimer;
    FTimerHandle CooldownTimer;
    
    void EndReaction();
    void EndCooldown();
};