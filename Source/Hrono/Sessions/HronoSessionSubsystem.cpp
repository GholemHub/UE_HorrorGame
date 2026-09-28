#include "Sessions/HronoSessionSubsystem.h"
#include "Sessions/HronoSessionPolicy.h"
#include "Hrono.h"
#include "Containers/Ticker.h"
#include "UI/HronoLoadingSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"

void UHronoSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Status = FText::FromString(TEXT("Choose Create Session or Find Sessions."));
	MapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::HandlePostLoadMap);
	if (GEngine)
	{
		NetworkHandle = GEngine->OnNetworkFailure().AddWeakLambda(this,
			[this](UWorld* World, UNetDriver*, ENetworkFailure::Type, const FString& Error) { HandleFailure(World, Error); });
		TravelHandle = GEngine->OnTravelFailure().AddWeakLambda(this,
			[this](UWorld* World, ETravelFailure::Type, const FString& Error) { HandleFailure(World, Error); });
	}
	EnsureInviteDelegate();
	TickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &ThisClass::TickOperation), 0.5f);
}

void UHronoSessionSubsystem::Deinitialize()
{
	ClearOperationDelegates();
	FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
	if (InviteSessions.IsValid())
	{
		InviteSessions->ClearOnSessionUserInviteAcceptedDelegate_Handle(InviteHandle);
		InviteSessions->ClearOnSessionFailureDelegate_Handle(SessionFailureHandle);
	}
	FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(MapHandle);
	if (GEngine)
	{
		GEngine->OnNetworkFailure().Remove(NetworkHandle);
		GEngine->OnTravelFailure().Remove(TravelHandle);
	}
	Sessions.Reset();
	InviteSessions.Reset();
	Super::Deinitialize();
}

IOnlineSessionPtr UHronoSessionSubsystem::ResolveSessions() const
{
	return Online::GetSessionInterface(GetWorld());
}

int32 UHronoSessionSubsystem::ResolveLocalUser() const
{
	const ULocalPlayer* Player = GetGameInstance()->GetFirstGamePlayer();
	return Player ? Player->GetControllerId() : INDEX_NONE;
}

void UHronoSessionSubsystem::EnsureInviteDelegate()
{
	const IOnlineSessionPtr Backend = ResolveSessions();
	if (Backend == InviteSessions) return;
	if (InviteSessions.IsValid())
	{
		InviteSessions->ClearOnSessionUserInviteAcceptedDelegate_Handle(InviteHandle);
		InviteSessions->ClearOnSessionFailureDelegate_Handle(SessionFailureHandle);
	}
	InviteSessions = Backend;
	if (Backend.IsValid())
	{
		InviteHandle = Backend->AddOnSessionUserInviteAcceptedDelegate_Handle(
			FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &ThisClass::HandleInvite));
		SessionFailureHandle = Backend->AddOnSessionFailureDelegate_Handle(FOnSessionFailureDelegate::CreateWeakLambda(this,
			[this](const FUniqueNetId&, ESessionFailure::Type) { HandleFailure(GetWorld(), TEXT("Online session connection lost.")); }));
	}
}

bool UHronoSessionSubsystem::IsBusy() const
{
	return bMenuPreloadPending || State == EHronoSessionState::Destroying || State == EHronoSessionState::Creating
		|| State == EHronoSessionState::Starting || State == EHronoSessionState::Searching
		|| State == EHronoSessionState::Joining || State == EHronoSessionState::Travelling;
}

bool UHronoSessionSubsystem::ReserveMenuPreload()
{
	if (IsBusy() || State == EHronoSessionState::InSession || (GetWorld() && GetWorld()->GetNetMode() != NM_Standalone)) return false;
	bMenuPreloadPending = true;
	Status = FText::FromString(TEXT("Loading content..."));
	OnChanged.Broadcast();
	return true;
}

void UHronoSessionSubsystem::ReleaseMenuPreload()
{
	if (bMenuPreloadPending) Status = FText::FromString(TEXT("Ready. Choose a session action."));
	bMenuPreloadPending = false;
}

