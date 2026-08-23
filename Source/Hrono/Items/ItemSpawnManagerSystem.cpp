#include "Items/ItemSpawnManagerSystem.h"

#include "Items/Base_Item.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"

namespace
{
	const FName ItemSpawnPointTag(TEXT("ItemSpawnPoint"));

	bool IsItemSpawnPointComponent(const USceneComponent* Component)
	{
		if (!Component)
		{
			return false;
		}

		if (Component->ComponentHasTag(ItemSpawnPointTag))
		{
			return true;
		}

		const FString ComponentName = Component->GetName();
		return ComponentName == TEXT("PointSet")
			|| ComponentName == TEXT("PointSetComponent")
			|| ComponentName.EndsWith(TEXT("PointSet"));
	}

	void GatherItemSpawnPointComponents(
		const AActor* Actor,
		TArray<USceneComponent*>& OutComponents)
	{
		OutComponents.Reset();
		if (!Actor)
		{
			return;
		}

		TArray<USceneComponent*> SceneComponents;
		Actor->GetComponents(SceneComponents);
		for (USceneComponent* SceneComponent : SceneComponents)
		{
			if (IsItemSpawnPointComponent(SceneComponent))
			{
				OutComponents.AddUnique(SceneComponent);
			}
		}
	}
}

AItemSpawnManagerSystem::AItemSpawnManagerSystem()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AItemSpawnManagerSystem::BeginPlay()
{
	Super::BeginPlay();

	AvailableItems.Reset();
	for (const FItemSpawnDefinition& Definition : DefaultAvailableItems)
	{
		RegisterAvailableItem(Definition);
	}

	BeginPlaySpawnedItems.Reset();

	if (!bSpawnItemsOnBeginPlay)
	{
		return;
	}

	if (!HasAuthority())
	{
		return;
	}

	if (AvailableItems.IsEmpty())
	{
		return;
	}

	TArray<AActor*> SpawnLocations;
	SpawnLocations.Reserve(PossibleSpawnLocations.Num());

	for (ABase_Item* Location : PossibleSpawnLocations)
	{
		if (IsValid(Location))
		{
			SpawnLocations.Add(Location);
		}
	}

	if (SpawnLocations.IsEmpty())
	{
		return;
	}

	SpawnItemsAtRandomLocations(
		SpawnLocations,
		BeginPlayItemCount,
		BeginPlaySpawnRequest,
		BeginPlaySpawnedItems);
}

bool AItemSpawnManagerSystem::RegisterAvailableItem(const FItemSpawnDefinition& Definition)
{
	if (!Definition.ItemClass)
	{
		return false;
	}

	const FName ItemId = GetEffectiveItemId(Definition);
	for (FItemSpawnDefinition& Existing : AvailableItems)
	{
		if (GetEffectiveItemId(Existing) == ItemId && Existing.ItemClass == Definition.ItemClass)
		{
			Existing = Definition;
			return true;
		}
	}

	AvailableItems.Add(Definition);
	return true;
}

ABase_Item* AItemSpawnManagerSystem::SpawnItem(const FSpawnRequest& Data)
{
	FSpawnedItemInfo SpawnedInfo;
	return SpawnItemWithInfo(Data, SpawnedInfo);
}

