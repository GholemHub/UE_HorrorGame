#include "Items/Rune_Item.h"

#include "HronoCharacter.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ARune_Item::ARune_Item()
{
	ItemType = EItemType::Rune;
	ItemName = NSLOCTEXT("HronoItems", "RuneItemName", "Rune");
	ItemDescription = NSLOCTEXT("HronoItems", "RuneItemDescription", "A ritual rune used by the pentagram.");
}

void ARune_Item::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARune_Item, RuneId);
	DOREPLIFETIME(ARune_Item, RitualTargetPlayer);
	DOREPLIFETIME(ARune_Item, TimelineToRestore);
	DOREPLIFETIME(ARune_Item, bPlacedInPentagram);
	DOREPLIFETIME(ARune_Item, PlacedSlotId);
	DOREPLIFETIME(ARune_Item, PlacedPentagram);
	DOREPLIFETIME(ARune_Item, bRitualCompleted);
}

void ARune_Item::OnRep_OwningCharacter()
{
	Super::OnRep_OwningCharacter();

	// OwningCharacter and placement state are independent replicated fields.
	// Whichever RepNotify arrives last must leave a placed rune locked in its slot.
	if (bPlacedInPentagram)
	{
		ApplyPlacedState();
	}
}

void ARune_Item::InitializeForKilledPlayer(AHronoCharacter* KilledPlayer, EItemTimeline OriginalTimeline)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[RuneRitual] Initialize ignored without server authority: %s"), *GetName());
		return;
	}

	if (!IsValid(KilledPlayer) || OriginalTimeline == EItemTimeline::Both)
	{
		UE_LOG(LogTemp, Error, TEXT("[RuneRitual] Invalid ritual target or original timeline for %s"), *GetName());
		return;
	}

	RitualTargetPlayer = KilledPlayer;
	TimelineToRestore = OriginalTimeline;
	ForceNetUpdate();

	UE_LOG(LogTemp, Log, TEXT("[RuneRitual] %s initialized for Player=%s ReturnTimeline=%s RuneId=%s"),
		*GetName(),
		*GetNameSafe(KilledPlayer),
		*StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(TimelineToRestore)),
		*RuneId.ToString());
}

ARune_Item* ARune_Item::SpawnRuneForKilledPlayer(
	const UObject* WorldContextObject,
	TSubclassOf<ARune_Item> RuneClass,
	const FTransform& SpawnTransform,
	AHronoCharacter* KilledPlayer,
	EItemTimeline OriginalTimeline)
{
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	if (!World || World->GetNetMode() == NM_Client || !RuneClass || !IsValid(KilledPlayer))
	{
		UE_LOG(LogTemp, Error, TEXT("[RuneRitual] Spawn Rune failed: call it on the server with a valid class and player"));
		return nullptr;
	}

	ARune_Item* Rune = World->SpawnActorDeferred<ARune_Item>(
		RuneClass,
		SpawnTransform,
		nullptr,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

	if (!Rune)
	{
		return nullptr;
	}

	Rune->RitualTargetPlayer = KilledPlayer;
	Rune->TimelineToRestore = OriginalTimeline;
	// After a death both players are in the same timeline. Spawn the rune there,
	// while TimelineToRestore keeps the victim's actual destination.
	Rune->ItemTimeline = KilledPlayer->GetTimeline();
	UGameplayStatics::FinishSpawningActor(Rune, SpawnTransform);

	UE_LOG(LogTemp, Log, TEXT("[RuneRitual] Spawned %s for Player=%s ReturnTimeline=%s"),
		*GetNameSafe(Rune),
		*GetNameSafe(KilledPlayer),
		*StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(OriginalTimeline)));
	return Rune;
}

bool ARune_Item::CanPlaceInRuneSlot(FName RequiredRuneId) const
{
	return !bPlacedInPentagram && !RequiredRuneId.IsNone() && RuneId == RequiredRuneId;
}

void ARune_Item::PlaceRuneInPentagram(
	AActor* Pentagram,
	FName SlotId,
	FName RequiredRuneId,
	FTransform SlotWorldTransform,
	int32 RequiredRuneCount)
{
	if (HasAuthority())
	{
		PlaceRuneOnAuthority(Pentagram, SlotId, RequiredRuneId, SlotWorldTransform, RequiredRuneCount);
		return;
	}

	// A picked-up Base_Item is owned by its character, so its client is allowed
	// to send this RPC. Dropped world runes must be picked up before placement.
	ServerPlaceRuneInPentagram(Pentagram, SlotId, RequiredRuneId, SlotWorldTransform, RequiredRuneCount);
}

void ARune_Item::ServerPlaceRuneInPentagram_Implementation(
	AActor* Pentagram,
	FName SlotId,
	FName RequiredRuneId,
	FTransform SlotWorldTransform,
	int32 RequiredRuneCount)
{
	PlaceRuneOnAuthority(Pentagram, SlotId, RequiredRuneId, SlotWorldTransform, RequiredRuneCount);
}

