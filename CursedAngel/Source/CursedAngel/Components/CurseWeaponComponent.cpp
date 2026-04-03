// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/CurseWeaponComponent.h"
#include "Projectiles/CurseProjectile.h"
#include "Characters/CursedAngelCharacter.h"
#include "Characters/Ripley.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"

UCurseWeaponComponent::UCurseWeaponComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default weapons
    FCurseWeaponData CorruptionRailData;
    CorruptionRailData.WeaponType = ECurseWeaponType::CorruptionRail;
    CorruptionRailData.CursePower = 100.0f;
    CorruptionRailData.CooldownTime = 2.0f;
    CorruptionRailData.bEnvironmentalInteraction = true;
    
    FCurseWeaponData FragmentNeedlesData;
    FragmentNeedlesData.WeaponType = ECurseWeaponType::FragmentNeedles;
    FragmentNeedlesData.CursePower = 75.0f;
    FragmentNeedlesData.CooldownTime = 1.0f;
    FragmentNeedlesData.bEnvironmentalInteraction = true;
    
    FCurseWeaponData VoidMinesData;
    VoidMinesData.WeaponType = ECurseWeaponType::VoidMines;
    VoidMinesData.CursePower = 150.0f;
    VoidMinesData.CooldownTime = 3.0f;
    VoidMinesData.bEnvironmentalInteraction = true;
    
    AvailableWeapons.Add(CorruptionRailData);
    AvailableWeapons.Add(FragmentNeedlesData);
    AvailableWeapons.Add(VoidMinesData);
}

void UCurseWeaponComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize cooldown map
    for (const FCurseWeaponData& WeaponData : AvailableWeapons)
    {
        WeaponCooldowns.Add(WeaponData.WeaponType, 0.0f);
    }
    
    // Initialize socket indices for alternating pattern
    CurrentSocketIndices.Add(ECurseWeaponType::CorruptionRail, 0);
    CurrentSocketIndices.Add(ECurseWeaponType::FragmentNeedles, 0);
    CurrentSocketIndices.Add(ECurseWeaponType::VoidMines, 0);
}

bool UCurseWeaponComponent::CanFireWeapon(ECurseWeaponType WeaponType) const
{
    if (const float* CooldownPtr = WeaponCooldowns.Find(WeaponType))
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        return CurrentTime >= *CooldownPtr;
    }
    return false;
}

