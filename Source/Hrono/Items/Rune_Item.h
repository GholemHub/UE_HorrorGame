#pragma once

#include "CoreMinimal.h"
#include "Items/Base_Item.h"
#include "Rune_Item.generated.h"

class AHronoCharacter;
class ARune_Item;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
	FRunePlacementResultDelegate,
	ARune_Item*, Rune,
	bool, bAccepted,
	FName, SlotId,
	FName, RequiredRuneId);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FRuneRitualCompletedDelegate,
	ARune_Item*, CompletingRune,
	AHronoCharacter*, RitualTargetPlayer,
	EItemTimeline, OriginalTimeline);

/**
 * Pickable ritual rune used by BP_TableRitualManager.
 *
 * The server validates the rune ID and slot, locks an accepted rune into the
 * supplied world transform. RunePentagram decides which player is transferred
 * after all three unique slots have been filled.
 */
UCLASS(Blueprintable)
class HRONO_API ARune_Item : public ABase_Item
{
	GENERATED_BODY()

public:
	ARune_Item();

	/** Symbol/identity of this rune. It must equal the slot's Required Rune Id. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Rune", meta = (ExposeOnSpawn = "true"))
	FName RuneId = TEXT("Rune.None");

	/** Optional death-ritual target metadata retained for Blueprint logic. */
	UPROPERTY(ReplicatedUsing = OnRep_RitualTarget, VisibleInstanceOnly, BlueprintReadOnly,
		Category = "Rune|Ritual")
	TObjectPtr<AHronoCharacter> RitualTargetPlayer;

	/** Timeline the optional target occupied before death. */
	UPROPERTY(ReplicatedUsing = OnRep_RitualTarget, VisibleInstanceOnly, BlueprintReadOnly,
		Category = "Rune|Ritual")
	EItemTimeline TimelineToRestore = EItemTimeline::Past;

	UPROPERTY(ReplicatedUsing = OnRep_PlacementState, VisibleInstanceOnly, BlueprintReadOnly,
		Category = "Rune|Placement")
	bool bPlacedInPentagram = false;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Rune|Placement")
	FName PlacedSlotId = NAME_None;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Rune|Placement")
	TObjectPtr<AActor> PlacedPentagram;

	UPROPERTY(ReplicatedUsing = OnRep_RitualCompleted, VisibleInstanceOnly, BlueprintReadOnly,
		Category = "Rune|Ritual")
	bool bRitualCompleted = false;

	/** Fired on server and clients after validation. Use this for sounds/UI. */
	UPROPERTY(BlueprintAssignable, Category = "Rune|Events")
	FRunePlacementResultDelegate OnRunePlacementResult;

	/** Fired once on the rune that fills the final required slot. */
	UPROPERTY(BlueprintAssignable, Category = "Rune|Events")
	FRuneRitualCompletedDelegate OnRitualCompleted;

	/** Assigns the killed player and the timeline they must return to. Server only. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Rune|Ritual")
	void InitializeForKilledPlayer(AHronoCharacter* KilledPlayer, EItemTimeline OriginalTimeline);

	/** Convenience server-only spawn node for the death Blueprint. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Rune|Ritual",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Spawn Rune For Killed Player"))
	static ARune_Item* SpawnRuneForKilledPlayer(
		const UObject* WorldContextObject,
		TSubclassOf<ARune_Item> RuneClass,
		const FTransform& SpawnTransform,
		AHronoCharacter* KilledPlayer,
		EItemTimeline OriginalTimeline);

	/** Returns true when this rune matches the requested slot and is not already placed. */
	UFUNCTION(BlueprintPure, Category = "Rune|Placement")
	bool CanPlaceInRuneSlot(FName RequiredRuneId) const;

	/**
	 * Multiplayer-safe placement request. Call it on the rune currently held by the player.
	 * Slot Id must be unique within the pentagram. Required Rune Count is the number
	 * of correct slots needed to complete the ritual.
	 */
	UFUNCTION(BlueprintCallable, Category = "Rune|Placement", meta = (DisplayName = "Place Rune In Pentagram"))
	void PlaceRuneInPentagram(
		AActor* Pentagram,
		FName SlotId,
		FName RequiredRuneId,
		FTransform SlotWorldTransform,
		int32 RequiredRuneCount);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_OwningCharacter(AHronoCharacter* PreviousOwningCharacter) override;

protected:
	UFUNCTION(Server, Reliable)
	void ServerPlaceRuneInPentagram(
		AActor* Pentagram,
		FName SlotId,
		FName RequiredRuneId,
		FTransform SlotWorldTransform,
		int32 RequiredRuneCount);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlacementResult(bool bAccepted, FName SlotId, FName RequiredRuneId);

	UFUNCTION()
	void OnRep_PlacementState();

	UFUNCTION()
	void OnRep_RitualTarget();

	UFUNCTION()
	void OnRep_RitualCompleted();

private:
	void PlaceRuneOnAuthority(
		AActor* Pentagram,
		FName SlotId,
		FName RequiredRuneId,
		const FTransform& SlotWorldTransform,
		int32 RequiredRuneCount);

	bool IsSlotAlreadyOccupied(AActor* Pentagram, FName SlotId) const;
	void CheckForCompletedRitual(int32 RequiredRuneCount);
	void ApplyPlacedState();
	void BroadcastRitualCompleted();
};
