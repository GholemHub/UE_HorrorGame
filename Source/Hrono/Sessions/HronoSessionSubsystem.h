#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "HronoSessionSubsystem.generated.h"

UENUM(BlueprintType)
enum class EHronoSessionState : uint8
{
	Idle, Destroying, Creating, Starting, Searching, ResultsReady, Joining, Travelling, InSession, Failed
};

USTRUCT(BlueprintType)
struct FHronoSessionEntry
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly) FString Host;
	UPROPERTY(BlueprintReadOnly) FString SessionId;
	UPROPERTY(BlueprintReadOnly) int32 Ping = 0;
	UPROPERTY(BlueprintReadOnly) int32 OpenSlots = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHronoSessionChanged);

/** Persistent owner of one serialized session operation and its OSS delegates. */
UCLASS()
class HRONO_API UHronoSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	UFUNCTION(BlueprintCallable, Category="Hrono|Sessions") void HostSession(bool bLAN = false);
	UFUNCTION(BlueprintCallable, Category="Hrono|Sessions") void FindSessions(bool bLAN = false);
	UFUNCTION(BlueprintCallable, Category="Hrono|Sessions") void JoinSessionByIndex(int32 Index);
	UFUNCTION(BlueprintCallable, Category="Hrono|Sessions") void LeaveSession();
	UFUNCTION(BlueprintPure, Category="Hrono|Sessions") bool IsBusy() const;
	/** Reserve the operation while the menu asynchronously preloads content. */
	bool ReserveMenuPreload();
	void ReleaseMenuPreload();
	bool IsMenuPreloading() const { return bMenuPreloadPending; }
	UPROPERTY(BlueprintReadOnly, Category="Hrono|Sessions") EHronoSessionState State = EHronoSessionState::Idle;
	UPROPERTY(BlueprintReadOnly, Category="Hrono|Sessions") FText Status;
	UPROPERTY(BlueprintReadOnly, Category="Hrono|Sessions") TArray<FHronoSessionEntry> Entries;
	UPROPERTY(BlueprintAssignable, Category="Hrono|Sessions") FHronoSessionChanged OnChanged;
	/** Compatibility bridge for old GameInstance Create Session callers. */
	UFUNCTION(BlueprintCallable, meta=(WorldContext="WorldContextObject"), Category="Hrono|Sessions")
	static void RequestHost(UObject* WorldContextObject);

protected:
	/** Native seams allow tests to replace only external backend and map travel. */
	virtual IOnlineSessionPtr ResolveSessions() const;
	virtual int32 ResolveLocalUser() const;
	virtual bool TravelToSession(const FString& Address, bool bHost);
	virtual void ReturnToMenu();
	void HandleInvite(bool bSuccess, int32 LocalUser, FUniqueNetIdPtr User, const FOnlineSessionSearchResult& Result);
	void HandlePostLoadMap(UWorld* World);
	void HandleFailure(UWorld* World, const FString& Message);

private:
	friend class FHronoSessionFlowTest;
	enum class EAfterDestroy : uint8 { None, Host, Find, Join, Leave, Fail };
	IOnlineSessionPtr Sessions;
	IOnlineSessionPtr InviteSessions;
	TSharedPtr<FOnlineSessionSearch> Search;
	TArray<FOnlineSessionSearchResult> Results;
	FOnlineSessionSearchResult PendingJoin;
	FDelegateHandle CreateHandle, StartHandle, FindHandle, JoinHandle, DestroyHandle, InviteHandle;
	FDelegateHandle MapHandle, NetworkHandle, TravelHandle, SessionFailureHandle;
	FTSTicker::FDelegateHandle TickerHandle;
	double OperationStartedAt = 0.0;
	bool bTimedOut = false;
	bool bMenuPreloadPending = false;
	bool TickOperation(float DeltaSeconds);
	EAfterDestroy AfterDestroy = EAfterDestroy::None;
	bool bLANRequest = false;
	bool bReturnAfterFailure = false;
	int32 LocalUserNum = INDEX_NONE;
	bool BeginRequest();
	void SetState(EHronoSessionState NewState, const FString& Message);
	void ClearOperationDelegates();
	void CancelLoading();
	void Fail(const FString& Message, bool bReturn = false);
	void DestroyThen(EAfterDestroy Next);
	void ContinueAfterDestroy();
	void Create();
	void SearchSessions();
	void Join();
	void HandleCreate(FName Name, bool bSuccess);
	void HandleStart(FName Name, bool bSuccess);
	void HandleFind(bool bSuccess);
	void HandleJoin(FName Name, EOnJoinSessionCompleteResult::Type Result);
	void HandleDestroy(FName Name, bool bSuccess);
	void EnsureInviteDelegate();
	FString FailureMessage;
};
