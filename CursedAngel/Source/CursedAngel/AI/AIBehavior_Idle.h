#pragma once

#include "CoreMinimal.h"
#include "AIBehaviorBase.h"
#include "Animation/AnimMontage.h"
#include "AIBehavior_Idle.generated.h"

/**
 * UAIBehavior_Idle
 * 
 * Fallback AI behavior that plays idle animations at intervals.
 * Always returns true for CanActivate, making it the lowest priority default behavior.
 * Can be extended in Blueprint for custom idle logic (patrol, look around, etc.).
 */
UCLASS(Blueprintable, BlueprintType)
class CURSEDANGEL_API UAIBehavior_Idle : public UAIBehaviorBase
{
	GENERATED_BODY()

public:
	UAIBehavior_Idle();

	// Properties
	
	/** Whether to play idle animations automatically */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
	bool bPlayIdleAnimations;

	/** Array of idle animation montages to randomly play */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
	TArray<UAnimMontage*> IdleMontages;

	/** Time interval between idle animations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
	float IdleAnimationInterval;

	// Overrides
	
	virtual void OnActivate() override;
	virtual void OnTick(float DeltaTime) override;
	virtual void OnDeactivate() override;
	virtual bool CanActivate() override;

private:
	/** Elapsed time since last idle animation */
	float ElapsedTime;

	/** Helper to play a random idle montage */
	void PlayRandomIdleMontage();
};
