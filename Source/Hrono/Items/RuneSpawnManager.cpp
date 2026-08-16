#include "Items/RuneSpawnManager.h"

#include "HronoCharacter.h"
#include "Items/Base_Item.h"
#include "Items/Rune_Item.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"

ARuneSpawnManager::ARuneSpawnManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

int32 ARuneSpawnManager::SpawnRunesForKilledPlayer(
	AHronoCharacter* KilledPlayer,
	EItemTimeline OriginalTimeline)
{
	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_Client || !HasAuthority())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[RuneSpawner] Spawn ignored on client for %s"), *GetName());
		return 0;
	}

	if (!IsValid(KilledPlayer) || OriginalTimeline == EItemTimeline::Both)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[RuneSpawner] Invalid killed player or OriginalTimeline on %s"), *GetName());
		return 0;
	}

	const double ServerTime = World->GetTimeSeconds();
	if (LastSpawnedForPlayer == KilledPlayer && LastSpawnServerTime >= 0.0 &&
		ServerTime - LastSpawnServerTime < DuplicateSpawnGuardSeconds)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[RuneSpawner] Duplicate death spawn blocked for Player=%s after %.3fs"),
			*GetNameSafe(KilledPlayer), ServerTime - LastSpawnServerTime);
		return 0;
	}

	TArray<TSubclassOf<ARune_Item>> ValidRuneClasses;
	for (const TSubclassOf<ARune_Item> RuneClass : RuneClasses)
	{
		if (RuneClass)
		{
			ValidRuneClasses.Add(RuneClass);
		}
	}

	if (ValidRuneClasses.IsEmpty())
	{
		UE_LOG(LogTemp, Error,
			TEXT("[RuneSpawner] No Rune Classes configured in Details for %s"), *GetName());
		return 0;
	}

	const EItemTimeline SpawnTimeline = KilledPlayer->GetTimeline();
	TArray<ABase_Item*> ValidSpawnPoints;
	for (ABase_Item* SpawnPoint : PossibleSpawnPoints)
	{
		if (IsSpawnPointValidForTimeline(SpawnPoint, SpawnTimeline))
		{
			ValidSpawnPoints.AddUnique(SpawnPoint);
		}
	}

	if (ValidSpawnPoints.IsEmpty())
	{
		UE_LOG(LogTemp, Error,
			TEXT("[RuneSpawner] No valid BP_ItemPointSpawn references for Timeline=%s on %s"),
			*StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(SpawnTimeline)),
			*GetName());
		return 0;
	}

	// Fisher-Yates shuffles make both point selection and rune-class selection random.
	for (int32 Index = ValidSpawnPoints.Num() - 1; Index > 0; --Index)
	{
		ValidSpawnPoints.Swap(Index, FMath::RandRange(0, Index));
	}
	for (int32 Index = ValidRuneClasses.Num() - 1; Index > 0; --Index)
	{
		ValidRuneClasses.Swap(Index, FMath::RandRange(0, Index));
	}

	const int32 RequestedCount = FMath::Max(1, RunesToSpawn);
	const int32 TargetSpawnCount = FMath::Min(RequestedCount, ValidSpawnPoints.Num());
	SpawnedRunes.Reset();

	for (int32 Index = 0; Index < TargetSpawnCount; ++Index)
	{
		ABase_Item* SpawnPoint = ValidSpawnPoints[Index];
		USceneComponent* SpawnComponent = ResolveSpawnComponent(SpawnPoint);
		const FTransform SpawnTransform = SpawnComponent
			? SpawnComponent->GetComponentTransform()
			: SpawnPoint->GetActorTransform();

		// If fewer classes than requested were configured, reuse them only after
		// every configured class has already had a chance to spawn once.
		const TSubclassOf<ARune_Item> RuneClass =
			ValidRuneClasses[Index % ValidRuneClasses.Num()];

		ARune_Item* SpawnedRune = ARune_Item::SpawnRuneForKilledPlayer(
			this,
			RuneClass,
			SpawnTransform,
			KilledPlayer,
			OriginalTimeline);

		if (!IsValid(SpawnedRune))
		{
			continue;
		}

		SpawnedRune->SetItemTimeline(SpawnTimeline);

		if (SpawnComponent)
		{
			TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents(SpawnedRune);
			for (UPrimitiveComponent* Primitive : PrimitiveComponents)
			{
				if (IsValid(Primitive))
				{
					Primitive->SetSimulatePhysics(false);
					Primitive->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
				}
			}

			SpawnedRune->AttachToComponent(
				SpawnComponent,
				FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		}

		SpawnedRunes.Add(SpawnedRune);
		UE_LOG(LogTemp, Log,
			TEXT("[RuneSpawner] Spawned Rune=%s RuneId=%s Point=%s Timeline=%s"),
			*GetNameSafe(SpawnedRune),
			*SpawnedRune->RuneId.ToString(),
			*GetNameSafe(SpawnPoint),
			*StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(SpawnTimeline)));
	}

	if (SpawnedRunes.Num() > 0)
	{
		LastSpawnedForPlayer = KilledPlayer;
		LastSpawnServerTime = ServerTime;
	}

	UE_LOG(LogTemp, Warning,
		TEXT("[RuneSpawner] Finished: spawned %d/%d runes using %d valid random points"),
		SpawnedRunes.Num(), RequestedCount, ValidSpawnPoints.Num());

	return SpawnedRunes.Num();
}

USceneComponent* ARuneSpawnManager::ResolveSpawnComponent(const ABase_Item* SpawnPoint) const
{
	if (!IsValid(SpawnPoint))
	{
		return nullptr;
	}

	TArray<USceneComponent*> SceneComponents;
	SpawnPoint->GetComponents(SceneComponents);
	for (USceneComponent* SceneComponent : SceneComponents)
	{
		if (!IsValid(SceneComponent))
		{
			continue;
		}

		const FName ComponentName = SceneComponent->GetFName();
		if (ComponentName == TEXT("PointSetComponent") || ComponentName == TEXT("PointSet"))
		{
			return SceneComponent;
		}
	}

	return SpawnPoint->GetRootComponent();
}

bool ARuneSpawnManager::IsSpawnPointValidForTimeline(
	const ABase_Item* SpawnPoint,
	EItemTimeline TargetTimeline) const
{
	if (!IsValid(SpawnPoint))
	{
		return false;
	}

	return !bRespectSpawnPointTimeline ||
		SpawnPoint->ItemTimeline == EItemTimeline::Both ||
		SpawnPoint->ItemTimeline == TargetTimeline;
}
