#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Room.generated.h"

class ABase_Item;
class AClock;
class ADrag_Item;
class AHotDot;
class ARoom;
class UBoxComponent;
class UPrimitiveComponent;
class USceneComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FRoomCursedStateChangedSignature,
	ARoom*, Room,
	bool, bIsCursed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FRoomPuzzleCompletedSignature,
	ARoom*, Room,
	ABase_Item*, CursedItem,
	ABase_Item*, SpawnedKey);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FWrongRoomChosenSignature,
	ARoom*, Room,
	ABase_Item*, CursedItem);

/** Replicated result of the native painting-evidence selection algorithm. */
USTRUCT(BlueprintType)
struct HRONO_API FRoomPaintingEvidenceState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room|Paintings")
	int32 PatternIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room|Paintings")
	int32 PatternSeed = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room|Paintings")
	TArray<TObjectPtr<AActor>> SelectedPaintings;
};

/**
 * Server-authoritative representation of one searchable room.
 *
 * The Scare Director chooses exactly one placed Room each game. Only that room can
 * resolve the cursed-item puzzle. When a cursed item is inside RoomVolume and every
 * configured door is closed, the room replaces the item with a key.
 */
UCLASS(BlueprintType, Blueprintable)
class HRONO_API ARoom : public AActor
{
	GENERATED_BODY()

public:
	ARoom();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Authority-only. Normally called by Scare Director at the start of a game. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Room")
	void SetCursed(bool bNewCursed);

	UFUNCTION(BlueprintPure, Category = "Room")
	bool IsCursed() const { return bIsCursed; }

	/** Applies a deterministic clock pattern selected by the Scare Director. Server only. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Room|Clocks")
	void ConfigureClockAnomalies(int32 PatternIndex, int32 PatternSeed);

	/** Human-readable list of every configured clock and its current behavior. */
	UFUNCTION(BlueprintPure, Category = "Room|Clocks")
	FString GetClockAnomalySummary() const;

	/** Applies the HotDot activation pattern selected by the Scare Director. Server only. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Room|HotDots")
	void ConfigureHotDots(int32 PatternIndex, int32 PatternSeed);

	UFUNCTION(BlueprintPure, Category = "Room|HotDots")
	FString GetHotDotSummary() const;

	/** Selects painting evidence and invokes OnCursedPainting. Server only. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Room|Paintings")
	void ConfigurePaintingEvidence(int32 PatternIndex, int32 PatternSeed);

	/** Returns the most recent replicated selection as a Blueprint output array. */
	UFUNCTION(BlueprintPure, Category = "Room|Paintings")
	TArray<AActor*> GetSelectedCursedPaintings() const;

	/**
	 * Implement this in BP_Rooms. EvidencePaintings contains two actors for the
	 * cursed room, and zero or one actor for an ordinary room.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Room|Paintings",
		meta = (DisplayName = "On Cursed Painting"))
	void OnCursedPainting(const TArray<AActor*>& EvidencePaintings);

	static int32 GetOrdinaryClockPatternCount();
	static int32 GetCursedClockPatternCount();

	UFUNCTION(BlueprintPure, Category = "Room|Puzzle")
	bool IsPuzzleCompleted() const { return bPuzzleCompleted; }

	UFUNCTION(BlueprintPure, Category = "Room|Puzzle")
	bool IsCursedItemInRoom() const;

	/** True only when at least one valid door is configured and all such doors are closed. */
	UFUNCTION(BlueprintPure, Category = "Room|Puzzle")
	bool AreAllDoorsClosed() const;

