// Copyright Epic Games, Inc. All Rights Reserved.

#include "StratosGameMode.h"
#include "StratosCharacter.h"
#include "UObject/ConstructorHelpers.h"

AStratosGameMode::AStratosGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
