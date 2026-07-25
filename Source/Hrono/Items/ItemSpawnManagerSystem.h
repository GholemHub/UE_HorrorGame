#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HronoSharedTools.h"
#include "ItemSpawnManagerSystem.generated.h"

class ABase_Item;
class USceneComponent;

/**
 * One item type the spawn manager may select. Use the same ItemId for two
 * different classes that represent the same logical item (for example
 * BP_Key_Past and BP_Key_Future).
 */
USTRUCT(BlueprintType)
struct FItemSpawnDefinition
{
	GENERATED_BODY()

	/** Logical identity used when preventing duplicate timeline versions. Empty uses the class name. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Spawn")
	FName ItemId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Spawn")
	TSubclassOf<ABase_Item> ItemClass;

	/** The timeline versions this item is allowed to spawn in. Both is intentionally not supported here. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Spawn")
	TArray<EItemTimeline> SupportedTimelines = { EItemTimeline::Past, EItemTimeline::Future };
};

/** Parameters supplied by the actor requesting an item. Empty filters mean "any registered item/timeline". */
USTRUCT(BlueprintType)
struct FSpawnRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Spawn")
	FTransform SpawnTransform = FTransform::Identity;

	/** Restricts selection to registered classes deriving from one of these classes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Spawn")
	TArray<TSubclassOf<ABase_Item>> AllowedItemClasses;

	/** Restricts selection to these timelines. Empty permits Past and Future. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Spawn")
	TArray<EItemTimeline> AllowedTimelines;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Spawn")
	TObjectPtr<AActor> Owner = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Spawn")
	TObjectPtr<APawn> Instigator = nullptr;

	/**
	 * Optional component to attach the spawned item to. If empty, a PointSet on
	 * the request owner is preferred, followed by the manager's PointSetComponent.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Spawn")
	TObjectPtr<USceneComponent> AttachToComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Spawn")
	FName AttachSocketName = NAME_None;
};

/** Describes the item/timeline pair selected by the manager for the last successful spawn. */
USTRUCT(BlueprintType)
struct FSpawnedItemInfo
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Spawn")
	TObjectPtr<ABase_Item> Item = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Spawn")
	FName ItemId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Spawn")
	EItemTimeline Timeline = EItemTimeline::Past;
};

/** Place this actor in the level or create a Blueprint child to control item spawning from Blueprints. */
UCLASS(BlueprintType, Blueprintable)
class HRONO_API AItemSpawnManagerSystem : public AActor
{
	GENERATED_BODY()

public:
	AItemSpawnManagerSystem();

protected:
	virtual void BeginPlay() override;

public:
	/** Adds or replaces a definition. Call during game setup, before requesting spawns. */
	UFUNCTION(BlueprintCallable, Category = "Item Spawn")
	bool RegisterAvailableItem(const FItemSpawnDefinition& Definition);

	/**
	 * Selects an unused ItemId/timeline pair, spawns it on the server, and returns it.
	 * Returns nullptr when called on a client or when no eligible pair remains.
	 */
	UFUNCTION(BlueprintCallable, Category = "Item Spawn")
	ABase_Item* SpawnItem(const FSpawnRequest& Data);

	/**
	 * Same spawn behavior as SpawnItem, but also reports the logical item id and timeline selected.
	 */
	UFUNCTION(BlueprintCallable, Category = "Item Spawn")
	ABase_Item* SpawnItemWithInfo(const FSpawnRequest& Data, FSpawnedItemInfo& OutInfo);

	/**
	 * Randomly selects from a mixed array of shelves and dedicated item points,
	 * then spawns at most one item at each selected location. A value <= 0 for
	 * ItemCount attempts to fill every supplied location. The location becomes
	 * the request owner, so its ItemTimeline restricts the item timeline.
	 */
	UFUNCTION(BlueprintCallable, Category = "Item Spawn")
	int32 SpawnItemsAtRandomLocations(
		const TArray<AActor*>& PossibleLocations,
		int32 ItemCount,
		const FSpawnRequest& BaseRequest,
		TArray<FSpawnedItemInfo>& OutSpawnedItems);

	/** Clears the history, allowing all registered timeline versions to be spawned again. */
	UFUNCTION(BlueprintCallable, Category = "Item Spawn")
	void ResetSpawnHistory();

	UFUNCTION(BlueprintPure, Category = "Item Spawn")
	bool HasSpawnedItemTimeline(FName ItemId, EItemTimeline Timeline) const;

	/** Definitions copied into AvailableItems on BeginPlay. Configure these on BP_ItemSpawnManagerSystem. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Spawn")
	TArray<FItemSpawnDefinition> DefaultAvailableItems;

	/**
	 * All shelves and dedicated item-point actors that may receive an item when
	 * the manager starts. Both location types derive from ABase_Item.
	 */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Item Spawn|Automatic")
	TArray<TObjectPtr<ABase_Item>> PossibleSpawnLocations;

	/** Automatically choose random entries from PossibleSpawnLocations on BeginPlay. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Spawn|Automatic")
	bool bSpawnItemsOnBeginPlay = true;

	/** Number of locations to fill automatically. A value <= 0 attempts to fill every location. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Spawn|Automatic", meta = (ClampMin = "0", UIMin = "0"))
	int32 BeginPlayItemCount = 0;

	/** Optional class and timeline filters used by the automatic BeginPlay spawn. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Spawn|Automatic")
	FSpawnRequest BeginPlaySpawnRequest;

	/** Successful automatic spawns from the most recent BeginPlay spawn pass. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Item Spawn|Automatic")
	TArray<FSpawnedItemInfo> BeginPlaySpawnedItems;

	/** Display the BeginPlay spawn summary and spawned item names in the game viewport. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Spawn|Debug")
	bool bShowSpawnDebugOnScreen = true;

	/** How long the BeginPlay spawn diagnostics remain visible. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Spawn|Debug",
		meta = (ClampMin = "0.1", UIMin = "0.1", Units = "s"))
	float SpawnDebugMessageDuration = 15.0f;

	/** Runtime registered item definitions. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Item Spawn")
	TArray<FItemSpawnDefinition> AvailableItems;

	/** Fallback attachment point when neither the request nor its owner supplies one. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Spawn")
	TObjectPtr<USceneComponent> PointSetComponent = nullptr;

private:
	struct FUsedItemTimeline
	{
		FName ItemId;
		EItemTimeline Timeline = EItemTimeline::Past;

		bool operator==(const FUsedItemTimeline& Other) const
		{
			return ItemId == Other.ItemId && Timeline == Other.Timeline;
		}

		friend uint32 GetTypeHash(const FUsedItemTimeline& Value)
		{
			return HashCombine(GetTypeHash(Value.ItemId), GetTypeHash(static_cast<uint8>(Value.Timeline)));
		}
	};

	FName GetEffectiveItemId(const FItemSpawnDefinition& Definition) const;
	bool IsDefinitionAllowedByRequest(const FItemSpawnDefinition& Definition, const FSpawnRequest& Request) const;
	bool IsTimelineAllowedByRequest(EItemTimeline Timeline, const FSpawnRequest& Request) const;
	USceneComponent* ResolveAttachComponent(const FSpawnRequest& Request) const;
	void ShowBeginPlaySpawnDebug(int32 TargetCount, int32 ValidLocationCount) const;

	TSet<FUsedItemTimeline> UsedItemTimelines;
};
