#include "Items/ItemSpawnManagerSystem.h"

#include "Items/Base_Item.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"

#include "Items/Drag_Item.h"

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

	if (const ADrag_Item* DragItemOwner = Cast<ADrag_Item>(Data.Owner.Get()))
	{
		const EItemTimeline DragItemTimeline = DragItemOwner->GetItemTimeline();

		if (DragItemTimeline != EItemTimeline::Both)
		{
			OwnerTimeline = DragItemTimeline;
		}

		UE_LOG(
			LogTemp,
			Log,
			TEXT("[ItemSpawnManager] Owner %s timeline: %s"),
			*GetNameSafe(DragItemOwner),
			*UEnum::GetValueAsString(DragItemTimeline));
	}
	else if (Data.Owner)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[ItemSpawnManager] Owner %s is not ADrag_Item. "
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
		const FString RequiredTimelineText = OwnerTimeline.IsSet()
			? UEnum::GetValueAsString(OwnerTimeline.GetValue())
			: TEXT("Any");

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[ItemSpawnManager] No unused item/timeline pair matches request. "
				"Owner=%s RequiredTimeline=%s"),
			*GetNameSafe(Data.Owner.Get()),
			*RequiredTimelineText);

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
			Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		const bool bAttached = SpawnedItem->AttachToComponent(
			AttachComponent,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			AttachSocketName);

		if (bAttached)
		{
			SpawnedItem->SetActorRelativeTransform(SpawnedItem->HoldOffset);
		}
		else
		{
			UE_LOG(
				LogTemp,
				Error,
				TEXT("[ItemSpawnManager] Failed to attach %s to %s."),
				*GetNameSafe(SpawnedItem),
				*GetNameSafe(AttachComponent));
		}
	}
	else
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[ItemSpawnManager] No attachment component supplied. "
				"Item spawned at request transform: %s"),
			*Data.SpawnTransform.ToHumanReadableString());
	}

	const FName ItemId = GetEffectiveItemId(*Choice.Definition);

	UsedItemTimelines.Add(
		FUsedItemTimeline{ ItemId, Choice.Timeline });

	OutInfo.Item = SpawnedItem;
	OutInfo.ItemId = ItemId;
	OutInfo.Timeline = Choice.Timeline;

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[ItemSpawnManager] Spawned %s in %s timeline for owner %s."),
		*ItemId.ToString(),
		*UEnum::GetValueAsString(Choice.Timeline),
		*GetNameSafe(Data.Owner.Get()));

	return SpawnedItem;
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
