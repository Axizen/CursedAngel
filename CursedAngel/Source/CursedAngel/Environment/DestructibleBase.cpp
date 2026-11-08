// Copyright Epic Games, Inc. All Rights Reserved.

#include "Environment/DestructibleBase.h"
#include "AI/EnemyDirectorSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Environment/CombatDebris.h"
#include "Environment/EnvironmentCombatSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "GeometryCollection/GeometryCollectionActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Field/FieldSystemComponent.h"
#include "Field/FieldSystemObjects.h"

ADestructibleBase::ADestructibleBase()
{
	PrimaryActorTick.bCanEverTick = true;
	IntactMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IntactMesh"));
	RootComponent = IntactMeshComponent;
	IntactMeshComponent->SetSimulatePhysics(false);
	IntactMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	IntactMeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
}

void ADestructibleBase::BeginPlay()
{
	Super::BeginPlay();
	Health = MaxHealth;
	bHasBeenDestroyed = false;
	DamageStage = EStructuralDamageStage::Intact;
}

void ADestructibleBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TArray<AStaticMeshActor*> Expired;
	for (auto& Pair : ActiveChunks) { Pair.Value -= DeltaTime; if (Pair.Value <= 0.0f) Expired.Add(Pair.Key); }
	for (AStaticMeshActor* Chunk : Expired) { ActiveChunks.Remove(Chunk); ReturnChunkToPool(Chunk); }
}

void ADestructibleBase::TakeDamageCustom(float DamageAmount, AActor* DamageCauser)
{
	if (bHasBeenDestroyed || Health <= 0.0f || DamageAmount <= 0.0f) return;
	LastDamageCauser = DamageCauser;
	Health = FMath::Max(0.0f, Health - DamageAmount);
	UpdateDamageStage();
	if (Health <= 0.0f) DestroyObject();
}

void ADestructibleBase::UpdateDamageStage()
{
	const float HealthFraction = MaxHealth > 0.0f ? Health / MaxHealth : 0.0f;
	EStructuralDamageStage NewStage = EStructuralDamageStage::Intact;
	if (Health <= 0.0f) NewStage = EStructuralDamageStage::Destroyed;
	else if (HealthFraction <= FMath::Clamp(CriticalHealthFraction, 0.0f, 1.0f)) NewStage = EStructuralDamageStage::Critical;
	else if (HealthFraction <= FMath::Clamp(DamagedHealthFraction, CriticalHealthFraction, 1.0f)) NewStage = EStructuralDamageStage::Damaged;
	if (NewStage != DamageStage)
	{
		const EStructuralDamageStage PreviousStage = DamageStage;
		DamageStage = NewStage;
		OnDamageStageChanged.Broadcast(NewStage, PreviousStage);
	}
}

void ADestructibleBase::ApplyStructuralImpact(float ImpactSpeed, AActor* ImpactCauser, float StyleMultiplier)
{
	if (bHasBeenDestroyed || ImpactSpeed < MinimumImpactSpeed) return;
	const float EffectiveStyle = FMath::Clamp(StyleMultiplier, 0.0f, MaximumStyleImpactMultiplier);
	const float Damage = ((ImpactSpeed - MinimumImpactSpeed) / 1000.0f) * ImpactDamagePer1000Speed * FMath::Max(0.1f, EffectiveStyle);
	if (Damage <= 0.0f) return;
	OnStructuralImpact.Broadcast(ImpactCauser, ImpactSpeed, Damage);
	TakeDamageCustom(Damage, ImpactCauser);
}

void ADestructibleBase::DestroyObject()
{
	if (bHasBeenDestroyed || Health > 0.0f) return;
	bHasBeenDestroyed = true;
	UpdateDamageStage();
	if (IntactMeshComponent) { IntactMeshComponent->SetVisibility(false); IntactMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision); }
	if (bUseChaosDestruction && ChaosGeometryCollection) SpawnChunksChaos(); else SpawnChunksPreFractured();
	if (DestructionVFX) UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), DestructionVFX, GetActorLocation(), GetActorRotation());
	if (DestructionSFX) UGameplayStatics::PlaySoundAtLocation(GetWorld(), DestructionSFX, GetActorLocation());
	if (UWorld* World = GetWorld())
	{
		if (UEnemyDirectorSubsystem* Director = World->GetSubsystem<UEnemyDirectorSubsystem>()) Director->RecordDestruction(LastDamageCauser, DestructionProfileWeight);
		if (bPermanentArenaChange)
		{
			if (UEnvironmentCombatSubsystem* Environment = World->GetSubsystem<UEnvironmentCombatSubsystem>()) Environment->RecordStructuralPieceBroken(ArenaRegionId, this, 0.0f);
		}
	}
	OnDestructibleBroken.Broadcast(LastDamageCauser, DestructionProfileWeight);
	SpawnDataCurrency();
	SetLifeSpan(0.5f);
}

