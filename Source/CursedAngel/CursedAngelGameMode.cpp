// Copyright Epic Games, Inc. All Rights Reserved.

#include "CursedAngelGameMode.h"
#include "Ripley.h"
#include "UObject/ConstructorHelpers.h"

ACursedAngelGameMode::ACursedAngelGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