void UHronoSessionSubsystem::SetState(EHronoSessionState NewState, const FString& Message)
{
	State = NewState;
	OperationStartedAt = FPlatformTime::Seconds();
	bTimedOut = false;
	Status = FText::FromString(Message);
	UE_LOG(LogHrono, Log, TEXT("Session state=%d: %s"), static_cast<int32>(State), *Message);
	OnChanged.Broadcast();
}

bool UHronoSessionSubsystem::TickOperation(float DeltaSeconds)
{
	EnsureInviteDelegate();
	if (bMenuPreloadPending || !IsBusy() || bTimedOut || FPlatformTime::Seconds() - OperationStartedAt < 60.0) return true;
	if (State == EHronoSessionState::Travelling)
	{
		Fail(TEXT("Connection timed out. Please try again."), true);
		return true;
	}
	// Create/Join/Destroy have no cancellation API. Do not start a competing
	// operation until their completion arrives; clean up even a late success.
	bTimedOut = true;
	FailureMessage = TEXT("Online request timed out. Try again.");
	CancelLoading();
	Status = FText::FromString(TEXT("Online service timed out. Waiting for cleanup; you can exit the game."));
	if (State == EHronoSessionState::Destroying)
	{
		AfterDestroy = EAfterDestroy::Fail;
		FailureMessage = TEXT("Closing the session timed out. You can now retry.");
	}
	if (State == EHronoSessionState::Searching && Sessions.IsValid()) Sessions->CancelFindSessions();
	OnChanged.Broadcast();
	return true;
}

bool UHronoSessionSubsystem::BeginRequest()
{
	if (IsBusy()) return false;
	EnsureInviteDelegate();
	Sessions = ResolveSessions();
	LocalUserNum = ResolveLocalUser();
	if (State == EHronoSessionState::InSession || (GetWorld() && GetWorld()->GetNetMode() != NM_Standalone))
	{
		Status = FText::FromString(TEXT("Leave the current session before creating or joining another."));
		OnChanged.Broadcast();
		return false;
	}
	if (!Sessions.IsValid() || LocalUserNum < 0)
	{
		Fail(TEXT("Online service or local player is unavailable. Check Steam and try again."));
		return false;
	}
	bReturnAfterFailure = false;
	return true;
}

void UHronoSessionSubsystem::HostSession(bool bLAN)
{
	if (!BeginRequest()) return;
	bLANRequest = bLAN;
	Entries.Reset(); Results.Reset(); Search.Reset();
	DestroyThen(EAfterDestroy::Host);
}

void UHronoSessionSubsystem::FindSessions(bool bLAN)
{
	if (!BeginRequest()) return;
	bLANRequest = bLAN;
	Entries.Reset(); Results.Reset(); Search.Reset();
	DestroyThen(EAfterDestroy::Find);
}

void UHronoSessionSubsystem::JoinSessionByIndex(int32 Index)
{
	if (IsBusy()) return;
	if (State == EHronoSessionState::InSession) { BeginRequest(); return; }
	if (!Results.IsValidIndex(Index) || !HronoSessionPolicy::IsCompatible(Results[Index]))
	{
		CancelLoading();
		SetState(EHronoSessionState::Failed, TEXT("Select an available compatible session, or search again."));
		return;
	}
	if (!BeginRequest()) return;
	PendingJoin = Results[Index];
	DestroyThen(EAfterDestroy::Join);
}

void UHronoSessionSubsystem::HandleInvite(bool bSuccess, int32 LocalUser, FUniqueNetIdPtr User, const FOnlineSessionSearchResult& Result)
{
	if (IsBusy() || State == EHronoSessionState::InSession || (GetWorld() && GetWorld()->GetNetMode() != NM_Standalone))
	{
		Status = FText::FromString(TEXT("Invite ignored: finish or leave the current session first."));
		OnChanged.Broadcast();
		return;
	}
	if (LocalUser != ResolveLocalUser()) return;
	if (!bSuccess || !HronoSessionPolicy::IsCompatible(Result))
	{
		Fail(TEXT("The invited session is full or uses a different game version."));
		return;
	}
	if (!BeginRequest()) return;
	PendingJoin = Result;
	DestroyThen(EAfterDestroy::Join);
}