void UCurseWeaponComponent::FireCurseWeapon()
{
    if (!CanFireWeapon(CurrentWeapon))
    {
        return;
    }
    
    // Check global cooldown
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime < LastFireTime + GlobalCooldown)
    {
        return;
    }
    
    // Find weapon data
    const FCurseWeaponData* WeaponData = nullptr;
    for (const FCurseWeaponData& Data : AvailableWeapons)
    {
        if (Data.WeaponType == CurrentWeapon)
        {
            WeaponData = &Data;
            break;
        }
    }
    
    if (!WeaponData)
    {
        return;
    }
    
    // Set cooldowns
    WeaponCooldowns[CurrentWeapon] = CurrentTime + WeaponData->CooldownTime;
    LastFireTime = CurrentTime;
    
    // Get weapon stats from map
    FCurseWeaponStats Stats;
    if (const FCurseWeaponStats* StatsPtr = WeaponStatsMap.Find(CurrentWeapon))
    {
        Stats = *StatsPtr;
    }
    else
    {
        // Use default stats if not found
        Stats.Damage = WeaponData->CursePower;
        Stats.CooldownTime = WeaponData->CooldownTime;
        Stats.bEnvironmentalInteraction = WeaponData->bEnvironmentalInteraction;
    }
    
    // Get spawn config from stats
    FProjectileSpawnConfig SpawnConfig = Stats.SpawnConfig;
    
    // Determine spawn locations based on spawn pattern
    TArray<FVector> SpawnLocations;
    TArray<FVector> SpawnDirections;
    
    switch (SpawnConfig.SpawnPattern)
    {
        case ESpawnPattern::Single:
            // Spawn from first socket only
            if (SpawnConfig.SpawnSockets.Num() > 0)
            {
                SpawnLocations.Add(GetProjectileSpawnLocation(CurrentWeapon, 0));
                SpawnDirections.Add(GetProjectileSpawnDirection(CurrentWeapon));
            }
            break;
            
        case ESpawnPattern::Alternating:
            // Spawn from next socket in sequence
            if (SpawnConfig.SpawnSockets.Num() > 0)
            {
                int32 SocketIndex = GetNextSocketIndex(CurrentWeapon);
                SpawnLocations.Add(GetProjectileSpawnLocation(CurrentWeapon, SocketIndex));
                SpawnDirections.Add(GetProjectileSpawnDirection(CurrentWeapon));
            }
            break;
            
        case ESpawnPattern::Simultaneous:
            // Spawn from all sockets
            for (int32 i = 0; i < SpawnConfig.SpawnSockets.Num(); ++i)
            {
                SpawnLocations.Add(GetProjectileSpawnLocation(CurrentWeapon, i));
                SpawnDirections.Add(GetProjectileSpawnDirection(CurrentWeapon));
            }
            break;
    }
    
    // Get projectile class from map
    TSubclassOf<ACurseProjectile>* ProjectileClassPtr = ProjectileClasses.Find(CurrentWeapon);
    if (ProjectileClassPtr && *ProjectileClassPtr)
    {
        // Spawn projectiles at each location
        for (int32 i = 0; i < SpawnLocations.Num(); ++i)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = GetOwner();
            SpawnParams.Instigator = Cast<APawn>(GetOwner());
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            
            ACurseProjectile* Projectile = GetWorld()->SpawnActor<ACurseProjectile>(
                *ProjectileClassPtr,
                SpawnLocations[i],
                SpawnDirections[i].Rotation(),
                SpawnParams
            );
            
            if (Projectile)
            {
                // Initialize projectile with stats and owner
                // Note: Initialize expects ARipley* for now, will be refactored to ACursedAngelCharacter* later
                ARipley* OwnerRipley = Cast<ARipley>(GetOwner());
                if (OwnerRipley)
                {
                    Projectile->Initialize(Stats, OwnerRipley);
                }
            }
        }
        
        // Fire the weapon (Blueprint implementation for VFX)
        // Use first spawn location and direction for event
        if (SpawnLocations.Num() > 0)
        {
            OnWeaponFired(CurrentWeapon, SpawnLocations[0], SpawnDirections[0]);
        }
    }
}

void UCurseWeaponComponent::SwitchWeapon(ECurseWeaponType NewWeapon)
{
    if (CurrentWeapon != NewWeapon)
    {
        // Check if weapon exists in available weapons
        bool bWeaponExists = false;
        for (const FCurseWeaponData& WeaponData : AvailableWeapons)
        {
            if (WeaponData.WeaponType == NewWeapon)
            {
                bWeaponExists = true;
                break;
            }
        }
        
        if (bWeaponExists)
        {
            CurrentWeapon = NewWeapon;
            OnWeaponSwitched(NewWeapon);
        }
    }
}

void UCurseWeaponComponent::UpdateCooldowns()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    for (auto& CooldownPair : WeaponCooldowns)
    {
        if (CooldownPair.Value > CurrentTime)
        {
            CooldownPair.Value = FMath::Max(0.0f, CooldownPair.Value - GetWorld()->GetDeltaSeconds());
        }
    }
}

FCurseWeaponStats UCurseWeaponComponent::GetWeaponStats(ECurseWeaponType WeaponType) const
{
    if (const FCurseWeaponStats* StatsPtr = WeaponStatsMap.Find(WeaponType))
    {
        return *StatsPtr;
    }
    
    // Return default stats if not found
    return FCurseWeaponStats();
}

