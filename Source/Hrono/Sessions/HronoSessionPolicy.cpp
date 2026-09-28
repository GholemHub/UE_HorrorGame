#include "Sessions/HronoSessionPolicy.h"
#include "OnlineSubsystem.h"

FOnlineSessionSettings HronoSessionPolicy::MakeSettings(bool bLAN)
{
	FOnlineSessionSettings Settings;
	Settings.NumPublicConnections = MaxPlayers;
	Settings.NumPrivateConnections = 0;
	Settings.bIsLANMatch = bLAN;
	Settings.bShouldAdvertise = true;
	Settings.bAllowJoinInProgress = true;
	Settings.bAllowInvites = true;
	Settings.bUsesPresence = !bLAN;
	Settings.bAllowJoinViaPresence = !bLAN;
	Settings.bUseLobbiesIfAvailable = !bLAN;
	Settings.bUseLobbiesVoiceChatIfAvailable = false;
	Settings.Set(GameKey, FString(GameId), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	Settings.Set(ProtocolKey, Protocol, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	Settings.Set(MapKey, FString(GameplayMap), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	return Settings;
}

bool HronoSessionPolicy::IsCompatible(const FOnlineSessionSearchResult& Result, bool bRequireOpenSlot)
{
	const FOnlineSessionSettings& Settings = Result.Session.SessionSettings;
	FString Game, Map;
	int32 Version = 0;
	return Result.IsValid() && Settings.Get(GameKey, Game) && Game == GameId
		&& Settings.Get(ProtocolKey, Version) && Version == Protocol
		&& Settings.Get(MapKey, Map) && Map == GameplayMap
		&& Settings.BuildUniqueId == GetBuildUniqueId()
		&& Settings.NumPublicConnections == MaxPlayers && Settings.NumPrivateConnections == 0
		&& (!bRequireOpenSlot || Result.Session.NumOpenPublicConnections > 0);
}
