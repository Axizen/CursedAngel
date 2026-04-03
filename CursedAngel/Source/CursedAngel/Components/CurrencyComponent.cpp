// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/CurrencyComponent.h"
#include "Net/UnrealNetwork.h"

UCurrencyComponent::UCurrencyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Initialize currency values
	DataFragments = 0;
	CurseEssence = 0;
	CodeKeys = 0;

	// Enable replication for this component
	SetIsReplicatedByDefault(true);
}

void UCurrencyComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize currency values to ensure they are valid
	DataFragments = FMath::Max(0, DataFragments);
	CurseEssence = FMath::Max(0, CurseEssence);
	CodeKeys = FMath::Max(0, CodeKeys);
}

void UCurrencyComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Register replicated properties
	DOREPLIFETIME(UCurrencyComponent, DataFragments);
	DOREPLIFETIME(UCurrencyComponent, CurseEssence);
	DOREPLIFETIME(UCurrencyComponent, CodeKeys);
}

void UCurrencyComponent::AddDataFragments(int32 Amount)
{
	// Clamp amount to prevent negative additions
	Amount = FMath::Max(0, Amount);

	if (Amount > 0)
	{
		DataFragments += Amount;
		
		UE_LOG(LogTemp, Log, TEXT("CurrencyComponent: Added %d Data Fragments. New total: %d"), Amount, DataFragments);
		
		// Broadcast currency change event
		OnCurrencyChanged.Broadcast(DataFragments, CurseEssence, CodeKeys, FName("DataFragments"));
	}
}

void UCurrencyComponent::AddCurseEssence(int32 Amount)
{
	// Clamp amount to prevent negative additions
	Amount = FMath::Max(0, Amount);

	if (Amount > 0)
	{
		CurseEssence += Amount;
		
		UE_LOG(LogTemp, Log, TEXT("CurrencyComponent: Added %d Curse Essence. New total: %d"), Amount, CurseEssence);
		
		// Broadcast currency change event
		OnCurrencyChanged.Broadcast(DataFragments, CurseEssence, CodeKeys, FName("CurseEssence"));
	}
}

void UCurrencyComponent::AddCodeKeys(int32 Amount)
{
	// Clamp amount to prevent negative additions
	Amount = FMath::Max(0, Amount);

	if (Amount > 0)
	{
		CodeKeys += Amount;
		
		UE_LOG(LogTemp, Log, TEXT("CurrencyComponent: Added %d Code Keys. New total: %d"), Amount, CodeKeys);
		
		// Broadcast currency change event
		OnCurrencyChanged.Broadcast(DataFragments, CurseEssence, CodeKeys, FName("CodeKeys"));
	}
}

bool UCurrencyComponent::SpendDataFragments(int32 Amount)
{
	// Validate amount
	if (Amount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrencyComponent: Attempted to spend invalid amount of Data Fragments: %d"), Amount);
		return false;
	}

	// Check if sufficient funds
	if (DataFragments < Amount)
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrencyComponent: Insufficient Data Fragments. Required: %d, Available: %d"), Amount, DataFragments);
		return false;
	}

	// Spend currency
	DataFragments -= Amount;
	
	UE_LOG(LogTemp, Log, TEXT("CurrencyComponent: Spent %d Data Fragments. Remaining: %d"), Amount, DataFragments);
	
	// Broadcast currency change event
	OnCurrencyChanged.Broadcast(DataFragments, CurseEssence, CodeKeys, FName("DataFragments"));

	return true;
}

bool UCurrencyComponent::SpendCurseEssence(int32 Amount)
{
	// Validate amount
	if (Amount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrencyComponent: Attempted to spend invalid amount of Curse Essence: %d"), Amount);
		return false;
	}

	// Check if sufficient funds
	if (CurseEssence < Amount)
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrencyComponent: Insufficient Curse Essence. Required: %d, Available: %d"), Amount, CurseEssence);
		return false;
	}

	// Spend currency
	CurseEssence -= Amount;
	
	UE_LOG(LogTemp, Log, TEXT("CurrencyComponent: Spent %d Curse Essence. Remaining: %d"), Amount, CurseEssence);
	
	// Broadcast currency change event
	OnCurrencyChanged.Broadcast(DataFragments, CurseEssence, CodeKeys, FName("CurseEssence"));

	return true;
}

bool UCurrencyComponent::SpendCodeKeys(int32 Amount)
{
	// Validate amount
	if (Amount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrencyComponent: Attempted to spend invalid amount of Code Keys: %d"), Amount);
		return false;
	}

	// Check if sufficient funds
	if (CodeKeys < Amount)
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrencyComponent: Insufficient Code Keys. Required: %d, Available: %d"), Amount, CodeKeys);
		return false;
	}

	// Spend currency
	CodeKeys -= Amount;
	
	UE_LOG(LogTemp, Log, TEXT("CurrencyComponent: Spent %d Code Keys. Remaining: %d"), Amount, CodeKeys);
	
	// Broadcast currency change event
	OnCurrencyChanged.Broadcast(DataFragments, CurseEssence, CodeKeys, FName("CodeKeys"));

	return true;
}

int32 UCurrencyComponent::GetDataFragments() const
{
	return DataFragments;
}

int32 UCurrencyComponent::GetCurseEssence() const
{
	return CurseEssence;
}

int32 UCurrencyComponent::GetCodeKeys() const
{
	return CodeKeys;
}

void UCurrencyComponent::OnRep_DataFragments()
{
	// Called when DataFragments is replicated to clients
	UE_LOG(LogTemp, Log, TEXT("CurrencyComponent: DataFragments replicated. New value: %d"), DataFragments);
	
	// Broadcast currency change event for UI updates on client
	OnCurrencyChanged.Broadcast(DataFragments, CurseEssence, CodeKeys, FName("DataFragments"));
}

void UCurrencyComponent::OnRep_CurseEssence()
{
	// Called when CurseEssence is replicated to clients
	UE_LOG(LogTemp, Log, TEXT("CurrencyComponent: CurseEssence replicated. New value: %d"), CurseEssence);
	
	// Broadcast currency change event for UI updates on client
	OnCurrencyChanged.Broadcast(DataFragments, CurseEssence, CodeKeys, FName("CurseEssence"));
}

void UCurrencyComponent::OnRep_CodeKeys()
{
	// Called when CodeKeys is replicated to clients
	UE_LOG(LogTemp, Log, TEXT("CurrencyComponent: CodeKeys replicated. New value: %d"), CodeKeys);
	
	// Broadcast currency change event for UI updates on client
	OnCurrencyChanged.Broadcast(DataFragments, CurseEssence, CodeKeys, FName("CodeKeys"));
}
