#include "AIBehavior_Idle.h"
#include "Characters/CursedAngelCharacter.h"
#include "GameFramework/Character.h"

UAIBehavior_Idle::UAIBehavior_Idle()
{
	BehaviorName = FName("Idle");
	Priority = 0; // Lowest priority - fallback behavior
	bPlayIdleAnimations = true;
	IdleAnimationInterval = 5.0f;
	ElapsedTime = 0.0f;
}

bool UAIBehavior_Idle::CanActivate()
{
	// Idle behavior is always available as a fallback
	return true;
}

void UAIBehavior_Idle::OnActivate()
{
	Super::OnActivate();
	
	bIsActive = true;
	ElapsedTime = 0.0f;
}

void UAIBehavior_Idle::OnTick(float DeltaTime)
{
	Super::OnTick(DeltaTime);
	
	if (!bIsActive)
	{
		return;
	}

	ElapsedTime += DeltaTime;

	// Play idle animation at intervals
	if (bPlayIdleAnimations && ElapsedTime >= IdleAnimationInterval)
	{
		PlayRandomIdleMontage();
		ElapsedTime = 0.0f;
	}
}

void UAIBehavior_Idle::OnDeactivate()
{
	Super::OnDeactivate();
	
	bIsActive = false;

	// Stop any playing montage
	if (OwnerActor)
	{
		if (ACharacter* Character = Cast<ACharacter>(OwnerActor))
		{
			if (USkeletalMeshComponent* Mesh = Character->GetMesh())
			{
				if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
				{
					AnimInstance->Montage_Stop(0.2f);
				}
			}
		}
	}
}

void UAIBehavior_Idle::PlayRandomIdleMontage()
{
	if (IdleMontages.Num() == 0 || !OwnerActor)
	{
		return;
	}

	// Select a random idle montage
	int32 RandomIndex = FMath::RandRange(0, IdleMontages.Num() - 1);
	UAnimMontage* SelectedMontage = IdleMontages[RandomIndex];

	if (!SelectedMontage)
	{
		return;
	}

	// Play the montage on the character
	if (ACharacter* Character = Cast<ACharacter>(OwnerActor))
	{
		if (USkeletalMeshComponent* Mesh = Character->GetMesh())
		{
			if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
			{
				AnimInstance->Montage_Play(SelectedMontage);
			}
		}
	}
}
