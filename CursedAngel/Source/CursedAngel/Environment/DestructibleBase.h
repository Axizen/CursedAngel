// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "GeometryCollection/GeometryCollectionObject.h"
#include "Chaos/ClusterCreationParameters.h"
#include "Engine/StaticMeshActor.h"


// Forward declarations
class AGeometryCollectionActor;
class UFieldSystemComponent;

#include "DestructibleBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDestructibleBroken, AActor*, DamageCauser, float, DestructionWeight);