#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionSettings.h"

/** Shared advertising/admission contract. Bump Protocol when network gameplay changes. */
namespace HronoSessionPolicy
{
	inline constexpr int32 MaxPlayers = 2;
	inline constexpr int32 Protocol = 10;
	inline constexpr const TCHAR* GameplayMap = TEXT("/Game/_Alex/DemoMap1");
	inline const FName GameKey(TEXT("HRONO_GAME"));
	inline const FName ProtocolKey(TEXT("HRONO_PROTOCOL"));
	inline const FName MapKey(TEXT("HRONO_MAP"));
	inline constexpr const TCHAR* GameId = TEXT("689194FA4DB5B3760B91C296C562CFD4");
	FOnlineSessionSettings MakeSettings(bool bLAN);
	bool IsCompatible(const FOnlineSessionSearchResult& Result, bool bRequireOpenSlot = true);
}