void UHronoSessionSubsystem::ClearOperationDelegates()
{
	if (!Sessions.IsValid()) return;
	Sessions->ClearOnCreateSessionCompleteDelegate_Handle(CreateHandle); CreateHandle.Reset();
	Sessions->ClearOnStartSessionCompleteDelegate_Handle(StartHandle); StartHandle.Reset();
	Sessions->ClearOnFindSessionsCompleteDelegate_Handle(FindHandle); FindHandle.Reset();
	Sessions->ClearOnJoinSessionCompleteDelegate_Handle(JoinHandle); JoinHandle.Reset();
	Sessions->ClearOnDestroySessionCompleteDelegate_Handle(DestroyHandle); DestroyHandle.Reset();
}

void UHronoSessionSubsystem::CancelLoading()
{
	if (UHronoLoadingSubsystem* Loading = GetGameInstance()->GetSubsystem<UHronoLoadingSubsystem>())
		Loading->CancelLoadingFlow();
}

void UHronoSessionSubsystem::Fail(const FString& Message, bool bReturn)
{
	bMenuPreloadPending = false;
	ClearOperationDelegates();
	CancelLoading();
	FailureMessage = Message;
	bReturnAfterFailure = bReturn;
	DestroyThen(EAfterDestroy::Fail);
}

void UHronoSessionSubsystem::DestroyThen(EAfterDestroy Next)
{
	AfterDestroy = Next;
	if (!Sessions.IsValid() || !Sessions->GetNamedSession(NAME_GameSession))
	{
		ContinueAfterDestroy();
		return;
	}
	SetState(EHronoSessionState::Destroying, TEXT("Closing the previous session..."));
	DestroyHandle = Sessions->AddOnDestroySessionCompleteDelegate_Handle(
		FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::HandleDestroy));
	if (!Sessions->DestroySession(NAME_GameSession)) HandleDestroy(NAME_GameSession, false);
}

void UHronoSessionSubsystem::HandleDestroy(FName Name, bool bSuccess)
{
	if (Name != NAME_GameSession || State != EHronoSessionState::Destroying) return;
	Sessions->ClearOnDestroySessionCompleteDelegate_Handle(DestroyHandle); DestroyHandle.Reset();
	if (!bSuccess && Sessions->GetNamedSession(NAME_GameSession))
	{
		CancelLoading();
		SetState(EHronoSessionState::Failed, TEXT("Could not close the previous session. Retry or restart the game."));
		if (bReturnAfterFailure || AfterDestroy == EAfterDestroy::Leave) ReturnToMenu();
		return;
	}
	ContinueAfterDestroy();
}

void UHronoSessionSubsystem::ContinueAfterDestroy()
{
	const EAfterDestroy Next = AfterDestroy;
	AfterDestroy = EAfterDestroy::None;
	switch (Next)
	{
	case EAfterDestroy::Host: Create(); break;
	case EAfterDestroy::Find: SearchSessions(); break;
	case EAfterDestroy::Join: Join(); break;
	case EAfterDestroy::Leave:
		SetState(EHronoSessionState::Idle, TEXT("Session closed.")); ReturnToMenu(); break;
	case EAfterDestroy::Fail:
		SetState(EHronoSessionState::Failed, FailureMessage);
		if (bReturnAfterFailure) ReturnToMenu();
		break;
	default: break;
	}
}

void UHronoSessionSubsystem::Create()
{
	SetState(EHronoSessionState::Creating, TEXT("Creating a session for two players..."));
	CreateHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::HandleCreate));
	const FOnlineSessionSettings Settings = HronoSessionPolicy::MakeSettings(bLANRequest);
	if (!Sessions->CreateSession(LocalUserNum, NAME_GameSession, Settings) && State == EHronoSessionState::Creating)
		Fail(TEXT("Could not start session creation. Check the online service and retry."));
}

