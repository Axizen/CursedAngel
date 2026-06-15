// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ActionBase.h"
#include "Action_MeleeAttack.generated.h"

/**
 * UAction_MeleeAttack - Melee attack with DMC-style hitstop, enemy knockback, and combo cancel window.
 *
 * HITSTOP:
 *   On hit, both the attacker and each struck enemy have CustomTimeDilation set to HitstopTimeDilation
 *   (near-freeze). HitstopTimeRemaining is decremented in OnTick() using a real-time compensated delta
 *   (DeltaTime / HitstopTimeDilation) so the freeze always lasts exactly HitstopDuration real-world
 *   seconds regardless of the current time dilation value. NO timer delegates or FTimerHandle used.
 *
 * KNOCKBACK:
 *   After damage is applied, MeleeLaunchImpulse is fired in the attacker's forward direction.
 *   ACharacter enemies are launched via LaunchCharacter(); physics-enabled actors use AddImpulse().
 *
 * COMBO CANCEL WINDOW:
 *   When the active montage's playback position exceeds AttackCancelWindow (0-1 fraction of total
 *   length) AND ComboBuffer in ActionComponent holds a pending input, OnCancel() is invoked to chain
 *   early into the next attack without waiting for the montage to finish.
 *
 * Designer: Tune HitstopDuration, HitstopTimeDilation, MeleeLaunchImpulse, AttackCancelWindow
 *           in CharacterDataAsset->CombatFeel, or override per-action below.
 * Programmer: RestoreTimeDilation() is always called inline (from OnTick on expiry, from OnCancel
 *             on interrupt). No dangling timer handles. HitstopActors array tracks all frozen actors.
 */
UCLASS(Blueprintable)
class CURSEDANGEL_API UAction_MeleeAttack : public UActionBase
{
	GENERATED_BODY()

public:
	UAction_MeleeAttack();

	// =============================================================================
	// Melee Hit Properties
	// =============================================================================

	/** Damage dealt per successful hit.
	 *  Designer: Increase for harder-hitting attacks; balance against enemy health pools. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee Attack",
		meta = (Tooltip = "Damage dealt per hit.", ClampMin = "0.0"))
	float Damage = 30.0f;

	/** Style points awarded per hit to the StyleComponent rank system.
	 *  Designer: Higher values push the style rank meter faster. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee Attack",
		meta = (Tooltip = "Style points awarded to the DMC rank system per hit.", ClampMin = "0"))
	int32 StylePoints = 10;

	/** Distance in cm to trace in front of the character for hit detection.
	 *  Designer: Increase for longer reach; keep in line with animation reach. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee Attack",
		meta = (Tooltip = "Trace length in cm from character forward. Match to animation reach.", ClampMin = "10.0"))
	float TraceDistance = 200.0f;

	/** Sphere radius in cm for hit detection. Larger = more forgiving hitbox.
	 *  Designer: 50 cm is DMC-style generous; reduce for more precise gameplay. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee Attack",
		meta = (Tooltip = "Sphere trace radius in cm. Larger = more forgiving hitbox.", ClampMin = "1.0"))
	float TraceRadius = 50.0f;

	// =============================================================================
	// Knockback (Launch Impulse)
	// =============================================================================

	/** Force in cm/s applied to hit enemies in the attacker's forward direction.
	 *  Creates satisfying knockback feel on contact.
	 *  Designer: 600 is standard DMC-style knockback; increase for heavy attacks.
	 *  Programmer: ACharacter enemies use LaunchCharacter(); physics actors use AddImpulse(). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee Attack|Knockback",
		meta = (Tooltip = "Force applied to hit enemies in attacker-forward direction. Creates knockback feel.", ClampMin = "0.0"))
	float MeleeLaunchImpulse = 600.0f;

	// =============================================================================
	// Hitstop (Time Freeze on Hit)
	// =============================================================================

	/** Real-world seconds both the attacker and hit enemy are time-frozen on a successful hit.
	 *  0.05-0.08s = 3-5 frames at 60fps, which is the DMC-standard hitstop feel.
	 *  Designer: Increase for heavier hits; keep below 0.1s or it feels sluggish.
	 *  Programmer: Tracked via HitstopTimeRemaining, decremented in OnTick with no timer. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee Attack|Hitstop",
		meta = (Tooltip = "Real-time seconds both attacker and enemy freeze on hit. 0.05-0.08s = 3-5 frames at 60fps.", ClampMin = "0.0", ClampMax = "0.5"))
	float HitstopDuration = 0.06f;

	/** Time dilation multiplier applied to attacker and enemy during hitstop.
	 *  0.05 = near-freeze (5% speed). Applied via CustomTimeDilation on each actor.
	 *  Designer: Lower = more dramatic freeze; 0.05 is DMC-standard.
	 *  Programmer: Real-time delta in OnTick = DeltaTime / HitstopTimeDilation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee Attack|Hitstop",
		meta = (Tooltip = "Time scale during hitstop (0.05=near-freeze). Applied via CustomTimeDilation.", ClampMin = "0.01", ClampMax = "1.0"))
	float HitstopTimeDilation = 0.05f;

	// =============================================================================
	// Combo Cancel Window
	// =============================================================================

	/** Fraction of the attack montage (0.0-1.0) after which a buffered combo input can cancel
	 *  into the next hit. Enables DMC-style responsive combo chaining.
	 *  Designer: 0.6 = cancel available from 60% through the animation onward.
	 *            Lower values allow earlier cancels (more combo-friendly).
	 *  Programmer: Checked in OnTick via AnimInstance->Montage_GetPosition / GetPlayLength. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee Attack|Combo",
		meta = (Tooltip = "Fraction of attack montage (0-1) after which a buffered combo input can chain into the next hit.", ClampMin = "0.0", ClampMax = "1.0"))
	float AttackCancelWindow = 0.6f;

	// =============================================================================
	// Overrides
	// =============================================================================

	/** Called when the action is activated - starts montage, resets hit tracking and hitstop state. */
	virtual void OnActivate() override;

