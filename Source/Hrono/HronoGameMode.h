// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HronoGameMode.generated.h"

/**
 *  Simple GameMode for a first person game
 */
UCLASS(abstract)
class AHronoGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHronoGameMode();

	/** Number of connected gameplay players required before session-start gates unlock. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Session Start",
		meta = (ClampMin = "1", UIMin = "1"))
	int32 RequiredPlayersToStart = 2;

	/** Server-only authoritative check used by session-start door triggers. */
	UFUNCTION(BlueprintPure, Category = "Session Start")
	bool AreAllRequiredPlayersPresent();

protected:
	virtual void StartPlay() override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

private:
	void TryUnlockSessionStartGates();
};



