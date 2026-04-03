// Copyright Epic Games, Inc. All Rights Reserved.

#include "Actions/Action_Dodge.h"
#include "Data/ActionDataAsset.h"
#include "Characters/CursedAngelCharacter.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"

UAction_Dodge::UAction_Dodge()
{
	DodgeDirection = FVector::ForwardVector;
	DodgeSpeed = 1000.0f;
	DodgeDuration = 0.5f;
	bInvulnerable = true;
	ElapsedTime = 0.0f;
	OriginalCollisionType = ECollisionEnabled::QueryAndPhysics;
	bModifiedCollision = false;
}

void UAction_Dodge::OnActivate()
{
	if (!OwnerActor)
	{
		return;
	}

	// Reset elapsed time
	ElapsedTime = 0.0f;
	bIsActive = true;

	// Play montage if specified in ActionData
	if (ActionData && ActionData->ActionMontage)
	{
		PlayMontage(ActionData->ActionMontage);
	}

	// Apply launch velocity in dodge direction
	ACharacter* OwnerCharacter = Cast<ACharacter>(OwnerActor);
	if (OwnerCharacter)
	{
		// Transform dodge direction to world space
		FVector WorldDirection = OwnerActor->GetActorRotation().RotateVector(DodgeDirection.GetSafeNormal());
		FVector LaunchVelocity = WorldDirection * DodgeSpeed;
		
		// Apply launch velocity
		OwnerCharacter->LaunchCharacter(LaunchVelocity, true, true);

		// Handle invulnerability
		if (bInvulnerable)
		{
			UCapsuleComponent* CapsuleComp = OwnerCharacter->GetCapsuleComponent();
			if (CapsuleComp)
			{
				// Store original collision settings
				OriginalCollisionType = CapsuleComp->GetCollisionEnabled();
				bModifiedCollision = true;

				// Disable collision with enemies/projectiles while keeping world collision
				CapsuleComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
				CapsuleComp->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore); // Projectiles
			}
		}
	}
}

void UAction_Dodge::OnTick(float DeltaTime)
{
	if (!bIsActive)
	{
		return;
	}

	// Increment elapsed time
	ElapsedTime += DeltaTime;

	// Check if dodge duration has elapsed
	if (ElapsedTime >= DodgeDuration)
	{
		OnCancel();
	}
}

void UAction_Dodge::OnCancel()
{
	if (!OwnerActor)
	{
		bIsActive = false;
		return;
	}

	// Stop montage if playing
	ACharacter* OwnerCharacter = Cast<ACharacter>(OwnerActor);
	if (OwnerCharacter)
	{
		UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
		if (AnimInstance && ActionData && ActionData->ActionMontage)
		{
			AnimInstance->Montage_Stop(0.2f, ActionData->ActionMontage);
		}

		// Restore collision settings if modified
		if (bModifiedCollision && bInvulnerable)
		{
			UCapsuleComponent* CapsuleComp = OwnerCharacter->GetCapsuleComponent();
			if (CapsuleComp)
			{
				CapsuleComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
				CapsuleComp->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
			}
			bModifiedCollision = false;
		}
	}

	bIsActive = false;
}
