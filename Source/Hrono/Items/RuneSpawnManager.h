#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HronoSharedTools.h"
#include "RuneSpawnManager.generated.h"

class ABase_Item;
class AHronoCharacter;
class ARune_Item;
class USceneComponent;

/**
 * Level actor that spawns ritual runes at random BP_ItemPointSpawn actors.
 * Configure rune classes and placed spawn-point references in the Details panel,
 * then call SpawnRunesForKilledPlayer from the authoritative death event.
 */
UCLASS(BlueprintType, Blueprintable)
class HRONO_API ARuneSpawnManager : public AActor
{
	GENERATED_BODY()

public:
	ARuneSpawnManager();

	/** Rune Blueprint classes eligible for this spawn pass. Two are selected randomly. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rune Spawn|Setup")
	TArray<TSubclassOf<ARune_Item>> RuneClasses;

	/** BP_ItemPointSpawn actors placed in the level. A point is never selected twice per pass. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Rune Spawn|Setup")
	TArray<TObjectPtr<ABase_Item>> PossibleSpawnPoints;

	/** Number of runes created per death. Defaults to the requested two runes. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rune Spawn|Setup",
		meta = (ClampMin = "1", UIMin = "1"))
	int32 RunesToSpawn = 2;

	/** Only use Both/current-timeline points so the surviving player can reach every rune. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rune Spawn|Timeline")
	bool bRespectSpawnPointTimeline = true;

	/** Blocks duplicate server death notifications received within this interval. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rune Spawn|Networking",
		meta = (ClampMin = "0.0", Units = "s"))
	float DuplicateSpawnGuardSeconds = 0.25f;

	/** Runes created by the most recent successful spawn pass. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Rune Spawn|Runtime")
	TArray<TObjectPtr<ARune_Item>> SpawnedRunes;

	/**
	 * Randomly chooses unique configured points and spawns the configured number of runes.
	 * OriginalTimeline must be captured before Switch Player Timeline is called.
	 * Call this after the killed player has joined the survivor's timeline.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Rune Spawn",
		meta = (DisplayName = "Spawn Runes For Killed Player"))
	int32 SpawnRunesForKilledPlayer(
		AHronoCharacter* KilledPlayer,
		EItemTimeline OriginalTimeline);

private:
	USceneComponent* ResolveSpawnComponent(const ABase_Item* SpawnPoint) const;
	bool IsSpawnPointValidForTimeline(const ABase_Item* SpawnPoint, EItemTimeline TargetTimeline) const;

	double LastSpawnServerTime = -1.0;
	TWeakObjectPtr<AHronoCharacter> LastSpawnedForPlayer;
};
