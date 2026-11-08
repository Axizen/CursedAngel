// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CombatDebris.generated.h"

class UStaticMeshComponent;
class ADestructibleBase;

/** Physical debris retained for arena interaction, magnetic pickup, and improvised projectile combat. */
UCLASS(BlueprintType, Blueprintable)
class CURSEDANGEL_API ACombatDebris : public AActor
{
	GENERATED_BODY()
public:
	ACombatDebris();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debris|Components") TObjectPtr<UStaticMeshComponent> MeshComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debris|Physics", meta=(ClampMin="0.0", ToolTip="Seconds before unattended debris is removed. Zero means it persists until the environment debris budget removes it.")) float DebrisLifetime = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debris|Combat", meta=(ClampMin="0.0", ToolTip="Damage per 1000 cm/s of impact speed when debris hits an enemy.")) float DamagePer1000Speed = 25.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debris|Combat", meta=(ClampMin="0.0")) float ImpactStyleMultiplier = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debris|Combat", meta=(ClampMin="0.0", ToolTip="Radius for manager-mediated domino knockback after this debris hits an enemy.")) float DominoKnockbackRadius = 500.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debris|Combat", meta=(ClampMin="0.0")) float DominoKnockbackImpulse = 800.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debris|Combat", meta=(ClampMin="0.0")) float DominoUpwardImpulse = 300.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debris|Magnet") bool bMagnetizable = true;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debris|Magnet") bool bIsMagnetized = false;
	UFUNCTION(BlueprintCallable, Category="Debris|Magnet") bool BeginMagnetize(AActor* Holder);
	UFUNCTION(BlueprintCallable, Category="Debris|Magnet") void LaunchDebris(FVector Direction, float Speed, float StyleMultiplier = 1.0f);
	UFUNCTION(BlueprintCallable, Category="Debris|Magnet") void ReleaseMagnet();
	UFUNCTION(BlueprintPure, Category="Debris|Magnet") AActor* GetMagnetHolder() const { return MagnetHolder.Get(); }
	UFUNCTION(BlueprintPure, Category="Debris|Physics") float GetSpawnTime() const { return SpawnTime; }
protected:
	UFUNCTION() void HandleComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);
	UPROPERTY(Transient) TObjectPtr<AActor> MagnetHolder = nullptr;
	UPROPERTY(Transient) TSet<TObjectPtr<AActor>> ActorsAlreadyHit;
	float SpawnTime = 0.0f;
	float Age = 0.0f;
	float TimeSinceLastHit = 0.0f;
};