void UHronoSessionSubsystem::HandleCreate(FName Name, bool bSuccess)
{
	if (Name != NAME_GameSession || State != EHronoSessionState::Creating) return;
	Sessions->ClearOnCreateSessionCompleteDelegate_Handle(CreateHandle); CreateHandle.Reset();
	if (bTimedOut) { Fail(FailureMessage, bReturnAfterFailure); return; }
	if (!bSuccess) { Fail(TEXT("Session creation failed. Try again.")); return; }
	SetState(EHronoSessionState::Starting, TEXT("Starting the session..."));
	StartHandle = Sessions->AddOnStartSessionCompleteDelegate_Handle(
		FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::HandleStart));
	if (!Sessions->StartSession(NAME_GameSession) && State == EHronoSessionState::Starting) Fail(TEXT("Could not start the created session. Try again."));
}

void UHronoSessionSubsystem::HandleStart(FName Name, bool bSuccess)
{
	if (Name != NAME_GameSession || State != EHronoSessionState::Starting) return;
	Sessions->ClearOnStartSessionCompleteDelegate_Handle(StartHandle); StartHandle.Reset();
	if (bTimedOut) { Fail(FailureMessage, bReturnAfterFailure); return; }
	if (!bSuccess) { Fail(TEXT("Session start failed. Try again.")); return; }
	SetState(EHronoSessionState::Travelling, TEXT("Loading the game..."));
	if (!TravelToSession(HronoSessionPolicy::GameplayMap, true)) Fail(TEXT("Could not open the gameplay map."));
}

void UHronoSessionSubsystem::SearchSessions()
{
	Search = MakeShared<FOnlineSessionSearch>();
	Search->bIsLanQuery = bLANRequest;
	Search->MaxSearchResults = 100;
	Search->TimeoutInSeconds = 30.0f;
	if (!bLANRequest) Search->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	Search->QuerySettings.Set(HronoSessionPolicy::GameKey, FString(HronoSessionPolicy::GameId), EOnlineComparisonOp::Equals);
	SetState(EHronoSessionState::Searching, TEXT("Finding available sessions..."));
	FindHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::HandleFind));
	if (!Sessions->FindSessions(LocalUserNum, Search.ToSharedRef()) && State == EHronoSessionState::Searching) Fail(TEXT("Could not start the search. Try again."));
}

void UHronoSessionSubsystem::HandleFind(bool bSuccess)
{
	if (State != EHronoSessionState::Searching) return;
	Sessions->ClearOnFindSessionsCompleteDelegate_Handle(FindHandle); FindHandle.Reset();
	if (bTimedOut) { Fail(FailureMessage, bReturnAfterFailure); return; }
	CancelLoading();
	if (!bSuccess || !Search.IsValid()) { Fail(TEXT("Session search failed. Try again.")); return; }
	TSet<FString> Seen;
	for (const FOnlineSessionSearchResult& Result : Search->SearchResults)
	{
		if (!HronoSessionPolicy::IsCompatible(Result) || Seen.Contains(Result.GetSessionIdStr())) continue;
		Seen.Add(Result.GetSessionIdStr());
		Results.Add(Result);
		FHronoSessionEntry& Entry = Entries.AddDefaulted_GetRef();
		Entry.Host = Result.Session.OwningUserName;
		Entry.SessionId = Result.GetSessionIdStr();
		Entry.Ping = Result.PingInMs;
		Entry.OpenSlots = Result.Session.NumOpenPublicConnections;
	}
	SetState(EHronoSessionState::ResultsReady, Entries.IsEmpty()
		? TEXT("No available sessions found. Create one or search again.") : TEXT("Select a host, then join the selected session."));
}

void UHronoSessionSubsystem::Join()
{
	// Steam requires matching presence/lobby flags; returned lobby structs may omit them.
	if (!PendingJoin.Session.SessionSettings.bIsLANMatch && !PendingJoin.Session.SessionSettings.bIsDedicated)
	{
		PendingJoin.Session.SessionSettings.bUsesPresence = true;
		PendingJoin.Session.SessionSettings.bUseLobbiesIfAvailable = true;
	}
	SetState(EHronoSessionState::Joining, TEXT("Joining the selected session..."));
	JoinHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::HandleJoin));
	if (!Sessions->JoinSession(LocalUserNum, NAME_GameSession, PendingJoin) && State == EHronoSessionState::Joining) Fail(TEXT("Could not start joining. Search again."));
}

