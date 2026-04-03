// Fill out your copyright notice in the Description page of Project Settings.


#include "Environment/DestructibleBase.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
//#include "Components/DataManagerComponent.h"
//#include "Characters/Ripley.h"
//#include "Components/CurrencyComponent.h" // PLACEHOLDER: Will be created in Phase 2 Step 7-8
#include "GeometryCollection/GeometryCollectionActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Field/FieldSystemComponent.h"
#include "Field/FieldSystemObjects.h"


#include "TimerManager.h"

ADestructibleBase::ADestructibleBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create intact mesh component
	IntactMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IntactMesh"));
	RootComponent = IntactMeshComponent;

	// Enable physics simulation
	IntactMeshComponent->SetSimulatePhysics(false);
	IntactMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	IntactMeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
}

void ADestructibleBase::BeginPlay()
{
	Super::BeginPlay();

	// Initialize health to max
	Health = MaxHealth;
}

void ADestructibleBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update active chunks lifetime
	TArray<AStaticMeshActor*> ChunksToReturn;

	for (auto& ChunkPair : ActiveChunks)
	{
		AStaticMeshActor* Chunk = ChunkPair.Key;
		float& TimeRemaining = ChunkPair.Value;

		TimeRemaining -= DeltaTime;

		if (TimeRemaining <= 0.0f)
		{
			ChunksToReturn.Add(Chunk);
		}
	}

	// Return expired chunks to pool
	for (AStaticMeshActor* Chunk : ChunksToReturn)
	{
		ActiveChunks.Remove(Chunk);
		ReturnChunkToPool(Chunk);
	}
}

void ADestructibleBase::TakeDamageCustom(float DamageAmount, AActor* DamageCauser)
{
	// Don't take damage if already destroyed
	if (Health <= 0.0f)
	{
		return;
	}

	// Apply damage
	Health -= DamageAmount;

	// Check if destroyed
	if (Health <= 0.0f)
	{
		Health = 0.0f;
		DestroyObject();
	}
}

void ADestructibleBase::DestroyObject()
{
	// Don't destroy twice
	if (Health > 0.0f)
	{
		return;
	}

	// Hide intact mesh
	if (IntactMeshComponent)
	{
		IntactMeshComponent->SetVisibility(false);
		IntactMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Spawn chunks based on destruction type
	if (bUseChaosDestruction)
	{
		SpawnChunksChaos();
	}
	else
	{
		SpawnChunksPreFractured();
	}

	// Spawn VFX
	if (DestructionVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			DestructionVFX,
			GetActorLocation(),
			GetActorRotation()
		);
	}

	// Play SFX
	if (DestructionSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			DestructionSFX,
			GetActorLocation()
		);
	}

	// Spawn data currency
	SpawnDataCurrency();

	// Destroy actor after a delay (to allow chunks to finish spawning)
	SetLifeSpan(0.5f);
}

void ADestructibleBase::SpawnChunksPreFractured()
{
	// Validate chunk meshes
	if (ChunkMeshes.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("DestructibleBase: No chunk meshes assigned for %s"), *GetName());
		return;
	}

	FVector CenterLocation = GetActorLocation();
	FRotator BaseRotation = GetActorRotation();

	// Spawn chunks
	for (int32 i = 0; i < ChunkCount; i++)
	{
		// Get pooled chunk actor
		AStaticMeshActor* ChunkActor = GetPooledChunk();
		if (!ChunkActor)
		{
			continue;
		}

		// Select random chunk mesh
		int32 MeshIndex = FMath::RandRange(0, ChunkMeshes.Num() - 1);
		UStaticMesh* ChunkMesh = ChunkMeshes[MeshIndex];

		if (!ChunkMesh)
		{
			continue;
		}

		// Set chunk mesh
		UStaticMeshComponent* ChunkMeshComponent = ChunkActor->GetStaticMeshComponent();
		if (ChunkMeshComponent)
		{
			ChunkMeshComponent->SetStaticMesh(ChunkMesh);
			ChunkMeshComponent->SetSimulatePhysics(true);
			ChunkMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			ChunkMeshComponent->SetCollisionResponseToAllChannels(ECR_Block);

			// Random offset from center
			FVector RandomOffset = FVector(
				FMath::RandRange(-50.0f, 50.0f),
				FMath::RandRange(-50.0f, 50.0f),
				FMath::RandRange(-50.0f, 50.0f)
			);

			// Set chunk location and rotation
			ChunkActor->SetActorLocation(CenterLocation + RandomOffset);

			FRotator RandomRotation = FRotator(
				FMath::RandRange(0.0f, 360.0f),
				FMath::RandRange(0.0f, 360.0f),
				FMath::RandRange(0.0f, 360.0f)
			);
			ChunkActor->SetActorRotation(BaseRotation + RandomRotation);

			// Apply radial impulse from center
			FVector ImpulseDirection = (ChunkActor->GetActorLocation() - CenterLocation).GetSafeNormal();
			FVector Impulse = ImpulseDirection * ChunkImpulseStrength;

			// Add random variation
			Impulse.X += FMath::RandRange(-100.0f, 100.0f);
			Impulse.Y += FMath::RandRange(-100.0f, 100.0f);
			Impulse.Z += FMath::RandRange(0.0f, 200.0f); // Bias upward

			ChunkMeshComponent->AddImpulse(Impulse, NAME_None, true);

			// Make chunk visible
			ChunkActor->SetActorHiddenInGame(false);

			// Track active chunk with lifetime
			ActiveChunks.Add(ChunkActor, ChunkLifetime);
		}
	}
}