void ARune_Item::PlaceRuneOnAuthority(
	AActor* Pentagram,
	FName SlotId,
	FName RequiredRuneId,
	const FTransform& SlotWorldTransform,
	int32 RequiredRuneCount)
{
	const bool bValidRequest =
		IsValid(Pentagram) &&
		!SlotId.IsNone() &&
		RequiredRuneCount > 0 &&
		CanPlaceInRuneSlot(RequiredRuneId) &&
		!IsSlotAlreadyOccupied(Pentagram, SlotId);

	if (!bValidRequest)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[RuneRitual] REJECTED Rune=%s RuneId=%s Slot=%s Required=%s "
				"PentagramValid=%d SlotValid=%d CountValid=%d RuneMatches=%d Occupied=%d"),
			*GetName(), *RuneId.ToString(), *SlotId.ToString(), *RequiredRuneId.ToString(),
			IsValid(Pentagram), !SlotId.IsNone(), RequiredRuneCount > 0,
			CanPlaceInRuneSlot(RequiredRuneId),
			IsValid(Pentagram) && IsSlotAlreadyOccupied(Pentagram, SlotId));
		MulticastPlacementResult(false, SlotId, RequiredRuneId);
		return;
	}

	if (OwningCharacter && !OwningCharacter->ReleaseHeldItemForPlacement(this))
	{
		UE_LOG(LogTemp, Warning, TEXT("[RuneRitual] REJECTED %s could not leave the player's hand"), *GetName());
		MulticastPlacementResult(false, SlotId, RequiredRuneId);
		return;
	}

	bPlacedInPentagram = true;
	PlacedSlotId = SlotId;
	PlacedPentagram = Pentagram;

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetActorTransform(SlotWorldTransform, false, nullptr, ETeleportType::TeleportPhysics);
	AttachToActor(Pentagram, FAttachmentTransformRules::KeepWorldTransform);
	ApplyPlacedState();
	ForceNetUpdate();

	UE_LOG(LogTemp, Log, TEXT("[RuneRitual] ACCEPTED Rune=%s RuneId=%s Slot=%s"),
		*GetName(), *RuneId.ToString(), *SlotId.ToString());
	MulticastPlacementResult(true, SlotId, RequiredRuneId);
	CheckForCompletedRitual(RequiredRuneCount);
}

bool ARune_Item::IsSlotAlreadyOccupied(AActor* Pentagram, FName SlotId) const
{
	if (!GetWorld() || !IsValid(Pentagram) || SlotId.IsNone())
	{
		return false;
	}

	for (TActorIterator<ARune_Item> It(GetWorld()); It; ++It)
	{
		const ARune_Item* OtherRune = *It;
		if (OtherRune != this && OtherRune->bPlacedInPentagram &&
			OtherRune->PlacedPentagram == Pentagram && OtherRune->PlacedSlotId == SlotId)
		{
			return true;
		}
	}
	return false;
}

void ARune_Item::CheckForCompletedRitual(int32 RequiredRuneCount)
{
	if (!HasAuthority() || bRitualCompleted || !IsValid(PlacedPentagram))
	{
		return;
	}

	// Placement and pentagram completion do not depend on a death target. This
	// target is required only for the optional timeline-restoration part.
	if (!IsValid(RitualTargetPlayer) || TimelineToRestore == EItemTimeline::Both)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[RuneRitual] Rune=%s placed successfully without a ritual target; "
				"pentagram can complete, but no player timeline will be restored"),
			*GetName());
		return;
	}

	int32 CorrectPlacedCount = 0;
	for (TActorIterator<ARune_Item> It(GetWorld()); It; ++It)
	{
		const ARune_Item* OtherRune = *It;
		if (OtherRune->bPlacedInPentagram &&
			OtherRune->PlacedPentagram == PlacedPentagram &&
			OtherRune->RitualTargetPlayer == RitualTargetPlayer &&
			OtherRune->TimelineToRestore == TimelineToRestore)
		{
			++CorrectPlacedCount;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[RuneRitual] Pentagram=%s progress %d/%d for Player=%s"),
		*GetNameSafe(PlacedPentagram), CorrectPlacedCount, RequiredRuneCount, *GetNameSafe(RitualTargetPlayer));

	if (CorrectPlacedCount < RequiredRuneCount)
	{
		return;
	}

	bRitualCompleted = true;
	ForceNetUpdate();
	BroadcastRitualCompleted();

	UE_LOG(LogTemp, Warning, TEXT("[RuneRitual] GROUP COMPLETE TargetMetadata=%s OriginalTimeline=%s. "
		"RunePentagram will transfer the player who inserted the third rune."),
		*GetNameSafe(RitualTargetPlayer),
		*StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(TimelineToRestore)));
}

void ARune_Item::ApplyPlacedState()
{
	if (!bPlacedInPentagram)
	{
		return;
	}

	bIsPickedUp = false;
	SetActorHiddenInGame(false);
	SetActorEnableCollision(false);

	TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents(this);
	for (UPrimitiveComponent* Primitive : PrimitiveComponents)
	{
		if (IsValid(Primitive))
		{
			Primitive->SetSimulatePhysics(false);
			Primitive->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}

	// Physics detaches Base_Item's mesh from its scene root. Restore the authored
	// hierarchy so the replicated actor/slot transform also moves the rune mesh.
	if (IsValid(ItemMesh) && IsValid(GetRootComponent()) && ItemMesh != GetRootComponent())
	{
		if (ItemMesh->GetAttachParent() != GetRootComponent())
		{
			ItemMesh->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		}
		ItemMesh->SetRelativeTransform(ItemMeshRelativeTransform);
	}
}

void ARune_Item::MulticastPlacementResult_Implementation(bool bAccepted, FName SlotId, FName RequiredRuneId)
{
	OnRunePlacementResult.Broadcast(this, bAccepted, SlotId, RequiredRuneId);
}

void ARune_Item::OnRep_PlacementState()
{
	ApplyPlacedState();
}

void ARune_Item::OnRep_RitualTarget()
{
	// RepNotify intentionally provides a Blueprint-visible synchronization point
	// through the replicated properties; no presentation state is required here.
}

void ARune_Item::OnRep_RitualCompleted()
{
	if (bRitualCompleted)
	{
		BroadcastRitualCompleted();
	}
}

void ARune_Item::BroadcastRitualCompleted()
{
	OnRitualCompleted.Broadcast(this, RitualTargetPlayer, TimelineToRestore);
}
