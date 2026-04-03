// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/HealthComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Default values
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;
	bIsDead = false;
	bInvulnerable = false;
	InvulnerabilityDuration = 0.0f;
	LastDamageDealer = nullptr;
	LastDamageType = EDamageType::Physical;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize current health to max health
	CurrentHealth = MaxHealth;
	bIsDead = false;
	bInvulnerable = false;
}

void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

float UHealthComponent::TakeDamage(float Damage, EDamageType Type, AActor* DamageDealer)
{
	// Cannot take damage if dead or invulnerable
	if (bIsDead || bInvulnerable)
	{
		return 0.0f;
	}

	// Clamp damage to positive values
	Damage = FMath::Max(0.0f, Damage);

	// Store damage dealer and type for death attribution
	LastDamageDealer = DamageDealer;
	LastDamageType = Type;

	// Apply damage
	CurrentHealth = FMath::Max(0.0f, CurrentHealth - Damage);

	// Broadcast damage taken event
	OnDamageTaken.Broadcast(Damage, Type);

	// Broadcast health changed event
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	// Check for death
	if (CurrentHealth <= 0.0f)
	{
		Kill();
	}
	else if (InvulnerabilityDuration > 0.0f)
	{
		// Apply invulnerability frames
		bInvulnerable = true;

		// Clear existing timer if any
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(InvulnerabilityTimerHandle);
			World->GetTimerManager().SetTimer(
				InvulnerabilityTimerHandle,
				this,
				&UHealthComponent::EndInvulnerability,
				InvulnerabilityDuration,
				false
			);
		}
	}

	return Damage;
}

float UHealthComponent::Heal(float Amount)
{
	// Cannot heal if dead
	if (bIsDead)
	{
		return 0.0f;
	}

	// Clamp heal amount to positive values
	Amount = FMath::Max(0.0f, Amount);

	// Calculate actual heal amount (cannot exceed max health)
	float OldHealth = CurrentHealth;
	CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + Amount);
	float ActualHealAmount = CurrentHealth - OldHealth;

	// Broadcast health changed event if health actually changed
	if (ActualHealAmount > 0.0f)
	{
		OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	}

	return ActualHealAmount;
}

void UHealthComponent::Kill()
{
	// Already dead, do nothing
	if (bIsDead)
	{
		return;
	}

	// Set death state
	bIsDead = true;
	CurrentHealth = 0.0f;

	// Clear invulnerability timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(InvulnerabilityTimerHandle);
	}
	bInvulnerable = false;

	// Broadcast death event
	OnDeath.Broadcast(LastDamageDealer, LastDamageType);

	// Broadcast final health changed event
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

bool UHealthComponent::IsAlive() const
{
	return !bIsDead && CurrentHealth > 0.0f;
}

float UHealthComponent::GetHealthPercent() const
{
	if (MaxHealth <= 0.0f)
	{
		return 0.0f;
	}

	return CurrentHealth / MaxHealth;
}

void UHealthComponent::SetInvulnerable(bool bNewInvulnerable)
{
	bInvulnerable = bNewInvulnerable;

	// Clear timer if disabling invulnerability
	if (!bInvulnerable)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(InvulnerabilityTimerHandle);
		}
	}
}

void UHealthComponent::EndInvulnerability()
{
	bInvulnerable = false;
}
