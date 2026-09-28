#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameSession.h"
#include "HronoGameSession.generated.h"

/** Enforce the co-op limit even for direct IP login and URL/cvar overrides. */
UCLASS()
class HRONO_API AHronoGameSession : public AGameSession
{
	GENERATED_BODY()
public:
	AHronoGameSession();
	virtual void InitOptions(const FString& Options) override;
	virtual bool AtCapacity(bool bSpectator) override;
};
