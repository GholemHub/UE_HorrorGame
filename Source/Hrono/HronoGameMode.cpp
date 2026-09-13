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
	return bForceSessionStartDoorsUnlockedForTesting
		|| GetNumPlayers() >= FMath::Max(1, RequiredPlayersToStart);
}

void AHronoGameMode::ForceUnlockEntranceDoorsForTesting()
{
	if (!HasAuthority())
	{
		return;
	}

	// Keep the bypass active for the rest of this match. This also makes any
	// session-start gate streamed in later unlock itself during BeginPlay.
	bForceSessionStartDoorsUnlockedForTesting = true;

	for (TActorIterator<ADoorLockTrigger> It(GetWorld()); It; ++It)
	{
		ADoorLockTrigger* DoorLockTrigger = *It;
		if (IsValid(DoorLockTrigger) && DoorLockTrigger->bLockUntilAllPlayersPresent)
		{
			DoorLockTrigger->PermanentlyUnlockTriggeredDoors();
		}
	}
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
			DoorLockTrigger->PermanentlyUnlockTriggeredDoors();
		}
	}
}
