// Copyright Epic Games, Inc. All Rights Reserved.

#include "Characters/CursedAngelCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "CurseWeaponComponent.h"
#include "CursedAngelComponent.h"
#include "StyleComponent.h"
#include "FrankAI.h"
#include "CharacterDataAsset.h"
#include "HealthComponent.h"
#include "AirDashComponent.h"
#include "Components/ActionComponent.h"
#include "Components/CursedAngelCameraComponent.h"
#include "Animation/CursedAngelAnimInstance.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ACursedAngelCharacter

ACursedAngelCharacter::ACursedAngelCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	// NOTE: For advanced camera features (lock-on, dynamic modes), replace UCameraComponent
	// with UCursedAngelCameraComponent in the Blueprint-derived character class.
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Create curse weapon component
	CurseWeaponComponent = CreateDefaultSubobject<UCurseWeaponComponent>(TEXT("CurseWeaponComponent"));

	// Create cursed angel component
	CursedAngelComponent = CreateDefaultSubobject<UCursedAngelComponent>(TEXT("CursedAngelComponent"));

	// Create style component
	StyleComponent = CreateDefaultSubobject<UStyleComponent>(TEXT("StyleComponent"));

	// Create health component
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	// Create action component
	ActionComponent = CreateDefaultSubobject<UActionComponent>(TEXT("ActionComponent"));

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Initialization

void ACursedAngelCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Initialize from data asset if configured
	InitializeFromDataAsset();

	// NOTE: UCursedAngelAnimInstance (or a Blueprint child) should be set as the Anim Class
	// for the character's mesh component in the Blueprint-derived character class.
	// The anim instance handles linked animation layers, mesh swapping, and state management.
}

void ACursedAngelCharacter::LoadStatsFromDataAsset()
{
	if (!CharacterConfig)
	{
		return;
	}

	// Apply health stats
	if (HealthComponent)
	{
		HealthComponent->MaxHealth = CharacterConfig->MaxHealth;
		HealthComponent->CurrentHealth = CharacterConfig->MaxHealth;
	}

	// Apply movement stats
	if (UCharacterMovementComponent* CharMovement = GetCharacterMovement())
	{
		CharMovement->MaxWalkSpeed = CharacterConfig->MoveSpeed;
		CharMovement->JumpZVelocity = CharacterConfig->JumpHeight;
	}

	// Apply air dash count (find component in case it exists in subclass)
	if (UAirDashComponent* AirDash = FindComponentByClass<UAirDashComponent>())
	{
		AirDash->MaxAirDashes = CharacterConfig->AirDashCount;
	}

	// Apply weapon loadout
	if (CurseWeaponComponent)
	{
		// Set available weapons from data asset
		// Note: CurseWeaponComponent will need to support dynamic weapon loading
		// For now, we'll just set the starting weapon if it's in the available list
		if (CharacterConfig->AvailableWeapons.Num() > 0)
		{
			CurseWeaponComponent->CurrentWeapon = CharacterConfig->StartingWeapon;
		}
	}
}