void ADestructibleBase::SpawnChunksPreFractured()
{
	if (ChunkMeshes.Num() == 0 || !GetWorld()) return;
	const FVector Center = GetActorLocation();
	for (int32 Index = 0; Index < ChunkCount; ++Index)
	{
		UStaticMesh* Mesh = ChunkMeshes[FMath::RandRange(0, ChunkMeshes.Num() - 1)];
		if (!Mesh) continue;
		const FVector Location = Center + FVector(FMath::FRandRange(-50.f,50.f), FMath::FRandRange(-50.f,50.f), FMath::FRandRange(-50.f,50.f));
		const FRotator Rotation = GetActorRotation() + FRotator(FMath::FRandRange(0.f,360.f), FMath::FRandRange(0.f,360.f), FMath::FRandRange(0.f,360.f));
		if (DebrisActorClass)
		{
			FActorSpawnParameters Params; Params.Owner = this; Params.Instigator = GetInstigator();
			AActor* Spawned = GetWorld()->SpawnActor<AActor>(DebrisActorClass, Location, Rotation, Params);
			if (ACombatDebris* Debris = Cast<ACombatDebris>(Spawned))
			{
				if (Debris->MeshComponent) Debris->MeshComponent->SetStaticMesh(Mesh);
				Debris->DebrisLifetime = ChunkLifetime;
				const FVector Direction = (Location - Center).GetSafeNormal() + FVector(0,0,DebrisUpwardBias);
				if (Debris->MeshComponent) Debris->MeshComponent->AddImpulse(Direction.GetSafeNormal() * ChunkImpulseStrength, NAME_None, true);
			}
			else if (Spawned) Spawned->Destroy();
			continue;
		}
		AStaticMeshActor* Chunk = GetPooledChunk();
		if (!Chunk) continue;
		UStaticMeshComponent* Component = Chunk->GetStaticMeshComponent();
		if (!Component) continue;
		Component->SetStaticMesh(Mesh); Component->SetSimulatePhysics(true); Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); Component->SetCollisionResponseToAllChannels(ECR_Block);
		Chunk->SetActorLocation(Location); Chunk->SetActorRotation(Rotation);
		const FVector Direction = (Location - Center).GetSafeNormal() + FVector(0,0,DebrisUpwardBias);
		Component->AddImpulse(Direction.GetSafeNormal() * ChunkImpulseStrength, NAME_None, true);
		Chunk->SetActorHiddenInGame(false);
		if (ChunkLifetime > 0.0f) ActiveChunks.Add(Chunk, ChunkLifetime);
	}
}

void ADestructibleBase::SpawnChunksChaos()
{
	if (!GetWorld() || !ChaosGeometryCollection) { SpawnChunksPreFractured(); return; }
	FActorSpawnParameters Params; Params.Owner = this; Params.Instigator = GetInstigator(); Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AGeometryCollectionActor* Actor = GetWorld()->SpawnActor<AGeometryCollectionActor>(AGeometryCollectionActor::StaticClass(), GetActorLocation(), GetActorRotation(), Params);
	if (!Actor) return;
	UGeometryCollectionComponent* Collection = Actor->GetGeometryCollectionComponent();
	if (!Collection) { Actor->Destroy(); return; }
	Collection->SetRestCollection(ChaosGeometryCollection); Collection->SetSimulatePhysics(true); Collection->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); Collection->SetCollisionResponseToAllChannels(ECR_Block); Collection->SetDamageThreshold({DamageThreshold}); Collection->SetEnableDamageFromCollision(bEnableClustering);
	UFieldSystemComponent* Field = NewObject<UFieldSystemComponent>(Actor);
	if (Field) { Field->RegisterComponent(); Field->AttachToComponent(Actor->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale); URadialVector* Radial = NewObject<URadialVector>(Field); if (Radial) { Radial->Magnitude = FieldForceStrength; Radial->Position = GetActorLocation(); Field->ApplyPhysicsField(true, EFieldPhysicsType::Field_LinearForce, nullptr, Radial); } }
	// Collection intentionally persists for the rest of the encounter after a permanent structural collapse.
}

void ADestructibleBase::SpawnDataCurrency()
{
	if (DataDropAmount <= 0 || !GetWorld()) return;
	if (bAutoCollectData)
	{
		TArray<AActor*> Pawns; UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), Pawns);
		for (AActor* Actor : Pawns) { APawn* Pawn = Cast<APawn>(Actor); if (Pawn && Pawn->IsPlayerControlled() && FVector::Dist(GetActorLocation(), Pawn->GetActorLocation()) <= AutoCollectRadius) { UE_LOG(LogTemp, Log, TEXT("DestructibleBase: %d data fragments available for nearby player (currency integration pending)."), DataDropAmount); return; } }
	}
	else if (DataPickupClass) { FActorSpawnParameters Params; Params.Owner = this; Params.Instigator = GetInstigator(); GetWorld()->SpawnActor<AActor>(DataPickupClass, GetActorLocation() + FVector(0,0,50), FRotator::ZeroRotator, Params); }
}

AStaticMeshActor* ADestructibleBase::GetPooledChunk()
{
	for (AStaticMeshActor* Chunk : ChunkPool) if (Chunk && Chunk->IsHidden()) return Chunk;
	if (ChunkPool.Num() >= MaxPoolSize || !GetWorld()) return nullptr;
	AStaticMeshActor* NewChunk = GetWorld()->SpawnActor<AStaticMeshActor>();
	if (NewChunk) { NewChunk->SetActorHiddenInGame(true); ChunkPool.Add(NewChunk); }
	return NewChunk;
}
void ADestructibleBase::ReturnChunkToPool(AStaticMeshActor* Chunk)
{
	if (!Chunk) return;
	Chunk->SetActorHiddenInGame(true);
	if (UStaticMeshComponent* Component = Chunk->GetStaticMeshComponent()) { Component->SetSimulatePhysics(false); Component->SetCollisionEnabled(ECollisionEnabled::NoCollision); Component->SetPhysicsLinearVelocity(FVector::ZeroVector); Component->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector); }
	Chunk->SetActorLocation(FVector(0,0,-10000));
}
