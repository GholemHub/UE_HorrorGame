#pragma once
#include "Sessions/HronoSessionSubsystem.h"
#include "SessionTestSubsystem.generated.h"

/** Replace only the external service and travel; keep the production state machine. */
UCLASS(Transient, NotBlueprintable)
class USessionTestSubsystem : public UHronoSessionSubsystem
{
	GENERATED_BODY()
public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return false; }
	IOnlineSessionPtr Backend;
	int32 Travels = 0, MenuReturns = 0;
	bool bAllowTravel = true;
	FString LastAddress;
	void MapReady(UWorld* World) { HandlePostLoadMap(World); }
	void ConnectionFailure(UWorld* World) { HandleFailure(World, TEXT("test disconnect")); }
protected:
	virtual IOnlineSessionPtr ResolveSessions() const override { return Backend; }
	virtual int32 ResolveLocalUser() const override { return 0; }
	virtual bool TravelToSession(const FString& Address, bool bHost) override
	{
		++Travels; LastAddress = Address; return bAllowTravel;
	}
	virtual void ReturnToMenu() override { ++MenuReturns; }
};