void ADestructibleBase::SpawnChunksChaos()
{
	// Fall back to pre-fractured chunks if Chaos not configured
	if (!ChaosGeometryCollection)
	{
		UE_LOG(LogTemp, Warning, TEXT("DestructibleBase: bUseChaosDestruction is true but ChaosGeometryCollection is not set for %s. Falling back to pre-fractured chunks."), *GetName());
		SpawnChunksPreFractured();
		return;
	}

	// Spawn GeometryCollectionActor at location
	FVector SpawnLocation = GetActorLocation();
	FRotator SpawnRotation = GetActorRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AGeometryCollectionActor* GeometryCollectionActor = GetWorld()->SpawnActor<AGeometryCollectionActor>(
		AGeometryCollectionActor::StaticClass(),
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (!GeometryCollectionActor)
	{
		UE_LOG(LogTemp, Error, TEXT("DestructibleBase: Failed to spawn GeometryCollectionActor for %s"), *GetName());
		return;
	}

	// Get the geometry collection component
	UGeometryCollectionComponent* GeometryCollectionComponent = GeometryCollectionActor->GetGeometryCollectionComponent();
	if (!GeometryCollectionComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("DestructibleBase: GeometryCollectionActor has no GeometryCollectionComponent for %s"), *GetName());
		GeometryCollectionActor->Destroy();
		return;
	}

	// Set the geometry collection asset
	GeometryCollectionComponent->SetRestCollection(ChaosGeometryCollection);

	// Configure Chaos physics settings
	GeometryCollectionComponent->SetSimulatePhysics(true);
	GeometryCollectionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GeometryCollectionComponent->SetCollisionResponseToAllChannels(ECR_Block);

	// Set damage threshold (controls when clusters break)
	GeometryCollectionComponent->SetDamageThreshold({DamageThreshold});

	// Enable clustering and damage from collision
	GeometryCollectionComponent->SetEnableDamageFromCollision(bEnableClustering);
	// Note: ClusterConnectionType is configured in the GeometryCollection asset itself

	// Create and attach Field System Component for radial force
	UFieldSystemComponent* FieldSystemComponent = NewObject<UFieldSystemComponent>(GeometryCollectionActor, UFieldSystemComponent::StaticClass());
	if (FieldSystemComponent)
	{
		FieldSystemComponent->RegisterComponent();
		FieldSystemComponent->AttachToComponent(GeometryCollectionActor->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);

		// Create radial force field for explosion impulse
		URadialVector* RadialVector = NewObject<URadialVector>(FieldSystemComponent, URadialVector::StaticClass());
		if (RadialVector)
		{
			// Configure radial force at destruction location
			RadialVector->Magnitude = FieldForceStrength;
			RadialVector->Position = SpawnLocation;
			// Note: Radius is controlled by the field system's sphere radius parameter

			// Apply force to geometry collection
			FieldSystemComponent->ApplyPhysicsField(
				true,
				EFieldPhysicsType::Field_LinearForce,
				nullptr,
				RadialVector
			);

			UE_LOG(LogTemp, Log, TEXT("DestructibleBase: Applied radial force field to GeometryCollection for %s"), *GetName());
		}
	}

	// Set lifespan for cleanup
	GeometryCollectionActor->SetLifeSpan(ChunkLifetime);

	UE_LOG(LogTemp, Log, TEXT("DestructibleBase: Spawned Chaos GeometryCollectionActor for %s"), *GetName());
}

