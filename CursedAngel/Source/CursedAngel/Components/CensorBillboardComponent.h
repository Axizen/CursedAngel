// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CensorBillboardComponent.generated.h"

class UStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;

/**
 * UCensorBillboardComponent
 *
 * Attaches camera-facing billboard planes to bone sockets on the owner's SkeletalMeshComponent.
 * Used to display glitch/censor VFX over sensitive areas in Cursed Angel form.
 * Each tick, the planes are rotated to face the player camera.
 */
UCLASS(ClassGroup=(CursedAngel), meta=(BlueprintSpawnableComponent))
class CURSEDANGEL_API UCensorBillboardComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCensorBillboardComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ========== Configuration ==========

	/** Flat plane mesh used for each censor billboard */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Censor Billboard")
	UStaticMesh* PlaneMesh = nullptr;

	/** Glitch material applied to both billboard planes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Censor Billboard")
	UMaterialInterface* GlitchMaterial = nullptr;

	/** Socket name on the skeleton for the chest censor plane */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Censor Billboard")
	FName ChestSocketName = TEXT("socket_censor_chest");

	/** Socket name on the skeleton for the groin censor plane */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Censor Billboard")
	FName GroinSocketName = TEXT("socket_censor_groin");

	/** World scale for the chest billboard plane */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Censor Billboard")
	FVector ChestScale = FVector(1.0f, 1.0f, 1.0f);

	/** World scale for the groin billboard plane */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Censor Billboard")
	FVector GroinScale = FVector(1.0f, 1.0f, 1.0f);

	// ========== State ==========

	/** Whether the censor planes are currently active and visible */
	UPROPERTY(BlueprintReadOnly, Category = "Censor Billboard")
	bool bCensorActive = false;

	// ========== Blueprint Interface ==========

	/** Activate the censor billboards (show planes, start camera-facing updates) */
	UFUNCTION(BlueprintCallable, Category = "Censor Billboard")
	void ActivateCensor();

	/** Deactivate the censor billboards (hide planes) */
	UFUNCTION(BlueprintCallable, Category = "Censor Billboard")
	void DeactivateCensor();

	/** Returns true if the censor is currently active */
	UFUNCTION(BlueprintPure, Category = "Censor Billboard")
	bool IsCensorActive() const;

private:
	/** Static mesh component for the chest censor plane */
	UPROPERTY()
	UStaticMeshComponent* ChestPlane = nullptr;

	/** Static mesh component for the groin censor plane */
	UPROPERTY()
	UStaticMeshComponent* GroinPlane = nullptr;

	/** Creates and registers the plane mesh components, attaches them to the owner's skeletal mesh sockets */
	void CreatePlaneComponents();

	/** Updates the world rotation of both planes to face the player camera */
	void UpdateBillboardRotations();
};