	/** Per-frame update - decrements hitstop timer (real-time compensated), checks combo cancel window.
	 *  Programmer: All state is via float fields - no timer delegates. */
	virtual void OnTick(float DeltaTime) override;

	/** Called when action is cancelled or interrupted - always restores time dilation first.
	 *  Programmer: Calls RestoreTimeDilation() before any other cleanup to prevent stuck freeze. */
	virtual void OnCancel() override;

protected:
	/** Perform a sphere trace in front of the character to find hit actors.
	 *  Applies damage, style points, knockback impulse, and triggers hitstop on each new hit.
	 *  Programmer: Guarded by HitActorsThisAttack to prevent double-hits per swing. */
	TArray<AActor*> PerformMeleeTrace();

private:
	// =============================================================================
	// Runtime State (not designer-exposed)
	// =============================================================================

	/** Actors struck this attack swing - prevents double-hits. Cleared in OnActivate. */
	TArray<AActor*> HitActorsThisAttack;

	/** Whether damage has been applied this attack (for AnimNotify-triggered damage path). */
	bool bDamageApplied = false;

	/** Actors currently under hitstop time dilation. Restored to 1.0 in RestoreTimeDilation().
	 *  Programmer: Includes both the owner/attacker and each struck enemy. */
	TArray<AActor*> HitstopActors;

	/** Real-world seconds of hitstop remaining. Decremented in OnTick using real-time compensation.
	 *  Programmer: Real-time delta = DeltaTime / HitstopTimeDilation (compensates for frozen time).
	 *  Set to HitstopDuration in PerformMeleeTrace when a hit lands. 0 = no active hitstop. */
	float HitstopTimeRemaining = 0.0f;

	/** Restore CustomTimeDilation=1.0 on all actors in HitstopActors.
	 *  Programmer: Called inline from OnTick (natural expiry) and OnCancel (interrupt cleanup).
	 *  No timer delegate - always called directly when the condition is met. */
	void RestoreTimeDilation();
};