void ACursedAngelCharacter::InitializeFromDataAsset()
{
	LoadStatsFromDataAsset();

	// Populate available actions from CharacterDataAsset
	if (CharacterConfig && ActionComponent)
	{
		ActionComponent->AvailableActions = CharacterConfig->AvailableActions;
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ACursedAngelCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ACursedAngelCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACursedAngelCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACursedAngelCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ACursedAngelCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ACursedAngelCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ACursedAngelCharacter::FireCurseWeapon(ECurseWeaponType WeaponType)
{
	if (!CurseWeaponComponent)
	{
		return;
	}

	// Fire the curse weapon (now parameterless, uses socket-based spawning)
	CurseWeaponComponent->FireCurseWeapon();
}

void ACursedAngelCharacter::SwitchCurseWeapon(ECurseWeaponType NewWeapon)
{
	if (CurseWeaponComponent)
	{
		CurseWeaponComponent->SwitchWeapon(NewWeapon);
	}
}

void ACursedAngelCharacter::ActivateCursedAngel()
{
	if (CursedAngelComponent && CursedAngelComponent->CanTransform())
	{
		CursedAngelComponent->ActivateTransformation();
	}
}

void ACursedAngelCharacter::RequestFrankUtility()
{
	if (IsValid(FrankCompanion))
	{
		FrankCompanion->ActivateNearestUtility();
	}
}

ECurseWeaponType ACursedAngelCharacter::GetCurrentCurseWeapon() const
{
	if (CurseWeaponComponent)
	{
		return CurseWeaponComponent->CurrentWeapon;
	}
	return ECurseWeaponType::CorruptionRail;
}

bool ACursedAngelCharacter::CanActivateCursedAngel() const
{
	if (CursedAngelComponent)
	{
		return CursedAngelComponent->CanTransform();
	}
	return false;
}

void ACursedAngelCharacter::OnEnemyKilled(AActor* Enemy, bool bEnvironmentalKill)
{
	if (StyleComponent && CurseWeaponComponent)
	{
		// Forward to style component with current weapon
		ECurseWeaponType CurrentWeapon = CurseWeaponComponent->CurrentWeapon;
		StyleComponent->OnEnemyKilled(Enemy, CurrentWeapon, bEnvironmentalKill);
	}
	
	// Notify Frank of combat event
	if (IsValid(FrankCompanion))
	{
		FrankCompanion->OnCombatEvent();
	}
}

UStyleComponent* ACursedAngelCharacter::GetStyleComponent() const
{
	return StyleComponent;
}

EStyleRank ACursedAngelCharacter::GetCurrentStyleRank() const
{
	if (StyleComponent)
	{
		return StyleComponent->GetCurrentRank();
	}
	return EStyleRank::D;
}

float ACursedAngelCharacter::GetStyleDamageMultiplier() const
{
	if (StyleComponent)
	{
		return StyleComponent->GetDamageMultiplier();
	}
	return 1.0f;
}


//////////////////////////////////////////////////////////////////////////
// Virtual Functions for Subclass Customization

void ACursedAngelCharacter::OnCombatAction_Implementation()
{
	// Base implementation does nothing - subclasses can override for custom behavior
	// Example: Ripley might trigger a visual effect, Frank might play a sound
}

void ACursedAngelCharacter::OnStyleRankChanged_Implementation(EStyleRank NewRank)
{
	// Base implementation does nothing - subclasses can override for custom behavior
	// Example: Ripley might trigger screen effects, Frank might change animation state
}

void ACursedAngelCharacter::OnCursedAngelActivated_Implementation()
{
	// Base implementation does nothing - subclasses can override for custom behavior
	// Example: Ripley might change appearance, Frank might gain temporary buffs
}

void ACursedAngelCharacter::OnCursedAngelDeactivated_Implementation()
{
	// Base implementation does nothing - subclasses can override for custom behavior
	// Example: Ripley might revert appearance, Frank might return to normal state
}

//////////////////////////////////////////////////////////////////////////
// Protected Helper Functions

void ACursedAngelCharacter::ApplyDamageToTarget(AActor* Target, float Damage)
{
	if (!Target)
	{
		return;
	}

	// Apply style-based damage multiplier
	float ModifiedDamage = Damage * GetStyleDamageMultiplier();

	// TODO: Apply actual damage to target using Unreal's damage system
	// UGameplayStatics::ApplyDamage(Target, ModifiedDamage, GetController(), this, UDamageType::StaticClass());
}

bool ACursedAngelCharacter::CanPerformCombatAction() const
{
	// Check if character is in a state that allows combat
	// For now, just check if we have required components
	return CurseWeaponComponent != nullptr && StyleComponent != nullptr;
}

void ACursedAngelCharacter::UpdateStyleMeter(float DeltaTime)
{
	if (StyleComponent)
	{
		// Update style meter decay or other time-based updates
		// This can be called from Tick if needed by subclasses
		// StyleComponent handles its own updates, so base implementation is empty
	}
}

//////////////////////////////////////////////////////////////////////////
// Action System

bool ACursedAngelCharacter::ExecuteAction(FName ActionName)
{
	if (ActionComponent)
	{
		return ActionComponent->ExecuteAction(ActionName);
	}
	return false;
}

void ACursedAngelCharacter::QueueAction(FName ActionName)
{
	if (ActionComponent)
	{
		ActionComponent->QueueAction(ActionName);
	}
}

void ACursedAngelCharacter::CancelCurrentAction()
{
	if (ActionComponent)
	{
		ActionComponent->CancelCurrentAction();
	}
}

//////////////////////////////////////////////////////////////////////////
// Camera and Animation Instance Helpers

UCursedAngelCameraComponent* ACursedAngelCharacter::GetCursedAngelCamera() const
{
	return FindComponentByClass<UCursedAngelCameraComponent>();
}

UCursedAngelAnimInstance* ACursedAngelCharacter::GetCursedAngelAnimInstance() const
{
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		return Cast<UCursedAngelAnimInstance>(MeshComp->GetAnimInstance());
	}
	return nullptr;
}
