#include "Items/ItemSpawnManagerSystem.h"

#include "Items/Base_Item.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

namespace
{
	USceneComponent* FindChildSceneComponentByName(USceneComponent* Parent, FName ComponentName)
	{
		if (!Parent || ComponentName.IsNone())
		{
			return nullptr;
		}

		TArray<USceneComponent*> ChildComponents;
		Parent->GetChildrenComponents(true, ChildComponents);
		for (USceneComponent* ChildComponent : ChildComponents)
		{
			if (ChildComponent && ChildComponent->GetFName() == ComponentName)
			{
				return ChildComponent;
			}
		}

		return nullptr;
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
		ShowBeginPlaySpawnDebug(0, 0);
		return;
	}

	if (!HasAuthority())
	{
		return;
	}

	if (AvailableItems.IsEmpty())
	{
		ShowBeginPlaySpawnDebug(0, 0);
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
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[ItemSpawnManager] Automatic spawn skipped because PossibleSpawnLocations is empty."));
		ShowBeginPlaySpawnDebug(0, 0);
		return;
	}

	const int32 TargetCount = BeginPlayItemCount <= 0
		? SpawnLocations.Num()
		: FMath::Min(BeginPlayItemCount, SpawnLocations.Num());

	SpawnItemsAtRandomLocations(
		SpawnLocations,
		BeginPlayItemCount,
		BeginPlaySpawnRequest,
		BeginPlaySpawnedItems);

	ShowBeginPlaySpawnDebug(TargetCount, SpawnLocations.Num());
}

void AItemSpawnManagerSystem::ShowBeginPlaySpawnDebug(
	int32 TargetCount,
	int32 ValidLocationCount) const
{
	if (!bShowSpawnDebugOnScreen || !GEngine)
	{
		return;
	}

	FString Message;
	if (!bSpawnItemsOnBeginPlay)
	{
		Message = TEXT("[ItemSpawnManager] Automatic spawning is disabled.");
	}
	else
	{
		Message = FString::Printf(
			TEXT("[ItemSpawnManager] Spawned %d / %d item(s) | Valid locations: %d / %d"),
			BeginPlaySpawnedItems.Num(),
			TargetCount,
			ValidLocationCount,
			PossibleSpawnLocations.Num());

		for (int32 Index = 0; Index < BeginPlaySpawnedItems.Num(); ++Index)
		{
			const FSpawnedItemInfo& Info = BeginPlaySpawnedItems[Index];
			Message += FString::Printf(
				TEXT("\n  %d. %s (Actor: %s, Timeline: %s)"),
				Index + 1,
				*Info.ItemId.ToString(),
				*GetNameSafe(Info.Item.Get()),
				*UEnum::GetValueAsString(Info.Timeline));
		}

		if (BeginPlaySpawnedItems.Num() < TargetCount)
		{
			Message += TEXT("\n  WARNING: Target was not reached. Check the Output Log for rejected locations or exhausted item/timeline pairs.");
		}
	}

	const FColor MessageColor =
		BeginPlaySpawnedItems.Num() < TargetCount ? FColor::Yellow : FColor::Green;

	GEngine->AddOnScreenDebugMessage(
		-1,
		SpawnDebugMessageDuration,
		MessageColor,
		Message);
}

bool AItemSpawnManagerSystem::RegisterAvailableItem(const FItemSpawnDefinition& Definition)
{
	if (!Definition.ItemClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ItemSpawnManager] Cannot register an empty item class."));
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
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[ItemSpawnManager] SpawnItem must be called on the server."));

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
	else if (Data.Owner)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[ItemSpawnManager] Owner %s is not ABase_Item. "
				"Owner timeline restriction will not be applied."),
			*GetNameSafe(Data.Owner.Get()));
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
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[ItemSpawnManager] Failed to spawn %s."),
			*GetNameSafe(Choice.Definition->ItemClass));

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

		const bool bAttached = SpawnedItem->AttachToComponent(
			AttachComponent,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			AttachSocketName);

		if (!bAttached)
		{
			UE_LOG(
				LogTemp,
				Error,
				TEXT("[ItemSpawnManager] Failed to attach %s to %s."),
				*GetNameSafe(SpawnedItem),
				*GetNameSafe(AttachComponent));
		}
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

	TArray<AActor*> ShuffledLocations;
	ShuffledLocations.Reserve(PossibleLocations.Num());

	for (AActor* Location : PossibleLocations)
	{
		if (IsValid(Location) && Cast<ABase_Item>(Location))
		{
			ShuffledLocations.AddUnique(Location);
		}
		else if (Location)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[ItemSpawnManager] Ignoring location %s because it is not ABase_Item."),
				*GetNameSafe(Location));
		}
	}

	for (int32 Index = ShuffledLocations.Num() - 1; Index > 0; --Index)
	{
		const int32 SwapIndex = FMath::RandRange(0, Index);
		ShuffledLocations.Swap(Index, SwapIndex);
	}

	const int32 TargetCount = ItemCount <= 0
		? ShuffledLocations.Num()
		: FMath::Min(ItemCount, ShuffledLocations.Num());

	for (AActor* Location : ShuffledLocations)
	{
		if (OutSpawnedItems.Num() >= TargetCount)
		{
			break;
		}

		FSpawnRequest LocationRequest = BaseRequest;
		LocationRequest.Owner = Location;
		LocationRequest.SpawnTransform = Location->GetActorTransform();
		LocationRequest.AttachToComponent = nullptr;
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
		if (!Actor)
		{
			return nullptr;
		}

		TArray<USceneComponent*> SceneComponents;
		Actor->GetComponents(SceneComponents);
		for (USceneComponent* SceneComponent : SceneComponents)
		{
			if (!SceneComponent)
			{
				continue;
			}

			const FName ComponentName = SceneComponent->GetFName();
			if (ComponentName == TEXT("PointSetComponent") || ComponentName == TEXT("PointSet"))
			{
				return SceneComponent;
			}
		}

		return nullptr;
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
