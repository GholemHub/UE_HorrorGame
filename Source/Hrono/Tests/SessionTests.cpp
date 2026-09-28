#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/SessionBackendStub.h"
#include "Tests/SessionTestSubsystem.h"
#include "Tests/RadioVoiceTestController.h"
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Sessions/HronoSessionPolicy.h"
#include "Sessions/HronoGameSession.h"
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "HAL/IConsoleManager.h"
#include "Subsystems/SubsystemCollection.h"
#include "UObject/StrongObjectPtr.h"

namespace SessionTests
{
class FInfo : public FOnlineSessionInfo
{
	FUniqueNetIdRef Id;
public:
	explicit FInfo(const FString& Name) : Id(FUniqueNetIdString::Create(Name, TEXT("NULL"))) {}
	virtual const FUniqueNetId& GetSessionId() const override { return *Id; }
	virtual const uint8* GetBytes() const override { return Id->GetBytes(); }
	virtual int32 GetSize() const override { return Id->GetSize(); }
	virtual bool IsValid() const override { return true; }
	virtual FString ToString() const override { return Id->ToString(); }
	virtual FString ToDebugString() const override { return ToString(); }
};

FOnlineSessionSearchResult Result(const TCHAR* Name)
{
	FOnlineSessionSearchResult Row;
	Row.Session.SessionInfo = MakeShared<FInfo>(Name);
	Row.Session.OwningUserName = Name;
	Row.Session.OwningUserId = FUniqueNetIdString::Create(FString(Name) + TEXT("_user"), TEXT("NULL"));
	Row.Session.SessionSettings = HronoSessionPolicy::MakeSettings(false);
	Row.Session.NumOpenPublicConnections = 1;
	return Row;
}

class FBackend : public FSessionBackendStub
{
public:
	TUniquePtr<FNamedOnlineSession> Named;
	TSharedPtr<FOnlineSessionSearch> Search;
	FString JoinedId;
	int32 Creates = 0, Finds = 0, Joins = 0, Destroys = 0;
	bool bAcceptCreate = true, bAcceptFind = true, bAcceptJoin = true, bAcceptStart = true;
	bool bResolve = true, bAcceptDestroy = true;
	virtual FNamedOnlineSession* GetNamedSession(FName Name) override { return Named.Get(); }
	virtual bool CreateSession(int32 User, FName Name, const FOnlineSessionSettings& Settings) override
	{
		++Creates;
		if (!bAcceptCreate) return false;
		Named = MakeUnique<FNamedOnlineSession>(Name, Settings);
		return true;
	}
	virtual bool StartSession(FName Name) override { return bAcceptStart; }
	virtual bool FindSessions(int32 User, const TSharedRef<FOnlineSessionSearch>& Request) override
	{ ++Finds; Search = Request; return bAcceptFind; }
	virtual bool JoinSession(int32 User, FName Name, const FOnlineSessionSearchResult& Row) override
	{
		++Joins; JoinedId = Row.GetSessionIdStr();
		if (!bAcceptJoin) return false;
		Named = MakeUnique<FNamedOnlineSession>(Name, Row.Session);
		return true;
	}
	virtual bool GetResolvedConnectString(FName Name, FString& Address, FName PortType) override
	{ Address = TEXT("127.0.0.1:7777"); return bResolve; }
	virtual bool DestroySession(FName Name, const FOnDestroySessionCompleteDelegate& Delegate) override
	{ ++Destroys; return bAcceptDestroy; }
	void Destroyed(bool bSuccess = true)
	{
		if (bSuccess) Named.Reset();
		TriggerOnDestroySessionCompleteDelegates(NAME_GameSession, bSuccess);
	}
};

struct FScope
{
	TStrongObjectPtr<UGameInstance> Game{NewObject<UGameInstance>(GEngine)};
	UWorld* World;
	FScope(bool bMode = false)
	{
		Game->InitializeStandalone(); World = Game->GetWorld();
		if (bMode) World->SetGameMode(FURL());
		World->InitializeActorsForPlay(FURL());
	}
	~FScope()
	{
		Game->Shutdown(); World->DestroyWorld(false); GEngine->DestroyWorldContext(World);
	}
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHronoSessionPolicyTest, "Hrono.Sessions.Compatibility",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHronoSessionPolicyTest::RunTest(const FString& Parameters)
{
	using namespace SessionTests;
	const auto Valid = Result(TEXT("host"));
	TestTrue(TEXT("Our advertised session is compatible"), HronoSessionPolicy::IsCompatible(Valid));
	TestEqual(TEXT("Advertised maximum includes host"), Valid.Session.SessionSettings.NumPublicConnections, 2);
	auto Wrong = Valid; Wrong.Session.SessionSettings.NumPublicConnections = 100;
	TestFalse(TEXT("Old hundred-slot lobby excluded"), HronoSessionPolicy::IsCompatible(Wrong));
	Wrong = Valid; Wrong.Session.SessionSettings.Set(HronoSessionPolicy::GameKey, FString(TEXT("other app")));
	TestFalse(TEXT("Other Steam 480 game excluded"), HronoSessionPolicy::IsCompatible(Wrong));
	Wrong = Valid; Wrong.Session.SessionSettings.Set(HronoSessionPolicy::ProtocolKey, 99);
	TestFalse(TEXT("Incompatible gameplay protocol excluded"), HronoSessionPolicy::IsCompatible(Wrong));
	Wrong = Valid; ++Wrong.Session.SessionSettings.BuildUniqueId;
	TestFalse(TEXT("Incompatible engine build excluded"), HronoSessionPolicy::IsCompatible(Wrong));
	Wrong = Valid; Wrong.Session.SessionSettings.Set(HronoSessionPolicy::MapKey, FString(TEXT("other map")));
	TestFalse(TEXT("Unexpected map excluded"), HronoSessionPolicy::IsCompatible(Wrong));
	Wrong = Valid; Wrong.Session.NumOpenPublicConnections = 0;
	TestFalse(TEXT("Full lobby excluded"), HronoSessionPolicy::IsCompatible(Wrong));
	Wrong = Valid; Wrong.Session.SessionInfo.Reset();
	TestFalse(TEXT("Invalid result excluded"), HronoSessionPolicy::IsCompatible(Wrong));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHronoSessionFlowTest, "Hrono.Sessions.AsyncFlow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHronoSessionFlowTest::RunTest(const FString& Parameters)
{
	using namespace SessionTests;
	FScope Scope;
	TStrongObjectPtr<USessionTestSubsystem> Service(NewObject<USessionTestSubsystem>(Scope.Game.Get()));
	const auto Backend = MakeShared<FBackend, ESPMode::ThreadSafe>();
	Service->Backend = Backend;
	FSubsystemCollection<UGameInstanceSubsystem> Collection;
	Service->Initialize(Collection);
	auto StateIs = [this, &Service](const TCHAR* Name, EHronoSessionState Expected) { TestEqual(Name, Service->State, Expected); };
	TestTrue(TEXT("Menu preload reserves the canonical owner"), Service->ReserveMenuPreload());
	Backend->TriggerOnSessionUserInviteAcceptedDelegates(true, 0, FUniqueNetIdPtr(), Result(TEXT("during preload")));
	TestEqual(TEXT("Invite cannot interrupt menu preload"), Backend->Joins, 0);
	Service->ReleaseMenuPreload();
	Service->FindSessions();
	StateIs(TEXT("Search started"), EHronoSessionState::Searching);
	Service->HostSession(); Service->FindSessions();
	TestEqual(TEXT("Duplicate requests suppressed"), Backend->Finds, 1);
	TestEqual(TEXT("Competing create suppressed"), Backend->Creates, 0);
	Backend->TriggerOnFindSessionsCompleteDelegates(true);
	StateIs(TEXT("Zero results is a normal terminal state"), EHronoSessionState::ResultsReady);
	TestEqual(TEXT("No automatic join for empty results"), Backend->Joins, 0);
	Service->JoinSessionByIndex(0);
	StateIs(TEXT("Invalid index fails safely"), EHronoSessionState::Failed);
	Service->FindSessions();
	auto Full = Result(TEXT("full")); Full.Session.NumOpenPublicConnections = 0;
	auto Old = Result(TEXT("old")); Old.Session.SessionSettings.NumPublicConnections = 100;
	Backend->Search->SearchResults = { Result(TEXT("A")), Full, Old, Result(TEXT("B")), Result(TEXT("A")) };
	Backend->TriggerOnFindSessionsCompleteDelegates(true);
	TestEqual(TEXT("Only distinct compatible open hosts displayed"), Service->Entries.Num(), 2);
	TestEqual(TEXT("Search never joins the first host"), Backend->Joins, 0);
	Service->JoinSessionByIndex(1);
	TestEqual(TEXT("Explicit selection joins second host"), Backend->JoinedId, FString(TEXT("B")));
	Service->JoinSessionByIndex(0);
	TestEqual(TEXT("Duplicate join suppressed"), Backend->Joins, 1);
	Backend->TriggerOnJoinSessionCompleteDelegates(NAME_GameSession, EOnJoinSessionCompleteResult::SessionIsFull);
	StateIs(TEXT("Failed join cleans named session before retry"), EHronoSessionState::Destroying);
	Backend->Destroyed();
	StateIs(TEXT("Retry enabled after cleanup"), EHronoSessionState::Failed);
	Backend->bAcceptCreate = false;
	Service->HostSession();
	StateIs(TEXT("Synchronous create rejection recovers"), EHronoSessionState::Failed);
	Backend->bAcceptCreate = true;
	Service->HostSession();
	Backend->TriggerOnCreateSessionCompleteDelegates(NAME_GameSession, true);
	StateIs(TEXT("Successful creation explicitly starts session"), EHronoSessionState::Starting);
	Backend->TriggerOnStartSessionCompleteDelegates(NAME_GameSession, false);
	StateIs(TEXT("Failed start also cleans created session"), EHronoSessionState::Destroying);
	Backend->Destroyed();
	Service->HostSession();
	Service->OperationStartedAt = FPlatformTime::Seconds() - 61.0;
	Service->TickOperation(0.5f);
	TestTrue(TEXT("Timed-out backend cannot overlap a new operation"), Service->IsBusy());
	const int32 CreatesBeforeRetry = Backend->Creates;
	Service->HostSession();
	TestEqual(TEXT("Retry blocked until old operation settles"), Backend->Creates, CreatesBeforeRetry);
	Backend->TriggerOnCreateSessionCompleteDelegates(NAME_GameSession, true);
	StateIs(TEXT("Late success after timeout is cleaned, never travelled"), EHronoSessionState::Destroying);
	TestEqual(TEXT("No travel after expired request"), Service->Travels, 0);
	Backend->Destroyed();
	Service->HostSession();
	Backend->TriggerOnCreateSessionCompleteDelegates(NAME_GameSession, true);
	Backend->TriggerOnStartSessionCompleteDelegates(NAME_GameSession, true);
	StateIs(TEXT("Host stays busy during travel"), EHronoSessionState::Travelling);
	TestEqual(TEXT("Host destination is canonical map"), Service->LastAddress, FString(HronoSessionPolicy::GameplayMap));
	const int32 HostedCreates = Backend->Creates;
	Backend->TriggerOnSessionUserInviteAcceptedDelegates(true, 0, FUniqueNetIdPtr(), Result(TEXT("invite")));
	TestEqual(TEXT("Invite does not interrupt travel"), Backend->Joins, 1);
	Service->ConnectionFailure(Scope.World);
	Backend->Destroyed();
	TestEqual(TEXT("Network failure returns to menu after cleanup"), Service->MenuReturns, 1);
	StateIs(TEXT("Network failure retains readable reason"), EHronoSessionState::Failed);
	Service->FindSessions();
	Backend->TriggerOnFindSessionsCompleteDelegates(false);
	StateIs(TEXT("Search failure permits retry"), EHronoSessionState::Failed);
	Service->FindSessions();
	Backend->Search->SearchResults = { Result(TEXT("C")) };
	Backend->TriggerOnFindSessionsCompleteDelegates(true);
	Backend->bResolve = false;
	Service->JoinSessionByIndex(0);
	Backend->TriggerOnJoinSessionCompleteDelegates(NAME_GameSession, EOnJoinSessionCompleteResult::Success);
	StateIs(TEXT("Missing connect address cleans successful join"), EHronoSessionState::Destroying);
	Backend->Destroyed(); Backend->bResolve = true;
	Backend->TriggerOnSessionUserInviteAcceptedDelegates(true, 7, FUniqueNetIdPtr(), Result(TEXT("other user")));
	TestEqual(TEXT("Invite for another local user ignored"), Backend->Joins, 2);
	Backend->TriggerOnSessionUserInviteAcceptedDelegates(true, 0, FUniqueNetIdPtr(), Old);
	TestEqual(TEXT("Incompatible invite ignored"), Backend->Joins, 2);
	Backend->TriggerOnSessionUserInviteAcceptedDelegates(true, 0, FUniqueNetIdPtr(), Result(TEXT("invited")));
	TestEqual(TEXT("Compatible invite shares canonical join path"), Backend->Joins, 3);
	Backend->TriggerOnJoinSessionCompleteDelegates(NAME_GameSession, EOnJoinSessionCompleteResult::Success);
	StateIs(TEXT("Join success waits for map travel"), EHronoSessionState::Travelling);
	// No disk asset is loaded/saved: this transient world models the loaded destination.
	UWorld* ReadyWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("SessionDestination"), CreatePackage(HronoSessionPolicy::GameplayMap));
	ReadyWorld->SetGameInstance(Scope.Game.Get());
	Service->MapReady(ReadyWorld);
	StateIs(TEXT("Matching destination completes join"), EHronoSessionState::InSession);
	Service->HostSession();
	TestEqual(TEXT("Active session cannot be replaced by create"), Backend->Creates, HostedCreates);
	Backend->TriggerOnSessionUserInviteAcceptedDelegates(true, 0, FUniqueNetIdPtr(), Result(TEXT("another invite")));
	TestEqual(TEXT("Active session cannot be replaced by invite"), Backend->Joins, 3);
	Backend->TriggerOnSessionFailureDelegates(*FUniqueNetIdString::Create(TEXT("local"), TEXT("NULL")), ESessionFailure::ServiceConnectionLost);
	StateIs(TEXT("Backend session loss also starts cleanup"), EHronoSessionState::Destroying);
	Backend->Destroyed();
	TestEqual(TEXT("Backend loss returns to menu"), Service->MenuReturns, 2);
	Service->HostSession();
	Backend->TriggerOnCreateSessionCompleteDelegates(NAME_GameSession, true);
	Backend->TriggerOnStartSessionCompleteDelegates(NAME_GameSession, true);
	Service->MapReady(ReadyWorld);
	Service->LeaveSession(); Backend->Destroyed();
	StateIs(TEXT("Leave clears state"), EHronoSessionState::Idle);
	TestEqual(TEXT("Leave returns to menu"), Service->MenuReturns, 3);
	TestNull(TEXT("No stale named session after leave"), Backend->Named.Get());
	Service->HostSession();
	const int32 TravelsBeforeLoss = Service->Travels;
	Service->ConnectionFailure(Scope.World);
	TestTrue(TEXT("Loss during creation waits for terminal callback"), Service->IsBusy());
	Backend->TriggerOnCreateSessionCompleteDelegates(NAME_GameSession, true);
	StateIs(TEXT("Late create after connection loss is cleaned"), EHronoSessionState::Destroying);
	Backend->Destroyed();
	StateIs(TEXT("Connection loss cleanup permits retry"), EHronoSessionState::Failed);
	TestEqual(TEXT("Aborted creation never travels"), Service->Travels, TravelsBeforeLoss);
	Service->Deinitialize();
	const int32 AfterShutdown = Backend->Joins;
	Backend->TriggerOnSessionUserInviteAcceptedDelegates(true, 0, FUniqueNetIdPtr(), Result(TEXT("after shutdown")));
	TestEqual(TEXT("Deinitialize removes invite delegate"), Backend->Joins, AfterShutdown);

	// Exercise the actual native menu, including empty/default selection and locking.
	TStrongObjectPtr<UAudioMenuTestWidget> Menu(CreateWidget<UAudioMenuTestWidget>(Scope.Game.Get()));
	Menu->TakeWidget(); Menu->ConstructForTest();
	UHronoSessionSubsystem* UIService = Scope.Game->GetSubsystem<UHronoSessionSubsystem>();
	UComboBoxString* Combo = Cast<UComboBoxString>(Menu->GetWidgetFromName(TEXT("SessionCombo")));
	UButton* JoinButton = Cast<UButton>(Menu->GetWidgetFromName(TEXT("JoinSelectedButton")));
	UButton* FindButton = Cast<UButton>(Menu->GetWidgetFromName(TEXT("JoinSessionButton")));
	if (TestNotNull(TEXT("Native host selection widget"), Combo) && TestNotNull(TEXT("Native join selected button"), JoinButton))
	{
		FHronoSessionEntry Entry; Entry.Host = TEXT("UI host"); Entry.SessionId = TEXT("ui_host_id");
		UIService->Entries = {Entry};
		UIService->SetState(EHronoSessionState::ResultsReady, TEXT("Select a host."));
		TestEqual(TEXT("Results visible in combo"), Combo->GetOptionCount(), 1);
		TestEqual(TEXT("First result is never automatically selected"), Combo->GetSelectedIndex(), INDEX_NONE);
		TestFalse(TEXT("Join disabled without selection"), JoinButton->GetIsEnabled());
		Combo->SetSelectedIndex(0);
		TestTrue(TEXT("Explicit selection enables join"), JoinButton->GetIsEnabled());
		UIService->SetState(EHronoSessionState::Searching, TEXT("Searching..."));
		TestFalse(TEXT("Searching disables duplicate find"), FindButton->GetIsEnabled());
		TestFalse(TEXT("Searching disables join"), JoinButton->GetIsEnabled());
		UIService->Entries.Reset();
		UIService->SetState(EHronoSessionState::Failed, TEXT("Try again."));
		TestTrue(TEXT("Failure enables retry in actual menu"), FindButton->GetIsEnabled());
		TestEqual(TEXT("New search clears stale selection"), Combo->GetSelectedIndex(), INDEX_NONE);
		TestFalse(TEXT("Stale selection cannot enable join"), JoinButton->GetIsEnabled());
		UIService->SetState(EHronoSessionState::Idle, TEXT("Ready."));
	}
	Menu->DestructForTest();
	ReadyWorld->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHronoSessionAdmissionTest, "Hrono.Sessions.ServerAdmission",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHronoSessionAdmissionTest::RunTest(const FString& Parameters)
{
	using namespace SessionTests;
	FScope Scope(true);
	AGameModeBase* Mode = Scope.World->GetAuthGameMode();
	if (!TestNotNull(TEXT("Actual configured game mode"), Mode)) return false;
	AHronoGameSession* Session = Cast<AHronoGameSession>(Mode->GameSession);
	if (!TestNotNull(TEXT("GameMode uses authoritative session class"), Session)) return false;
	Session->InitOptions(TEXT("?MaxPlayers=100?MaxSpectators=100"));
	TestEqual(TEXT("URL cannot expand capacity"), Session->MaxPlayers, 2);
	TestTrue(TEXT("Spectators cannot bypass co-op limit"), Session->AtCapacity(true));
	TestFalse(TEXT("Empty server accepts first player"), Session->AtCapacity(false));
	APlayerController* First = Scope.World->SpawnActor<APlayerController>();
	First->PlayerState = Scope.World->SpawnActor<APlayerState>();
	TestFalse(TEXT("Host leaves one slot available"), Session->AtCapacity(false));
	APlayerController* Second = Scope.World->SpawnActor<APlayerController>();
	Second->PlayerState = Scope.World->SpawnActor<APlayerState>();
	TestTrue(TEXT("Two connected players fill server"), Session->AtCapacity(false));
	IConsoleVariable* Override = IConsoleManager::Get().FindConsoleVariable(TEXT("net.MaxPlayersOverride"));
	const int32 Previous = Override->GetInt(); Override->Set(100, ECVF_SetByCode);
	TestTrue(TEXT("Engine testing cvar cannot bypass limit"), Session->AtCapacity(false));
	Override->Set(Previous, ECVF_SetByCode);
	FString Error;
	Mode->PreLogin(TEXT(""), TEXT("127.0.0.1"), FUniqueNetIdRepl(), Error);
	TestFalse(TEXT("Third player rejected in PreLogin"), Error.IsEmpty());
	TestNull(TEXT("Login rechecks capacity before spawning third controller"), Mode->Login(nullptr, ROLE_AutonomousProxy, TEXT(""), TEXT(""), FUniqueNetIdRepl(), Error));
	Second->Destroy();
	TestFalse(TEXT("Disconnect makes a slot available for rejoin"), Session->AtCapacity(false));
	return true;
}
#endif
