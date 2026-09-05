// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "HronoMenuSettingsSaveGame.generated.h"

/** Audio values that are not handled by UGameUserSettings. */
UCLASS()
class HRONO_API UHronoMenuSettingsSaveGame final : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(SaveGame)
	float MasterVolume = 1.0f;

	UPROPERTY(SaveGame)
	float MusicVolume = 1.0f;

	UPROPERTY(SaveGame)
	float SfxVolume = 1.0f;
};