FVector UCurseWeaponComponent::GetProjectileSpawnLocation(ECurseWeaponType WeaponType, int32 SocketIndex)
{
    // Get weapon stats to access spawn config
    const FCurseWeaponStats* StatsPtr = WeaponStatsMap.Find(WeaponType);
    if (!StatsPtr)
    {
        return FVector::ZeroVector;
    }
    
    const FProjectileSpawnConfig& SpawnConfig = StatsPtr->SpawnConfig;
    
    // Validate socket index
    if (!SpawnConfig.SpawnSockets.IsValidIndex(SocketIndex))
    {
        return FVector::ZeroVector;
    }
    
    // Get owner's skeletal mesh
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        return FVector::ZeroVector;
    }
    
    USkeletalMeshComponent* MeshComponent = OwnerCharacter->GetMesh();
    if (!MeshComponent)
    {
        return FVector::ZeroVector;
    }
    
    // Get socket location
    FName SocketName = SpawnConfig.SpawnSockets[SocketIndex];
    if (!MeshComponent->DoesSocketExist(SocketName))
    {
        // Fallback to actor location if socket doesn't exist
        return OwnerCharacter->GetActorLocation() + SpawnConfig.SocketOffset;
    }
    
    FVector SocketLocation = MeshComponent->GetSocketLocation(SocketName);
    
    // Apply socket offset
    return SocketLocation + SpawnConfig.SocketOffset;
}

FVector UCurseWeaponComponent::GetProjectileSpawnDirection(ECurseWeaponType WeaponType)
{
    // Get weapon stats to access spawn config
    const FCurseWeaponStats* StatsPtr = WeaponStatsMap.Find(WeaponType);
    if (!StatsPtr)
    {
        return FVector::ForwardVector;
    }
    
    const FProjectileSpawnConfig& SpawnConfig = StatsPtr->SpawnConfig;
    
    // Get owner character
    ACursedAngelCharacter* OwnerCharacter = Cast<ACursedAngelCharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        return FVector::ForwardVector;
    }
    
    if (SpawnConfig.bUseCameraDirection)
    {
        // Use camera forward direction
        UCameraComponent* Camera = OwnerCharacter->FindComponentByClass<UCameraComponent>();
        if (Camera)
        {
            return Camera->GetForwardVector();
        }
    }
    
    // Fallback to actor forward direction
    return OwnerCharacter->GetActorForwardVector();
}

int32 UCurseWeaponComponent::GetNextSocketIndex(ECurseWeaponType WeaponType)
{
    // Get weapon stats to access spawn config
    const FCurseWeaponStats* StatsPtr = WeaponStatsMap.Find(WeaponType);
    if (!StatsPtr)
    {
        return 0;
    }
    
    const FProjectileSpawnConfig& SpawnConfig = StatsPtr->SpawnConfig;
    int32 SocketCount = SpawnConfig.SpawnSockets.Num();
    
    if (SocketCount == 0)
    {
        return 0;
    }
    
    // Get current index
    int32* CurrentIndexPtr = CurrentSocketIndices.Find(WeaponType);
    if (!CurrentIndexPtr)
    {
        // Initialize if not found
        CurrentSocketIndices.Add(WeaponType, 0);
        return 0;
    }
    
    int32 CurrentIndex = *CurrentIndexPtr;
    
    // Increment and wrap around
    int32 NextIndex = (CurrentIndex + 1) % SocketCount;
    CurrentSocketIndices[WeaponType] = NextIndex;
    
    return CurrentIndex; // Return current before incrementing for this shot
}

void UCurseWeaponComponent::LoadWeaponStatsFromDataAsset(UDataAsset* WeaponConfigAsset)
{
    // This function is a placeholder for future data asset integration
    // For now, designers can manually configure WeaponStatsMap in the editor
    // When data assets are created, this will load from them
}
