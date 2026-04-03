// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Frank.h"
#include "Characters/CursedAngelCharacter.h"
#include "CurseWeaponComponent.h"
#include "AIController.h"
#include "CharacterDataAsset.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Components/AIBehaviorComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"

#include "Physics/HairPhysicsComponent.h"

// Sets default values
AFrank::AFrank() : ACursedAngelCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Default to Companion mode
	CurrentMode = EFrankMode::Companion;

	// Create AI behavior component
	AIBehaviorComponent = CreateDefaultSubobject<UAIBehaviorComponent>(TEXT("AIBehaviorComponent"));

	// Physics components (FurPhysics) will be added in Blueprint
	// AI behavior will be controlled via Behavior Tree (BT_FrankCompanion)
}

// Called when the game starts or when spawned
void AFrank::BeginPlay()
{
	Super::BeginPlay();

	// Initialize mode-specific behavior
	if (CurrentMode == EFrankMode::Companion)
	{
		// Enable AI control for companion mode
		AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	}
}

// Called to bind functionality to input
void AFrank::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Future: Bind input actions when Frank becomes playable
	// if (bIsPlayable)
	// {
	//     PlayerInputComponent->BindAction("FrankAttack", IE_Pressed, this, &AFrank::OnFrankAttack);
	//     PlayerInputComponent->BindAction("FrankSpecialAbility", IE_Pressed, this, &AFrank::OnFrankSpecialAbility);
	// }
}

// Set Frank's mode (Companion or Playable)
void AFrank::SetMode(EFrankMode NewMode)
{
	CurrentMode = NewMode;

	if (CurrentMode == EFrankMode::Companion)
	{
		// Enable AI control
		if (AAIController* AIController = Cast<AAIController>(GetController()))
		{
			AIController->RunBehaviorTree(CharacterConfig ? CharacterConfig->CompanionBehaviorTree : nullptr);
		}
	}
	else if (CurrentMode == EFrankMode::Playable)
	{
		// Disable AI control, enable player input
		// This is a framework for future playability
		if (AAIController* AIController = Cast<AAIController>(GetController()))
		{
			AIController->StopMovement();
		}
	}
}

// Follow the player character (AI companion behavior)
void AFrank::FollowPlayer(ACursedAngelCharacter* Player)
{
	if (!Player)
	{
		return;
	}

	AAIController* AIController = Cast<AAIController>(GetController());
	if (!AIController)
	{
		return;
	}

	// Get follow distance from CharacterConfig
	float FollowDist = CharacterConfig ? CharacterConfig->FollowDistance : 300.0f;

	// Move to player location with follow distance offset
	FVector TargetLocation = Player->GetActorLocation();
	float DistanceToPlayer = FVector::Dist(GetActorLocation(), TargetLocation);

	// Only move if beyond follow distance
	if (DistanceToPlayer > FollowDist)
	{
		AIController->MoveToActor(Player, FollowDist);
	}
}

// Assist in combat by attacking a target (AI companion behavior)
void AFrank::AssistInCombat(AActor* Target)
{
	if (!Target)
	{
		return;
	}

	// Check if target is in combat range
	float CombatDist = CharacterConfig ? CharacterConfig->CombatRange : 500.0f;
	float DistanceToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());

	if (DistanceToTarget <= CombatDist)
	{
		// Face the target
		FVector DirectionToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		SetActorRotation(DirectionToTarget.Rotation());

		// Fire weapon if available
		if (UCurseWeaponComponent* WeaponComp = FindComponentByClass<UCurseWeaponComponent>())
		{
			WeaponComp->FireCurseWeapon();
		}
	}
	else
	{
		// Move closer to target
		if (AAIController* AIController = Cast<AAIController>(GetController()))
		{
			AIController->MoveToActor(Target, CombatDist * 0.8f); // Move to 80% of combat range
		}
	}
}

// Initialize from data asset (populate AI behaviors)
void AFrank::InitializeFromDataAsset()
{
	// Call parent implementation to populate actions
	Super::InitializeFromDataAsset();

	// Populate AI behaviors from CharacterDataAsset
	if (CharacterConfig && AIBehaviorComponent && CharacterConfig->CharacterRole == ECharacterRole::Companion)
	{
		AIBehaviorComponent->AvailableBehaviors = CharacterConfig->AIBehaviors;
	}
}