void ADestructibleBase::SpawnDataCurrency()
{
	if (DataDropAmount <= 0)
	{
		return;
	}

	if (bAutoCollectData)
	{
		// Find nearby player and add data directly
		// NOTE: Once CurrencyComponent is implemented (Phase 2 Step 7-8), uncomment the final logic
		// and replace APawn with the actual player character class (ARipley or ACursedAngelCharacter)
		
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);

		for (AActor* Actor : FoundActors)
		{
			APawn* PlayerPawn = Cast<APawn>(Actor);
			if (PlayerPawn && PlayerPawn->IsPlayerControlled())
			{
				float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());

				if (Distance <= AutoCollectRadius)
				{
					// PLACEHOLDER: Once CurrencyComponent is implemented, use:
					// UCurrencyComponent* CurrencyComp = PlayerPawn->FindComponentByClass<UCurrencyComponent>();
					// if (CurrencyComp)
					// {
					//     CurrencyComp->AddDataFragments(DataDropAmount);
					//     UE_LOG(LogTemp, Log, TEXT("DestructibleBase: Auto-collected %d Data Fragments for player within radius"), DataDropAmount);
					// }
					
					UE_LOG(LogTemp, Warning, TEXT("DestructibleBase: Auto-collect enabled but CurrencyComponent not yet implemented. %d Data Fragments would be collected."), DataDropAmount);
					break;
				}
			}
		}
	}
	else if (DataPickupClass)
	{
		// Spawn physical data pickup actor
		FVector SpawnLocation = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
		FRotator SpawnRotation = FRotator::ZeroRotator;

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();

		AActor* DataPickup = GetWorld()->SpawnActor<AActor>(DataPickupClass, SpawnLocation, SpawnRotation, SpawnParams);

		if (DataPickup)
		{
			UE_LOG(LogTemp, Log, TEXT("DestructibleBase: Spawned data pickup actor at location with %d Data Fragments"), DataDropAmount);
			
			// FUTURE: Set DataDropAmount on the spawned pickup actor
			// Example: If DataPickup has a SetDataAmount() function or DataAmount property
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("DestructibleBase: Failed to spawn DataPickupClass"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DestructibleBase: DataDropAmount > 0 but bAutoCollectData is false and DataPickupClass is not set"));
	}
}

AStaticMeshActor* ADestructibleBase::GetPooledChunk()
{
	// Try to get chunk from pool
	for (AStaticMeshActor* Chunk : ChunkPool)
	{
		if (Chunk && Chunk->IsHidden())
		{
			return Chunk;
		}
	}

	// Pool is empty or at capacity, create new chunk if under max pool size
	if (ChunkPool.Num() < MaxPoolSize)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;

		AStaticMeshActor* NewChunk = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (NewChunk)
		{
			ChunkPool.Add(NewChunk);
			NewChunk->SetActorHiddenInGame(true);
			return NewChunk;
		}
	}

	// Pool is at capacity and no chunks available, return nullptr
	UE_LOG(LogTemp, Warning, TEXT("DestructibleBase: Chunk pool exhausted for %s"), *GetName());
	return nullptr;
}

void ADestructibleBase::ReturnChunkToPool(AStaticMeshActor* Chunk)
{
	if (!Chunk)
	{
		return;
	}

	// Hide chunk and disable physics
	Chunk->SetActorHiddenInGame(true);

	UStaticMeshComponent* ChunkMeshComponent = Chunk->GetStaticMeshComponent();
	if (ChunkMeshComponent)
	{
		ChunkMeshComponent->SetSimulatePhysics(false);
		ChunkMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ChunkMeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
		ChunkMeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}

	// Move chunk to origin (out of the way)
	Chunk->SetActorLocation(FVector(0.0f, 0.0f, -10000.0f));
}