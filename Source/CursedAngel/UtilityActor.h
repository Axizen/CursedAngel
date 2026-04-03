// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CombatTypes.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "UtilityActor.generated.h"

class ACursedAngelCharacter;
class AFrankAI;

/**
 * Base class for utility actors that Frank can activate
 * Supports weapon-specific activation requirements and cooldown management
 */
UCLASS(BlueprintType, Blueprintable)
class CURSEDANGEL_API AUtilityActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AUtilityActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	/** Static mesh component for the utility visual */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* UtilityMesh;

	/** Activation volume for detecting player proximity */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* ActivationVolume;

	/** Particle system for highlighting when Frank can activate */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UParticleSystemComponent* HighlightVFX;

	/** Unique identifier for this utility type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility")
	FName UtilityType;

	/** Required weapon type to activate this utility */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility")
	ECurseWeaponType RequiredWeapon;

	/** Whether Frank must be present to activate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility")
	bool bRequiresFrank = true;

	/** Whether this utility is currently activated */
	UPROPERTY(BlueprintReadOnly, Category = "Utility")
	bool bIsActivated = false;

	/** Duration of the activation effect (0 = instant) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility")
	float ActivationDuration = 0.0f;

	/** Cooldown duration before utility can be activated again */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility")
	float CooldownDuration = 0.0f;

	/** Whether this utility is currently on cooldown */
	UPROPERTY(BlueprintReadOnly, Category = "Utility")
	bool bOnCooldown = false;

	/** Check if this utility can be activated */
	UFUNCTION(BlueprintCallable, Category = "Utility")
	virtual bool CanActivate(ACursedAngelCharacter* Player, AFrankAI* Frank);

	/** Activate this utility */
	UFUNCTION(BlueprintCallable, Category = "Utility")
	virtual void Activate(AFrankAI* Frank);

	/** Called when activation completes */
	UFUNCTION(BlueprintCallable, Category = "Utility")
	virtual void OnActivationComplete();

	/** Start cooldown timer */
	UFUNCTION(BlueprintCallable, Category = "Utility")
	void StartCooldown();

	/** End cooldown timer */
	UFUNCTION(BlueprintCallable, Category = "Utility")
	void EndCooldown();

	/** Enable or disable highlight effect */
	UFUNCTION(BlueprintCallable, Category = "Utility")
	void Highlight(bool bEnable);

	/** Blueprint event fired when utility is activated */
	UFUNCTION(BlueprintImplementableEvent, Category = "Utility")
	void OnUtilityActivated(AFrankAI* Frank);

	/** Blueprint event fired when utility is deactivated */
	UFUNCTION(BlueprintImplementableEvent, Category = "Utility")
	void OnUtilityDeactivated();

	/** Blueprint event fired when highlight state changes */
	UFUNCTION(BlueprintImplementableEvent, Category = "Utility")
	void OnHighlightChanged(bool bHighlighted);

protected:
	/** Timer handle for activation duration */
	FTimerHandle ActivationTimerHandle;

	/** Timer handle for cooldown */
	FTimerHandle CooldownTimerHandle;

	/** Current time remaining for activation */
	float ActivationTimeRemaining = 0.0f;
};
