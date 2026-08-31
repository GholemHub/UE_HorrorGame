// Copyright Epic Games, Inc. All Rights Reserved.

#include "HronoGameMode.h"

#include "EngineUtils.h"
#include "Enviroment/DoorLockTrigger.h"

AHronoGameMode::AHronoGameMode()
{
}

void AHronoGameMode::StartPlay()
{
	Super::StartPlay();
	TryUnlockSessionStartGates();
}

void AHronoGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	TryUnlockSessionStartGates();
}

bool AHronoGameMode::AreAllRequiredPlayersPresent()
{
	return GetNumPlayers() >= FMath::Max(1, RequiredPlayersToStart);
}

void AHronoGameMode::TryUnlockSessionStartGates()
{
	if (!HasAuthority() || !AreAllRequiredPlayersPresent())
	{
		return;
	}

	for (TActorIterator<ADoorLockTrigger> It(GetWorld()); It; ++It)
	{
		ADoorLockTrigger* DoorLockTrigger = *It;
		if (IsValid(DoorLockTrigger) && DoorLockTrigger->bLockUntilAllPlayersPresent)
		{
			DoorLockTrigger->UnlockTriggeredDoors();
		}
	}
}
