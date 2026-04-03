// Copyright Epic Games, Inc. All Rights Reserved.

#include "UtilityActor.h"
#include "CursedAngelCharacter.h"
#include "FrankAI.h"
#include "CurseWeaponComponent.h"
#include "TimerManager.h"

AUtilityActor::AUtilityActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create root component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	// Create utility mesh
	UtilityMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("UtilityMesh"));
	UtilityMesh->SetupAttachment(RootComponent);
	UtilityMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	UtilityMesh->SetCollisionResponseToAllChannels(ECR_Block);

	// Create activation volume
	ActivationVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("ActivationVolume"));
	ActivationVolume->SetupAttachment(RootComponent);
	ActivationVolume->SetBoxExtent(FVector(200.0f, 200.0f, 200.0f));
	ActivationVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ActivationVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	ActivationVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Create highlight VFX component
	HighlightVFX = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("HighlightVFX"));
	HighlightVFX->SetupAttachment(RootComponent);
	HighlightVFX->bAutoActivate = false;

	// Default values
	UtilityType = FName("Generic");
	RequiredWeapon = ECurseWeaponType::CorruptionRail;
	bRequiresFrank = true;
	bIsActivated = false;
	ActivationDuration = 0.0f;
	CooldownDuration = 5.0f;
	bOnCooldown = false;
	ActivationTimeRemaining = 0.0f;
}

void AUtilityActor::BeginPlay()
{
	Super::BeginPlay();
	
	// Ensure highlight is off at start
	if (HighlightVFX)
	{
		HighlightVFX->Deactivate();
	}
}

void AUtilityActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update activation timer if active
	if (bIsActivated && ActivationDuration > 0.0f)
	{
		ActivationTimeRemaining -= DeltaTime;
		if (ActivationTimeRemaining <= 0.0f)
		{
			OnActivationComplete();
		}
	}
}

bool AUtilityActor::CanActivate(ACursedAngelCharacter* Player, AFrankAI* Frank)
{
	// Check if already activated
	if (bIsActivated)
	{
		return false;
	}

	// Check if on cooldown
	if (bOnCooldown)
	{
		return false;
	}

	// Check if Frank is required and present
	if (bRequiresFrank && !Frank)
	{
		return false;
	}

	// Check if player is valid
	if (!Player)
	{
		return false;
	}

	// Check if player has the required weapon equipped
	UCurseWeaponComponent* WeaponComponent = Player->GetCurseWeaponComponent();
	if (!WeaponComponent)
	{
		return false;
	}

	// Check if player has the required weapon equipped
	if (WeaponComponent->CurrentWeapon != RequiredWeapon)
	{
		return false;
	}

	// Check if player is within activation volume
	if (ActivationVolume)
	{
		TArray<AActor*> OverlappingActors;
		ActivationVolume->GetOverlappingActors(OverlappingActors, ACursedAngelCharacter::StaticClass());
		
		if (OverlappingActors.Num() == 0)
		{
			return false;
		}
	}

	return true;
}

void AUtilityActor::Activate(AFrankAI* Frank)
{
	// Verify we can activate
	if (!CanActivate(Cast<ACursedAngelCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn()), Frank))
	{
		return;
	}

	// Set activated state
	bIsActivated = true;

	// Start activation timer if duration is set
	if (ActivationDuration > 0.0f)
	{
		ActivationTimeRemaining = ActivationDuration;
		GetWorldTimerManager().SetTimer(
			ActivationTimerHandle,
			this,
			&AUtilityActor::OnActivationComplete,
			ActivationDuration,
			false
		);
	}
	else
	{
		// Instant activation
		OnActivationComplete();
	}

	// Fire Blueprint event for VFX/audio/animation
	OnUtilityActivated(Frank);
}

void AUtilityActor::OnActivationComplete()
{
	// Clear activation timer
	if (ActivationTimerHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(ActivationTimerHandle);
	}

	// Reset activation state
	bIsActivated = false;
	ActivationTimeRemaining = 0.0f;

	// Fire Blueprint event
	OnUtilityDeactivated();

	// Start cooldown if applicable
	if (CooldownDuration > 0.0f)
	{
		StartCooldown();
	}
}

void AUtilityActor::StartCooldown()
{
	bOnCooldown = true;

	GetWorldTimerManager().SetTimer(
		CooldownTimerHandle,
		this,
		&AUtilityActor::EndCooldown,
		CooldownDuration,
		false
	);
}

void AUtilityActor::EndCooldown()
{
	bOnCooldown = false;

	// Clear cooldown timer
	if (CooldownTimerHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(CooldownTimerHandle);
	}
}

void AUtilityActor::Highlight(bool bEnable)
{
	if (!HighlightVFX)
	{
		return;
	}

	if (bEnable)
	{
		HighlightVFX->Activate(true);
	}
	else
	{
		HighlightVFX->Deactivate();
	}

	// Fire Blueprint event
	OnHighlightChanged(bEnable);
}