ABase_Item* AItemSpawnManagerSystem::SpawnItemWithInfo(
	const FSpawnRequest& Data,
	FSpawnedItemInfo& OutInfo)
{
	UWorld* World = GetWorld();
	if (!World || !HasAuthority())
	{
		OutInfo = FSpawnedItemInfo();
		return nullptr;
	}

	/*
	 * Determine the required timeline from the actor requesting the spawn.
	 *
	 * Past owner   -> spawn only Past item.
	 * Future owner -> spawn only Future item.
	 * Both/unknown -> don't add an owner restriction.
	 */
	TOptional<EItemTimeline> OwnerTimeline;

	// Every supported spawn location derives from ABase_Item: shelves derive
	// from ADrag_Item, while dedicated BP_ItemPointSpawn actors derive directly
	// from ABase_Item. Use the common timeline property so both kinds of location
	// constrain the random item selection in exactly the same way.
	if (const ABase_Item* TimelineOwner = Cast<ABase_Item>(Data.Owner.Get()))
	{
		const EItemTimeline LocationTimeline = TimelineOwner->ItemTimeline;

		if (LocationTimeline != EItemTimeline::Both)
		{
			OwnerTimeline = LocationTimeline;
		}

	}

	struct FCandidate
	{
		const FItemSpawnDefinition* Definition = nullptr;
		EItemTimeline Timeline = EItemTimeline::Both;
	};

	TArray<FCandidate> Candidates;

	for (const FItemSpawnDefinition& Definition : AvailableItems)
	{
		if (!Definition.ItemClass ||
			!IsDefinitionAllowedByRequest(Definition, Data))
		{
			continue;
		}

		const FName ItemId = GetEffectiveItemId(Definition);

		for (const EItemTimeline Timeline : Definition.SupportedTimelines)
		{
			// "Both" isn't a concrete spawn timeline.
			if (Timeline == EItemTimeline::Both)
			{
				continue;
			}

			// Respect timelines explicitly requested by the caller.
			if (!IsTimelineAllowedByRequest(Timeline, Data))
			{
				continue;
			}

			// Respect the timeline of the shelf/Drag_Item owner.
			if (OwnerTimeline.IsSet() && Timeline != OwnerTimeline.GetValue())
			{
				continue;
			}

			const FUsedItemTimeline UsedKey{ ItemId, Timeline };

			if (!UsedItemTimelines.Contains(UsedKey))
			{
				Candidates.Add({ &Definition, Timeline });
			}
		}
	}

	if (Candidates.IsEmpty())
	{
		OutInfo = FSpawnedItemInfo();
		return nullptr;
	}

	const FCandidate& Choice =
		Candidates[FMath::RandRange(0, Candidates.Num() - 1)];

	AActor* SpawnOwner = Data.Owner
		? Data.Owner.Get()
		: this;

	USceneComponent* AttachComponent = ResolveAttachComponent(Data);
	const FName AttachSocketName = Data.AttachSocketName;

	FTransform InitialSpawnTransform = Data.SpawnTransform;

	if (IsValid(AttachComponent))
	{
		InitialSpawnTransform = AttachSocketName.IsNone()
			? AttachComponent->GetComponentTransform()
			: AttachComponent->GetSocketTransform(
				AttachSocketName,
				ERelativeTransformSpace::RTS_World);
	}

	ABase_Item* SpawnedItem =
		World->SpawnActorDeferred<ABase_Item>(
			Choice.Definition->ItemClass,
			InitialSpawnTransform,
			SpawnOwner,
			Data.Instigator,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (!IsValid(SpawnedItem))
	{
		OutInfo = FSpawnedItemInfo();
		return nullptr;
	}

	SpawnedItem->SetItemTimeline(Choice.Timeline);
	SpawnedItem->FinishSpawning(InitialSpawnTransform);

	if (IsValid(AttachComponent))
	{
		if (UStaticMeshComponent* Mesh = SpawnedItem->GetItemMesh())
		{
			Mesh->SetSimulatePhysics(false);
			// Items attached to a spawn point must remain queryable so the
			// authoritative listen-server player can hit them with an interaction
			// trace. Component collision state is not replicated, so disabling it
			// here only disabled pickup on the server while clients retained the
			// Blueprint's default collision state.
			Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}

		SpawnedItem->AttachToComponent(
			AttachComponent,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			AttachSocketName);
	}
	const FName ItemId = GetEffectiveItemId(*Choice.Definition);

	UsedItemTimelines.Add(
		FUsedItemTimeline{ ItemId, Choice.Timeline });

	OutInfo.Item = SpawnedItem;
	OutInfo.ItemId = ItemId;
	OutInfo.Timeline = Choice.Timeline;

	return SpawnedItem;
}

int32 AItemSpawnManagerSystem::SpawnItemsAtRandomLocations(
	const TArray<AActor*>& PossibleLocations,
	int32 ItemCount,
	const FSpawnRequest& BaseRequest,
	TArray<FSpawnedItemInfo>& OutSpawnedItems)
{
	OutSpawnedItems.Reset();

	struct FSpawnLocationSlot
	{
		AActor* Owner = nullptr;
		USceneComponent* AttachComponent = nullptr;
	};

	TArray<FSpawnLocationSlot> ShuffledSlots;
	TSet<AActor*> SeenActors;

	for (AActor* Location : PossibleLocations)
	{
		if (!IsValid(Location)
			|| !Cast<ABase_Item>(Location)
			|| SeenActors.Contains(Location))
		{
			continue;
		}

		SeenActors.Add(Location);

		TArray<USceneComponent*> PointSets;
		GatherItemSpawnPointComponents(Location, PointSets);
		if (PointSets.IsEmpty())
		{
			ShuffledSlots.Add({ Location, nullptr });
			continue;
		}

		for (USceneComponent* PointSet : PointSets)
		{
			ShuffledSlots.Add({ Location, PointSet });
		}
	}

	for (int32 Index = ShuffledSlots.Num() - 1; Index > 0; --Index)
	{
		const int32 SwapIndex = FMath::RandRange(0, Index);
		ShuffledSlots.Swap(Index, SwapIndex);
	}

	const int32 TargetCount = ItemCount <= 0
		? ShuffledSlots.Num()
		: FMath::Min(ItemCount, ShuffledSlots.Num());

	for (const FSpawnLocationSlot& Slot : ShuffledSlots)
	{
		if (OutSpawnedItems.Num() >= TargetCount)
		{
			break;
		}

		FSpawnRequest LocationRequest = BaseRequest;
		LocationRequest.Owner = Slot.Owner;
		LocationRequest.SpawnTransform = Slot.AttachComponent
			? Slot.AttachComponent->GetComponentTransform()
			: Slot.Owner->GetActorTransform();
		LocationRequest.AttachToComponent = Slot.AttachComponent;
		LocationRequest.AttachSocketName = NAME_None;

		FSpawnedItemInfo SpawnedInfo;
		if (SpawnItemWithInfo(LocationRequest, SpawnedInfo))
		{
			OutSpawnedItems.Add(SpawnedInfo);
		}
	}

	return OutSpawnedItems.Num();
}

void AItemSpawnManagerSystem::ResetSpawnHistory()
{
	UsedItemTimelines.Empty();
}

bool AItemSpawnManagerSystem::HasSpawnedItemTimeline(FName ItemId, EItemTimeline Timeline) const
{
	return UsedItemTimelines.Contains(FUsedItemTimeline{ ItemId, Timeline });
}

FName AItemSpawnManagerSystem::GetEffectiveItemId(const FItemSpawnDefinition& Definition) const
{
	return !Definition.ItemId.IsNone() ? Definition.ItemId : Definition.ItemClass->GetFName();
}

bool AItemSpawnManagerSystem::IsDefinitionAllowedByRequest(const FItemSpawnDefinition& Definition, const FSpawnRequest& Request) const
{
	if (Request.AllowedItemClasses.IsEmpty())
	{
		return true;
	}

	for (const TSubclassOf<ABase_Item> AllowedClass : Request.AllowedItemClasses)
	{
		if (AllowedClass && Definition.ItemClass->IsChildOf(AllowedClass.Get()))
		{
			return true;
		}
	}

	return false;
}

bool AItemSpawnManagerSystem::IsTimelineAllowedByRequest(EItemTimeline Timeline, const FSpawnRequest& Request) const
{
	return Request.AllowedTimelines.IsEmpty() || Request.AllowedTimelines.Contains(Timeline);
}

USceneComponent* AItemSpawnManagerSystem::ResolveAttachComponent(const FSpawnRequest& Request) const
{
	if (Request.AttachToComponent)
	{
		return Request.AttachToComponent.Get();
	}

	auto FindPointSetComponent = [](const AActor* Actor) -> USceneComponent*
	{
		TArray<USceneComponent*> PointSets;
		GatherItemSpawnPointComponents(Actor, PointSets);
		return PointSets.IsEmpty() ? nullptr : PointSets[0];
	};

	if (USceneComponent* OwnerPointSet = FindPointSetComponent(Request.Owner.Get()))
	{
		return OwnerPointSet;
	}

	if (PointSetComponent)
	{
		return PointSetComponent.Get();
	}

	if (USceneComponent* ManagerPointSet = FindPointSetComponent(this))
	{
		return ManagerPointSet;
	}

	TArray<USceneComponent*> SceneComponents;
	GetComponents(SceneComponents);
	for (USceneComponent* SceneComponent : SceneComponents)
	{
		if (SceneComponent && SceneComponent->GetFName() == TEXT("PointSetComponent"))
		{
			return SceneComponent;
		}
	}

	return nullptr;
}
