// Copyright Epic Games, Inc. All Rights Reserved.

#include "Environment/CombatDebris.h"
#include "AI/EnemyBase.h"
#include "Components/HealthComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Environment/DestructibleBase.h"
#include "Environment/EnvironmentCombatSubsystem.h"
#include "Engine/World.h"

ACombatDebris::ACombatDebris()
{
	PrimaryActorTick.bCanEverTick = true;
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DebrisMesh"));
	RootComponent = MeshComponent;
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	MeshComponent->SetNotifyRigidBodyCollision(true);
	MeshComponent->OnComponentHit.AddDynamic(this, &ACombatDebris::HandleComponentHit);
}

void ACombatDebris::BeginPlay()
{
	Super::BeginPlay();
	SpawnTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	if (UWorld* World = GetWorld())
	{
		if (UEnvironmentCombatSubsystem* Environment = World->GetSubsystem<UEnvironmentCombatSubsystem>()) Environment->RegisterDebris(this);
	}
}

void ACombatDebris::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Age += DeltaTime;
	TimeSinceLastHit += DeltaTime;
	if (DebrisLifetime > 0.0f && Age >= DebrisLifetime && !bIsMagnetized) { Destroy(); return; }
	if (bIsMagnetized && IsValid(MagnetHolder))
	{
		const FVector HoldLocation = MagnetHolder->GetActorLocation() + MagnetHolder->GetActorForwardVector() * 110.0f + FVector(0.0f, 0.0f, 70.0f);
		SetActorLocation(FMath::VInterpTo(GetActorLocation(), HoldLocation, DeltaTime, 12.0f), false, nullptr, ETeleportType::TeleportPhysics);
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), MagnetHolder->GetActorRotation(), DeltaTime, 10.0f));
	}
	else if (!MagnetHolder.IsValid() && !MeshComponent->IsSimulatingPhysics())
	{
		MeshComponent->SetSimulatePhysics(true);
	}
}

void ACombatDebris::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld()) if (UEnvironmentCombatSubsystem* Environment = World->GetSubsystem<UEnvironmentCombatSubsystem>()) Environment->UnregisterDebris(this);
	Super::EndPlay(EndPlayReason);
}

bool ACombatDebris::BeginMagnetize(AActor* Holder)
{
	if (!bMagnetizable || !IsValid(Holder) || bIsMagnetized) return false;
	MagnetHolder = Holder;
	bIsMagnetized = true;
	if (MeshComponent) { MeshComponent->SetSimulatePhysics(false); MeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector); MeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector); }
	SetOwner(Holder);
	return true;
}

void ACombatDebris::LaunchDebris(FVector Direction, float Speed, float StyleMultiplier)
{
	if (!MeshComponent || Speed <= 0.0f) return;
	ReleaseMagnet();
	ImpactStyleMultiplier = FMath::Max(0.0f, StyleMultiplier);
	ActorsAlreadyHit.Empty();
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetPhysicsLinearVelocity(Direction.GetSafeNormal() * Speed, false);
	MeshComponent->WakeAllRigidBodies();
}

void ACombatDebris::ReleaseMagnet()
{
	bIsMagnetized = false;
	MagnetHolder = nullptr;
	SetOwner(nullptr);
	if (MeshComponent) MeshComponent->SetSimulatePhysics(true);
}

void ACombatDebris::HandleComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!IsValid(OtherActor) || OtherActor == this || bIsMagnetized || ActorsAlreadyHit.Contains(OtherActor) || TimeSinceLastHit < 0.08f) return;
	const FVector Velocity = MeshComponent ? MeshComponent->GetPhysicsLinearVelocity() : FVector::ZeroVector;
	const float ImpactSpeed = FMath::Max(Velocity.Size(), NormalImpulse.Size() / FMath::Max(1.0f, MeshComponent ? MeshComponent->GetMass() : 1.0f));
	if (ImpactSpeed < 250.0f) return;
	TimeSinceLastHit = 0.0f;
	if (AEnemyBase* Enemy = Cast<AEnemyBase>(OtherActor))
	{
		if (UHealthComponent* Health = Enemy->HealthComponent)
		{
			const float Damage = DamagePer1000Speed * (ImpactSpeed / 1000.0f) * FMath::Max(0.0f, ImpactStyleMultiplier);
			if (Damage > 0.0f) { Health->TakeDamage(Damage, EDamageType::Physical, this); ActorsAlreadyHit.Add(OtherActor); }
		}
	}
	else if (ADestructibleBase* Destructible = Cast<ADestructibleBase>(OtherActor))
	{
		Destructible->ApplyStructuralImpact(ImpactSpeed, this, ImpactStyleMultiplier);
		ActorsAlreadyHit.Add(OtherActor);
	}
}
