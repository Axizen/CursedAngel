// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatTypes.h"
#include "CurseWeaponComponent.generated.h"

class ACurseProjectile;
class ARipley;

UENUM(BlueprintType)
enum class EWeaponSwitchMode : uint8
{
    Instant UMETA(DisplayName = "Instant"),
    Wheel UMETA(DisplayName = "Wheel")
};

UENUM(BlueprintType)
enum class EReactionType : uint8
{
    PlatformExtension,
    HazardActivation,
    BarrierCreation
};

USTRUCT(BlueprintType)
struct FCurseWeaponData
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECurseWeaponType WeaponType = ECurseWeaponType::CorruptionRail;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CursePower = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CooldownTime = 2.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnvironmentalInteraction = true;
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CURSEDANGEL_API UCurseWeaponComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCurseWeaponComponent();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curse Weapons")
    TArray<FCurseWeaponData> AvailableWeapons;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curse Weapons")
    ECurseWeaponType CurrentWeapon = ECurseWeaponType::CorruptionRail;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curse Weapons")
    float GlobalCooldown = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Switching")
    EWeaponSwitchMode SwitchMode = EWeaponSwitchMode::Instant;
    
    UFUNCTION(BlueprintCallable, Category = "Curse Weapons")
    bool CanFireWeapon(ECurseWeaponType WeaponType) const;
    
    UFUNCTION(BlueprintCallable, Category = "Curse Weapons")
    void FireCurseWeapon();
    
    UFUNCTION(BlueprintCallable, Category = "Curse Weapons")
    void SwitchWeapon(ECurseWeaponType NewWeapon);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Curse Weapons")
    void OnWeaponFired(ECurseWeaponType WeaponType, FVector StartLocation, FVector Direction);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Curse Weapons")
    void OnWeaponSwitched(ECurseWeaponType NewWeapon);
    
    UFUNCTION(BlueprintCallable, Category = "Curse Weapons")
    FCurseWeaponStats GetWeaponStats(ECurseWeaponType WeaponType) const;
    
    UFUNCTION(BlueprintCallable, Category = "Curse Weapons")
    void LoadWeaponStatsFromDataAsset(class UDataAsset* WeaponConfigAsset);
    
    /** Get spawn location from socket for the given weapon and socket index */
    UFUNCTION(BlueprintCallable, Category = "Curse Weapons")
    FVector GetProjectileSpawnLocation(ECurseWeaponType WeaponType, int32 SocketIndex);
    
    /** Get spawn direction for the given weapon (camera-directed or socket-directed) */
    UFUNCTION(BlueprintCallable, Category = "Curse Weapons")
    FVector GetProjectileSpawnDirection(ECurseWeaponType WeaponType);
    
    /** Get the next socket index for alternating spawn pattern */
    UFUNCTION(BlueprintCallable, Category = "Curse Weapons")
    int32 GetNextSocketIndex(ECurseWeaponType WeaponType);

public:
    /** Map of projectile classes for each weapon type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curse Weapons")
    TMap<ECurseWeaponType, TSubclassOf<ACurseProjectile>> ProjectileClasses;
    
    /** Map of weapon stats for each weapon type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curse Weapons")
    TMap<ECurseWeaponType, FCurseWeaponStats> WeaponStatsMap;

private:
    TMap<ECurseWeaponType, float> WeaponCooldowns;
    float LastFireTime = 0.0f;
    
    /** Track current socket index for alternating spawn patterns */
    TMap<ECurseWeaponType, int32> CurrentSocketIndices;
    
    void UpdateCooldowns();
};
