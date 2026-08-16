#include "AI/DemonTargetingLibrary.h"

#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "HronoCharacter.h"

bool UDemonTargetingLibrary::FindClosestPlayerTarget(
	const UObject* WorldContextObject,
	const AActor* Hunter,
	AHronoCharacter*& OutTarget,
	float& OutDistance,
	bool bIgnoreSafePlayers)
{
	OutTarget = nullptr;
	OutDistance = TNumericLimits<float>::Max();

	if (!IsValid(Hunter) || !GEngine)
	{
		return false;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(
		WorldContextObject,
		EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		return false;
	}

	float ClosestDistanceSquared = TNumericLimits<float>::Max();
	TSet<TWeakObjectPtr<AHronoCharacter>> CheckedPlayers;

	const auto ConsiderPlayer = [&](AHronoCharacter* Player)
	{
		if (!IsValid(Player)
			|| Player == Hunter
			|| Player->IsActorBeingDestroyed()
			|| CheckedPlayers.Contains(Player)
			|| (bIgnoreSafePlayers && Player->IsSafeInHidingWardrobe()))
		{
			return;
		}

		CheckedPlayers.Add(Player);
		const float DistanceSquared = FVector::DistSquared(
			Hunter->GetActorLocation(),
			Player->GetActorLocation());
		if (DistanceSquared < ClosestDistanceSquared)
		{
			ClosestDistanceSquared = DistanceSquared;
			OutTarget = Player;
		}
	};

	// PlayerArray is authoritative on the server and scales with player count.
	// It also avoids scanning every actor in the world on each StateTree tick.
	if (const AGameStateBase* GameState = World->GetGameState())
	{
		for (APlayerState* PlayerState : GameState->PlayerArray)
		{
			ConsiderPlayer(PlayerState ? Cast<AHronoCharacter>(PlayerState->GetPawn()) : nullptr);
		}
	}

	// During very early startup a pawn can exist just before PlayerArray is ready.
	// Keep a fallback so the first target acquisition is still reliable.
	if (!OutTarget)
	{
		for (TActorIterator<AHronoCharacter> Iterator(World); Iterator; ++Iterator)
		{
			ConsiderPlayer(*Iterator);
		}
	}

	if (!OutTarget)
	{
		return false;
	}

	OutDistance = FMath::Sqrt(ClosestDistanceSquared);
	return true;
}