void UHronoSessionSubsystem::HandleJoin(FName Name, EOnJoinSessionCompleteResult::Type Result)
{
	if (Name != NAME_GameSession || State != EHronoSessionState::Joining) return;
	Sessions->ClearOnJoinSessionCompleteDelegate_Handle(JoinHandle); JoinHandle.Reset();
	if (bTimedOut) { Fail(FailureMessage, bReturnAfterFailure); return; }
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		Fail(Result == EOnJoinSessionCompleteResult::SessionIsFull ? TEXT("The session is now full. Search again.")
			: TEXT("Could not join this session. It may have closed. Search again."));
		return;
	}
	FString Address;
	if (!Sessions->GetResolvedConnectString(NAME_GameSession, Address) || Address.IsEmpty())
	{
		Fail(TEXT("The session returned no server address. Search again.")); return;
	}
	SetState(EHronoSessionState::Travelling, TEXT("Connecting to the host..."));
	if (!TravelToSession(Address, false)) Fail(TEXT("No local player is available for connection."));
}

bool UHronoSessionSubsystem::TravelToSession(const FString& Address, bool bHost)
{
	if (!GetWorld()) return false;
	if (bHost) UGameplayStatics::OpenLevel(this, FName(*Address), true, TEXT("listen"));
	else
	{
		APlayerController* Player = GetGameInstance()->GetFirstLocalPlayerController();
		if (!Player) return false;
		Player->ClientTravel(Address, TRAVEL_Absolute);
	}
	return true;
}

void UHronoSessionSubsystem::HandlePostLoadMap(UWorld* World)
{
	if (!World || World->GetGameInstance() != GetGameInstance()) return;
	EnsureInviteDelegate();
	const FString Map = UWorld::RemovePIEPrefix(World->GetOutermost()->GetName());
	if (State == EHronoSessionState::Travelling && Map == HronoSessionPolicy::GameplayMap)
		SetState(EHronoSessionState::InSession, TEXT("Connected."));
	else if (State == EHronoSessionState::InSession && Map != HronoSessionPolicy::GameplayMap)
	{
		// Existing OpenLevel-to-menu callers must not leave a stale named session.
		Sessions = ResolveSessions();
		Fail(TEXT("Session ended."));
	}
}

void UHronoSessionSubsystem::HandleFailure(UWorld* World, const FString& Message)
{
	if (!World || World->GetGameInstance() != GetGameInstance()) return;
	if (State == EHronoSessionState::Creating || State == EHronoSessionState::Starting
		|| State == EHronoSessionState::Joining || State == EHronoSessionState::Searching)
	{
		// Preserve the pending delegate: even a failed connection may deliver a
		// late create/join success, which must be cleaned before another request.
		bTimedOut = true;
		FailureMessage = FString::Printf(TEXT("Connection failed: %s"), *Message);
		bReturnAfterFailure = true;
		CancelLoading();
		Status = FText::FromString(TEXT("Connection failed. Waiting for online service cleanup..."));
		if (State == EHronoSessionState::Searching && Sessions.IsValid()) Sessions->CancelFindSessions();
		OnChanged.Broadcast();
		return;
	}
	if (State == EHronoSessionState::Destroying)
	{
		AfterDestroy = EAfterDestroy::Fail;
		FailureMessage = FString::Printf(TEXT("Connection failed: %s"), *Message);
		bReturnAfterFailure = true;
		CancelLoading();
		return;
	}
	if (IsBusy() || State == EHronoSessionState::InSession)
		Fail(FString::Printf(TEXT("Connection failed: %s"), *Message), true);
}

void UHronoSessionSubsystem::LeaveSession()
{
	if (IsBusy()) return;
	Sessions = ResolveSessions();
	CancelLoading();
	Entries.Reset(); Results.Reset(); Search.Reset();
	DestroyThen(EAfterDestroy::Leave);
}

void UHronoSessionSubsystem::ReturnToMenu()
{
	GetGameInstance()->ReturnToMainMenu();
}

void UHronoSessionSubsystem::RequestHost(UObject* WorldContextObject)
{
	if (UGameInstance* Game = UGameplayStatics::GetGameInstance(WorldContextObject))
		if (UHronoSessionSubsystem* Session = Game->GetSubsystem<UHronoSessionSubsystem>()) Session->HostSession();
}
