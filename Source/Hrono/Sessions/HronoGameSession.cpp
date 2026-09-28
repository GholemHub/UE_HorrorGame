#include "Sessions/HronoGameSession.h"
#include "Sessions/HronoSessionPolicy.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"

AHronoGameSession::AHronoGameSession()
{
	MaxPlayers = HronoSessionPolicy::MaxPlayers;
	MaxSpectators = 0;
}

void AHronoGameSession::InitOptions(const FString& Options)
{
	Super::InitOptions(Options);
	MaxPlayers = HronoSessionPolicy::MaxPlayers;
	MaxSpectators = 0;
}

bool AHronoGameSession::AtCapacity(bool bSpectator)
{
	AGameModeBase* Mode = GetWorld() ? GetWorld()->GetAuthGameMode() : nullptr;
	return bSpectator || !Mode || Mode->GetNumPlayers() >= HronoSessionPolicy::MaxPlayers;
}
