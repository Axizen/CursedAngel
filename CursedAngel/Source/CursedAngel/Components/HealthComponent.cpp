// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/HealthComponent.h"
#include "AI/EnemyDirectorSubsystem.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	MaxHealth = 100.0f; CurrentHealth = MaxHealth; bIsDead = false; bInvulnerable = false; InvulnerabilityDuration = 0.0f; LastDamageDealer = nullptr; LastDamageType = EDamageType::Physical;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay(); CurrentHealth = MaxHealth; bIsDead = false; bInvulnerable = false;
}
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) { Super::TickComponent(DeltaTime, TickType, ThisTickFunction); }

float UHealthComponent::TakeDamage(float Damage, EDamageType Type, AActor* DamageDealer)
{
	if (bIsDead || bInvulnerable) return 0.0f;
	Damage = FMath::Max(0.0f, Damage);
	if (Damage <= 0.0f) return 0.0f;
	LastDamageDealer = DamageDealer; LastDamageType = Type;
	const float PreviousHealth = CurrentHealth;
	CurrentHealth = FMath::Max(0.0f, CurrentHealth - Damage);
	const float ActualDamage = PreviousHealth - CurrentHealth;
	OnDamageTaken.Broadcast(ActualDamage, Type);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	if (const APawn* OwnerPawn = Cast<APawn>(GetOwner()); OwnerPawn && OwnerPawn->IsPlayerControlled())
	{
		if (UWorld* World = GetWorld()) if (UEnemyDirectorSubsystem* Director = World->GetSubsystem<UEnemyDirectorSubsystem>()) Director->RecordPlayerDamageTaken(ActualDamage);
	}
	if (CurrentHealth <= 0.0f) Kill();
	else if (InvulnerabilityDuration > 0.0f)
	{
		bInvulnerable = true;
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(InvulnerabilityTimerHandle);
			World->GetTimerManager().SetTimer(InvulnerabilityTimerHandle, this, &UHealthComponent::EndInvulnerability, InvulnerabilityDuration, false);
		}
	}
	return ActualDamage;
}

float UHealthComponent::Heal(float Amount)
{
	if (bIsDead) return 0.0f;
	Amount = FMath::Max(0.0f, Amount);
	const float OldHealth = CurrentHealth;
	CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + Amount);
	const float ActualHeal = CurrentHealth - OldHealth;
	if (ActualHeal > 0.0f) OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	return ActualHeal;
}

void UHealthComponent::Kill()
{
	if (bIsDead) return;
	bIsDead = true; CurrentHealth = 0.0f;
	if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(InvulnerabilityTimerHandle);
	bInvulnerable = false;
	OnDeath.Broadcast(LastDamageDealer, LastDamageType);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

bool UHealthComponent::IsAlive() const { return !bIsDead && CurrentHealth > 0.0f; }
float UHealthComponent::GetHealthPercent() const { return MaxHealth <= 0.0f ? 0.0f : CurrentHealth / MaxHealth; }
void UHealthComponent::SetInvulnerable(bool bNewInvulnerable)
{
	bInvulnerable = bNewInvulnerable;
	if (!bInvulnerable) if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(InvulnerabilityTimerHandle);
}
void UHealthComponent::EndInvulnerability() { bInvulnerable = false; }
