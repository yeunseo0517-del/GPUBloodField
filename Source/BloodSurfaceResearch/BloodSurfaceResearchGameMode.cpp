// Copyright Epic Games, Inc. All Rights Reserved.

#include "BloodSurfaceResearchGameMode.h"
#include "BloodSurfaceResearchCharacter.h"
#include "UObject/ConstructorHelpers.h"

ABloodSurfaceResearchGameMode::ABloodSurfaceResearchGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
