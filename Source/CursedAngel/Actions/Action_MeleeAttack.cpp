// Copyright Epic Games, Inc. All Rights Reserved.

#include "Action_MeleeAttack.h"
#include "ActionDataAsset.h"
#include "Components/StyleComponent.h"
#include "Characters/CursedAngelCharacter.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"

UAction_MeleeAttack::UAction_MeleeAttack()
{
	Damage = 30.0f;
	StylePoints = 10;
	TraceDistance = 200.0f;
	TraceRadius = 50.0f;
	bDamageApplied = false;
}

void UAction_MeleeAttack::OnActivate()
{
	Super::OnActivate();

	// Reset hit tracking
	HitActorsThisAttack.Empty();
	bDamageApplied = false;

	// Play montage from ActionData if available
	if (ActionData && ActionData->ActionMontage)
	{
		if (ACursedAngelCharacter* Character = GetOwnerCharacter())
		{
			if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
			{
				AnimInstance->Montage_Play(ActionData->ActionMontage);
				bIsActive = true;
			}
		}
	}
	else
	{
		// No montage, just activate
		bIsActive = true;
	}
}

void UAction_MeleeAttack::OnTick(float DeltaTime)
{
	Super::OnTick(DeltaTime);

	if (!bIsActive)
	{
		return;
	}

	// Check if montage is still playing
	if (ActionData && ActionData->ActionMontage)
	{
		if (ACursedAngelCharacter* Character = GetOwnerCharacter())
		{
			if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
			{
				if (!AnimInstance->Montage_IsPlaying(ActionData->ActionMontage))
				{
					// Montage finished, end action
					bIsActive = false;
					return;
				}
			}
		}
	}

	// Note: Actual damage application should be triggered by AnimNotify in the montage
	// This is a fallback or for continuous hit detection
	// The AnimNotify would call a Blueprint function that triggers PerformMeleeTrace
}

void UAction_MeleeAttack::OnCancel()
{
	Super::OnCancel();

	// Stop montage if playing
	if (ActionData && ActionData->ActionMontage)
	{
		if (ACursedAngelCharacter* Character = GetOwnerCharacter())
		{
			if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
			{
				if (AnimInstance->Montage_IsPlaying(ActionData->ActionMontage))
				{
					AnimInstance->Montage_Stop(0.2f, ActionData->ActionMontage);
				}
			}
		}
	}

	bIsActive = false;
	HitActorsThisAttack.Empty();
}

TArray<AActor*> UAction_MeleeAttack::PerformMeleeTrace()
{
	TArray<AActor*> HitActors;

	ACursedAngelCharacter* Character = GetOwnerCharacter();
	if (!Character)
	{
		return HitActors;
	}

	// Get trace start and end positions
	FVector StartLocation = Character->GetActorLocation();
	FVector ForwardVector = Character->GetActorForwardVector();
	FVector EndLocation = StartLocation + (ForwardVector * TraceDistance);

	// Perform sphere trace
	TArray<FHitResult> HitResults;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Character);

	bool bHit = UKismetSystemLibrary::SphereTraceMulti(
		Character->GetWorld(),
		StartLocation,
		EndLocation,
		TraceRadius,
		UEngineTypes::ConvertToTraceType(ECC_Pawn),
		false, // bTraceComplex
		ActorsToIgnore,
		EDrawDebugTrace::None, // Change to ForDuration for debugging
		HitResults,
		true // bIgnoreSelf
	);

	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor && !HitActorsThisAttack.Contains(HitActor))
			{
				// Filter by enemy tag (check if actor has "Enemy" tag)
				if (HitActor->ActorHasTag(FName("Enemy")))
				{
					HitActors.Add(HitActor);
					HitActorsThisAttack.Add(HitActor);

					// Apply damage
					UGameplayStatics::ApplyDamage(
						HitActor,
						Damage,
						Character->GetController(),
						Character,
						UDamageType::StaticClass()
					);

					// Add style points if StyleComponent exists
					if (UStyleComponent* StyleComp = Character->FindComponentByClass<UStyleComponent>())
					{
						StyleComp->AddStylePoints(StylePoints);
					}
				}
			}
		}
	}

	return HitActors;
}
