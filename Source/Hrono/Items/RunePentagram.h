#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HronoSharedTools.h"
#include "Interface/Enviroment_Interface.h"
#include "RunePentagram.generated.h"

class AHronoCharacter;
class ARune_Item;
class UBoxComponent;
class UPrimitiveComponent;
class USceneComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
	FPentagramRuneInteractionDelegate,
	bool, bAccepted,
	ARune_Item*, Rune,
	FName, SlotId,
	FName, ResultReason);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPentagramCompletedDelegate);

/**
 * Three-slot ritual pentagram. Pressing the normal Interact key while looking
 * at it attempts to insert the character's currently selected Rune_Item.
 */
UCLASS(BlueprintType, Blueprintable)
class HRONO_API ARunePentagram : public AActor, public IEnviroment_Interface
{
	GENERATED_BODY()

public:
	ARunePentagram();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pentagram|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	/** Assign the pentagram mesh in a Blueprint child. It receives the E interaction trace. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pentagram|Components")
	TObjectPtr<UStaticMeshComponent> PentagramMesh;

	/** Move these components in the Blueprint viewport to position the three runes. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pentagram|Components")
	TObjectPtr<USceneComponent> RuneSlotOne;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pentagram|Components")
	TObjectPtr<USceneComponent> RuneSlotTwo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pentagram|Components")
	TObjectPtr<USceneComponent> RuneSlotThree;

	/** Interaction volumes. Each box follows its corresponding rune slot. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pentagram|Components")
	TObjectPtr<UBoxComponent> RuneSlotOneCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pentagram|Components")
	TObjectPtr<UBoxComponent> RuneSlotTwoCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pentagram|Components")
	TObjectPtr<UBoxComponent> RuneSlotThreeCollision;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pentagram|Required Runes")
	FName RequiredRuneIdOne = TEXT("Rune.One");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pentagram|Required Runes")
	FName RequiredRuneIdTwo = TEXT("Rune.Two");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pentagram|Required Runes")
	FName RequiredRuneIdThree = TEXT("Rune.Three");

	UPROPERTY(ReplicatedUsing = OnRep_RuneSlots, VisibleInstanceOnly, BlueprintReadOnly,
		Category = "Pentagram|Runtime")
	TObjectPtr<ARune_Item> PlacedRuneOne;

	UPROPERTY(ReplicatedUsing = OnRep_RuneSlots, VisibleInstanceOnly, BlueprintReadOnly,
		Category = "Pentagram|Runtime")
	TObjectPtr<ARune_Item> PlacedRuneTwo;

	UPROPERTY(ReplicatedUsing = OnRep_RuneSlots, VisibleInstanceOnly, BlueprintReadOnly,
		Category = "Pentagram|Runtime")
	TObjectPtr<ARune_Item> PlacedRuneThree;

	UPROPERTY(ReplicatedUsing = OnRep_PentagramCompleted, VisibleInstanceOnly, BlueprintReadOnly,
		Category = "Pentagram|Runtime")
	bool bPentagramCompleted = false;

	/** Character whose interaction successfully inserted the third rune. */
	UPROPERTY(ReplicatedUsing = OnRep_CompletingPlayer, VisibleInstanceOnly, BlueprintReadOnly,
		Category = "Pentagram|Runtime")
	TObjectPtr<AHronoCharacter> CompletingPlayer;

	/** Timeline assigned to CompletingPlayer after the third rune was inserted. */
	UPROPERTY(ReplicatedUsing = OnRep_CompletingPlayer, VisibleInstanceOnly, BlueprintReadOnly,
		Category = "Pentagram|Runtime")
	EItemTimeline CompletingPlayerNewTimeline = EItemTimeline::Both;

	/** Continuously displays setup, slot progress, collision and last interaction in PIE. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pentagram|Debug")
	bool bShowDebugStatusOnScreen = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Pentagram|Debug")
	FString LastInteractionDebug = TEXT("Waiting for player interaction");

	/** Accepted/rejected result for sounds, UI and effects. ResultReason is a stable FName. */
	UPROPERTY(BlueprintAssignable, Category = "Pentagram|Events")
	FPentagramRuneInteractionDelegate OnRuneInteractionResult;

	/** Fired exactly once on the server and on every client when all three slots are filled. */
	UPROPERTY(BlueprintAssignable, Category = "Pentagram|Events")
	FPentagramCompletedDelegate OnPentagramCompleted;

	/** Fired immediately when the first correct rune is inserted. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Pentagram|Events",
		meta = (DisplayName = "On First Rune Inserted"))
	void BP_OnFirstRuneInserted(
		ARune_Item* InsertedRune,
		AHronoCharacter* PlayerWhoInsertedRune,
		FName SlotId);

	/** Fired immediately when the second correct rune is inserted. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Pentagram|Events",
		meta = (DisplayName = "On Second Rune Inserted"))
	void BP_OnSecondRuneInserted(
		ARune_Item* InsertedRune,
		AHronoCharacter* PlayerWhoInsertedRune,
		FName SlotId);

	/**
	 * Automatic Blueprint event invoked once after all three correct runes are inserted.
	 * It is intentionally not BlueprintCallable: Blueprint receives this event but
	 * cannot manually trigger it. The only parameter is the last placing player.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Pentagram|Events",
		meta = (DisplayName = "On All Three Runes Inserted"))
	void BP_OnAllThreeRunesInserted(AHronoCharacter* PlayerWhoPlacedLastRune);

	/** Blueprint event equivalent of OnPentagramCompleted. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Pentagram|Events",
		meta = (DisplayName = "On Pentagram Ritual Completed"))
	void BP_OnPentagramCompleted();

	/** Called automatically by the existing character E interaction. */
	virtual void Interact_Implementation(AActor* Interactor) override;

	UFUNCTION(BlueprintPure, Category = "Pentagram")
	bool IsPentagramComplete() const { return bPentagramCompleted; }

	UFUNCTION(BlueprintPure, Category = "Pentagram")
	int32 GetPlacedRuneCount() const;

	/** Complete human-readable runtime state for Print String or a debug widget. */
	UFUNCTION(BlueprintPure, Category = "Pentagram|Debug")
	FString GetPentagramDebugStatus() const;

	UFUNCTION(BlueprintCallable, Category = "Pentagram|Debug")
	void PrintPentagramDebugStatus(float Duration = 5.0f);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRuneInteractionResult(
		bool bAccepted,
		ARune_Item* Rune,
		FName SlotId,
		FName ResultReason);

	/** Delivers the insertion-order Blueprint event on the server and every client. */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRuneInserted(
		ARune_Item* InsertedRune,
		AHronoCharacter* PlayerWhoInsertedRune,
		FName SlotId,
		int32 InsertedRuneCount);

	UFUNCTION()
	void OnRep_RuneSlots();

	UFUNCTION()
	void OnRep_PentagramCompleted();

	UFUNCTION()
	void OnRep_CompletingPlayer();

private:
	bool TryInsertCurrentRune(AHronoCharacter* Character);
	bool TryPlaceRuneInMatchingSlot(ARune_Item* Rune, AHronoCharacter* PlacingCharacter);
	bool HasValidRequiredRuneSetup() const;
	void CheckPentagramCompletion(AHronoCharacter* PlayerWhoPlacedRune);
	void BroadcastPentagramCompleted();
	void DeliverThirdRuneEvents();
	void RefreshReplicatedRuneAttachments();
	void ConfigureInteractionCollision(UPrimitiveComponent* Component) const;
	void SetLastInteractionDebug(const FString& NewStatus, bool bSuccess);

	bool bThirdRuneEventsDelivered = false;
};
