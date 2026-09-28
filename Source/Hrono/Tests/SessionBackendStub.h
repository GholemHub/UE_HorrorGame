#pragma once
#include "Interfaces/OnlineSessionInterface.h"

// Unused external API defaults for the scripted OSS test backend.
#if WITH_DEV_AUTOMATION_TESTS
class FSessionBackendStub : public IOnlineSession
{
public:
	virtual class FNamedOnlineSession* AddNamedSession(FName SessionName, const FOnlineSessionSettings& SessionSettings) override { return {}; }
	virtual class FNamedOnlineSession* AddNamedSession(FName SessionName, const FOnlineSession& Session) override { return {}; }
	virtual FUniqueNetIdPtr CreateSessionIdFromString(const FString& SessionIdStr) override { return {}; }
	virtual class FNamedOnlineSession* GetNamedSession(FName SessionName) override { return {}; }
	virtual void RemoveNamedSession(FName SessionName) override {}
	virtual bool HasPresenceSession() override { return {}; }
	virtual EOnlineSessionState::Type GetSessionState(FName SessionName) const override { return {}; }
	virtual bool CreateSession(int32 HostingPlayerNum, FName SessionName, const FOnlineSessionSettings& NewSessionSettings) override { return {}; }
	virtual bool CreateSession(const FUniqueNetId& HostingPlayerId, FName SessionName, const FOnlineSessionSettings& NewSessionSettings) override { return {}; }
	virtual bool StartSession(FName SessionName) override { return {}; }
	virtual bool UpdateSession(FName SessionName, FOnlineSessionSettings& UpdatedSessionSettings, bool bShouldRefreshOnlineData = true) override { return {}; }
	virtual bool EndSession(FName SessionName) override { return {}; }
	virtual bool DestroySession(FName SessionName, const FOnDestroySessionCompleteDelegate& CompletionDelegate = FOnDestroySessionCompleteDelegate()) override { return {}; }
	virtual bool IsPlayerInSession(FName SessionName, const FUniqueNetId& UniqueId) override { return {}; }
	virtual bool StartMatchmaking(const TArray< FUniqueNetIdRef >& LocalPlayers, FName SessionName, const FOnlineSessionSettings& NewSessionSettings, TSharedRef<FOnlineSessionSearch>& SearchSettings) override { return {}; }
	virtual bool CancelMatchmaking(int32 SearchingPlayerNum, FName SessionName) override { return {}; }
	virtual bool CancelMatchmaking(const FUniqueNetId& SearchingPlayerId, FName SessionName) override { return {}; }
	virtual bool FindSessions(int32 SearchingPlayerNum, const TSharedRef<FOnlineSessionSearch>& SearchSettings) override { return {}; }
	virtual bool FindSessions(const FUniqueNetId& SearchingPlayerId, const TSharedRef<FOnlineSessionSearch>& SearchSettings) override { return {}; }
	virtual bool FindSessionById(const FUniqueNetId& SearchingUserId, const FUniqueNetId& SessionId, const FUniqueNetId& FriendId, const FOnSingleSessionResultCompleteDelegate& CompletionDelegate) override { return {}; }
	virtual bool CancelFindSessions() override { return {}; }
	virtual bool PingSearchResults(const FOnlineSessionSearchResult& SearchResult) override { return {}; }
	virtual bool JoinSession(int32 LocalUserNum, FName SessionName, const FOnlineSessionSearchResult& DesiredSession) override { return {}; }
	virtual bool JoinSession(const FUniqueNetId& LocalUserId, FName SessionName, const FOnlineSessionSearchResult& DesiredSession) override { return {}; }
	virtual bool FindFriendSession(int32 LocalUserNum, const FUniqueNetId& Friend) override { return {}; }
	virtual bool FindFriendSession(const FUniqueNetId& LocalUserId, const FUniqueNetId& Friend) override { return {}; }
	virtual bool FindFriendSession(const FUniqueNetId& LocalUserId, const TArray<FUniqueNetIdRef>& FriendList) override { return {}; }
	virtual bool SendSessionInviteToFriend(int32 LocalUserNum, FName SessionName, const FUniqueNetId& Friend) override { return {}; }
	virtual bool SendSessionInviteToFriend(const FUniqueNetId& LocalUserId, FName SessionName, const FUniqueNetId& Friend) override { return {}; }
	virtual bool SendSessionInviteToFriends(int32 LocalUserNum, FName SessionName, const TArray< FUniqueNetIdRef >& Friends) override { return {}; }
	virtual bool SendSessionInviteToFriends(const FUniqueNetId& LocalUserId, FName SessionName, const TArray< FUniqueNetIdRef >& Friends) override { return {}; }
	virtual bool GetResolvedConnectString(FName SessionName, FString& ConnectInfo, FName PortType = NAME_GamePort) override { return {}; }
	virtual bool GetResolvedConnectString(const class FOnlineSessionSearchResult& SearchResult, FName PortType, FString& ConnectInfo) override { return {}; }
	virtual FOnlineSessionSettings* GetSessionSettings(FName SessionName) override { return {}; }
	virtual bool RegisterPlayer(FName SessionName, const FUniqueNetId& PlayerId, bool bWasInvited) override { return {}; }
	virtual bool RegisterPlayers(FName SessionName, const TArray< FUniqueNetIdRef >& Players, bool bWasInvited = false) override { return {}; }
	virtual bool UnregisterPlayer(FName SessionName, const FUniqueNetId& PlayerId) override { return {}; }
	virtual bool UnregisterPlayers(FName SessionName, const TArray< FUniqueNetIdRef >& Players) override { return {}; }
	virtual void RegisterLocalPlayer(const FUniqueNetId& PlayerId, FName SessionName, const FOnRegisterLocalPlayerCompleteDelegate& Delegate) override {}
	virtual void UnregisterLocalPlayer(const FUniqueNetId& PlayerId, FName SessionName, const FOnUnregisterLocalPlayerCompleteDelegate& Delegate) override {}
	virtual int32 GetNumSessions() override { return {}; }
	virtual void DumpSessionState() override {}
};
#endif