	/** Re-evaluates the item/door conditions. Useful after changing room references at runtime. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Room|Puzzle")
	bool TryCompleteRoomPuzzle();

	/** Clears runtime puzzle state. It does not destroy an already spawned key. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Room|Puzzle")
	void ResetRoomPuzzle();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	/** Volume used to detect the dropped cursed item. Resize it to cover the room interior. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room|Components")
	TObjectPtr<UBoxComponent> RoomVolume;

	/** Placed painting actors. Each valid evidence painting must derive from ABase_Item. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Room|Contents",
		meta = (DisplayName = "Paintings"))
	TArray<TObjectPtr<AActor>> Paintings;

	/**
	 * Placed clock actors that belong to this room.
	 *
	 * This intentionally uses AActor references so Unreal's level actor picker always
	 * shows Blueprint clock instances. At runtime non-AClock entries are ignored with
	 * a warning, while BP_Clock_Item (whose native parent is AClock) is accepted.
	 */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Room|Contents",
		meta = (DisplayName = "Clocks"))
	TArray<TObjectPtr<AActor>> Clocks;

	/** Placed HotDot actors. Actor references keep Blueprint instances visible in the level picker. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Room|Contents",
		meta = (DisplayName = "Hot Dots"))
	TArray<TObjectPtr<AActor>> HotDots;

	/** Every timeline-specific entrance door that must be closed to solve this room. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Room|Puzzle")
	TArray<TObjectPtr<ADrag_Item>> RoomDoors;

	/** Optional class check for cursed items. Defaults to the existing BP_Cursed_Item. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room|Puzzle")
	TSubclassOf<ABase_Item> CursedItemClass;

	/** Optional tag check, allowing other cursed-item classes without changing this room. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room|Puzzle",
		meta = (GameplayTagFilter = "Item"))
	FGameplayTag CursedItemTag;

	/** Key spawned at the cursed item's transform. Defaults to the existing BP_Key_Item. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room|Puzzle")
	TSubclassOf<ABase_Item> KeyClass;

	/** Local-space offset from the cursed item used for the spawned key. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room|Puzzle")
	FVector KeySpawnOffset = FVector(0.0f, 0.0f, 20.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room|Puzzle")
	bool bConsumeCursedItem = true;

	/** Prevents a room with no configured doors from completing as soon as an item enters. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room|Puzzle")
	bool bRequireAtLeastOneDoor = true;

	UPROPERTY(BlueprintAssignable, Category = "Room|Events")
	FRoomCursedStateChangedSignature OnCursedStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Room|Events")
	FRoomPuzzleCompletedSignature OnRoomPuzzleCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Room|Events")
	FWrongRoomChosenSignature OnWrongRoomChosen;

	UFUNCTION(BlueprintImplementableEvent, Category = "Room|Events",
		meta = (DisplayName = "Receive Cursed State Changed"))
	void ReceiveCursedStateChanged(bool bNewCursed);

	UFUNCTION(BlueprintImplementableEvent, Category = "Room|Events",
		meta = (DisplayName = "Receive Room Puzzle Completed"))
	void ReceiveRoomPuzzleCompleted(ABase_Item* CursedItem, ABase_Item* SpawnedKeyActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Room|Events",
		meta = (DisplayName = "Receive Wrong Room Chosen"))
	void ReceiveWrongRoomChosen(ABase_Item* CursedItem);

	/** Server-only answer. Evidence outcomes replicate, but the selected room identity does not. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly,
		Category = "Room|State")
	bool bIsCursed = false;

	UPROPERTY(ReplicatedUsing = OnRep_PuzzleCompleted, VisibleInstanceOnly, BlueprintReadOnly,
		Category = "Room|State")
	bool bPuzzleCompleted = false;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Room|State")
	TObjectPtr<ABase_Item> SpawnedKey;

	/** Pattern and seed are replicated for debugging and late-joining clients. */
	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Room|Clocks")
	int32 ClockAnomalyPatternIndex = INDEX_NONE;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Room|Clocks")
	int32 ClockAnomalyPatternSeed = 0;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Room|HotDots")
	int32 HotDotPatternIndex = INDEX_NONE;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Room|HotDots")
	int32 HotDotPatternSeed = 0;

	UPROPERTY(ReplicatedUsing = OnRep_PaintingEvidenceState, VisibleInstanceOnly,
		BlueprintReadOnly, Category = "Room|Paintings")
	FRoomPaintingEvidenceState PaintingEvidenceState;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Room|State")
	TObjectPtr<ABase_Item> CurrentCursedItem;

protected:
	UFUNCTION()
	void HandleRoomBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void HandleRoomEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex);

	UFUNCTION()
	void HandleDoorStateChanged(bool bDoorIsClosed);

	UFUNCTION()
	void OnRep_PuzzleCompleted();

	UFUNCTION()
	void OnRep_PaintingEvidenceState();

private:
	bool IsCursedItem(const ABase_Item* Item) const;
	void BindDoorEvents();
	void UnbindDoorEvents();
	void InitializeClocks();
	void RefreshClockCursedStates();
	void ApplyClockAnomalyPattern();
	void ApplyHotDotPattern();
	void ApplyPaintingEvidencePattern();
	void DispatchCursedPaintingEvent();
	void DispatchCursedStateChanged();
};
