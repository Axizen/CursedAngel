// Copyright Epic Games, Inc. All Rights Reserved.

#include "ActionBase.h"
#include "CursedAngel/Characters/CursedAngelCharacter.h"
#include "CursedAngel/Data/ActionDataAsset.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UActionBase::UActionBase()
	: ActionData(nullptr)
	, OwnerActor(nullptr)
	, bIsActive(false)
	, CurrentCooldown(0.0f)
{
}

// ===== Virtual Functions =====

void UActionBase::OnActivate()
{
	// Default implementation - override in derived classes
}

void UActionBase::OnTick(float DeltaTime)
{
	// Default implementation - override in derived classes
}

void UActionBase::OnCancel()
{
	// Default implementation - override in derived classes
}

bool UActionBase::CanActivate() const
{
	// Default implementation - can activate if not on cooldown
	return CurrentCooldown <= 0.0f;
}

// ===== Blueprint Native Events =====

bool UActionBase::BP_CanActivate_Implementation() const
{
	return CanActivate();
}

// ===== Helper Functions =====

void UActionBase::PlayMontage(UAnimMontage* Montage)
{
	if (!Montage || !OwnerActor)
	{
		return;
	}

	// Try to get the character and play the montage
	if (ACharacter* Character = Cast<ACharacter>(OwnerActor))
	{
		if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Play(Montage);
		}
	}
}

void UActionBase::ApplyDamageToTarget(AActor* Target, float Damage)
{
	if (!Target || !OwnerActor)
	{
		return;
	}

	// Apply damage using the gameplay statics system
	UGameplayStatics::ApplyDamage(
		Target,
		Damage,
		OwnerActor->GetInstigatorController(),
		OwnerActor,
		UDamageType::StaticClass()
	);
}

ACursedAngelCharacter* UActionBase::GetOwnerCharacter() const
{
	if (!OwnerActor)
	{
		return nullptr;
	}

	return Cast<ACursedAngelCharacter>(OwnerActor);
}
