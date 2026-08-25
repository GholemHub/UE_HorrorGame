#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HronoSharedTools.h"

#include "Net/UnrealNetwork.h"
#include "GameplayTagContainer.h"
#include "HronoSharedTools.h"

#include "Base_Item.generated.h"

class AHronoCharacter;
class USoundBase;

/** Network-visible lifecycle of an item offered through a bound mirror. */
UENUM(BlueprintType)
enum class EMirrorItemTransferState : uint8
{
	None,
	Preview,
	Pending,
	Completed
};

UCLASS()
class HRONO_API ABase_Item : public AActor
{
	GENERATED_BODY()
	
public:	
	ABase_Item();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemType ItemType = EItemType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item TAG")
	FGameplayTag ItemTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item TAG")
	FGameplayTagContainer ItemTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item TAG")
	bool UsableValid = true;

	/** Explicit opt-in: only these items may be offered through a bound mirror. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Mirror Transfer")
	bool bCanTransferThroughMirror = false;

	/** Authoritative mirror-transfer state. Preview never creates another pickup item. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_MirrorTransferState,
		Category = "Item|Mirror Transfer")
	EMirrorItemTransferState MirrorTransferState = EMirrorItemTransferState::None;

	UFUNCTION(BlueprintPure, Category = "Item|Mirror Transfer")
	bool CanTransferThroughMirror() const { return bCanTransferThroughMirror; }

	/** Server-only state mutation used by the mirror transfer surface. */
	void SetMirrorTransferState(EMirrorItemTransferState NewState);

	
	bool HasTag(FGameplayTag Tag) const
	{
		return ItemTags.HasTag(Tag);
	}
	UFUNCTION(BlueprintNativeEvent)
	void Use(AActor* Character);

	virtual void Use_Implementation(AActor* Character);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> DefaultSceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* ItemMesh;
	/** Display name of the item */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FText ItemName;

	/** Description of the item */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FText ItemDescription;

	/** Played when this item is picked up. Assign any sound in Blueprint. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> PickupSound;

	/** Played when this item is dropped. Assign any sound in Blueprint. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> DropSound;

	/** Which timeline this item belongs to (determines who can see/pick it up) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_ItemTimeline, Category = "Item")
	EItemTimeline ItemTimeline = EItemTimeline::Both;

	/** Changes this item's timeline. This must be called on the server for replication. */
	UFUNCTION(BlueprintCallable, Category = "Item|Timeline")
	void SetItemTimeline(EItemTimeline NewTimeline);

	UPROPERTY(EditAnywhere)
	UStaticMesh* PastMesh;

	UPROPERTY(EditAnywhere)
	UStaticMesh* FutureMesh;

	UFUNCTION(BlueprintCallable, Category = "Item")
	virtual void UpdateMeshForLocalPlayer();

	virtual bool TryPickUp(AHronoCharacter* Character);
	void OnPickedUp(AHronoCharacter* Character);

	bool AttachToCharacter();

	/** Reattaches an already held item to its owner's timeline-specific interaction point. */
	bool RefreshHeldAttachmentPoint();

	/**
	 * Called after this item has been attached to a character's hand, and again
	 * when it is dropped. Use this to enable expensive item-only effects while held.
	 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Item|Pickup")
	void OnHeldStateChanged(bool bIsHeld, AHronoCharacter* Character);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OwningCharacter)
	AHronoCharacter* OwningCharacter;

	UPROPERTY(EditAnywhere, Category = "Pickup")
	FTransform HoldOffset;

	UFUNCTION()
	virtual void OnRep_OwningCharacter(AHronoCharacter* PreviousOwningCharacter);

	UFUNCTION(BlueprintCallable, Category = "Item")

	void Drop();

	UPROPERTY(BlueprintReadOnly, Category = "Item")
	bool bIsPickedUp = false;

	UFUNCTION()
	void DetachFromCharacter();
	
	UFUNCTION()
	void UpdateVisibilityForLocalPlayer(EItemTimeline ViewerTimeline);

	UFUNCTION()
	void OnRep_ItemTimeline();

	UFUNCTION()
	void OnRep_MirrorTransferState();

	/** Cosmetic hook for item-specific transfer effects. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Item|Mirror Transfer")
	void OnMirrorTransferStateChanged(EMirrorItemTransferState NewState);
	
protected:
	virtual void BeginPlay() override;

	/** The mesh's authored transform under DefaultSceneRoot, restored after physics detaches it. */
	FTransform ItemMeshRelativeTransform = FTransform::Identity;

	/** Applies mesh, gameplay tag, collision, and local visibility for ItemTimeline. */
	void ApplyItemTimelineState();

	/** Restores world physics and timeline interaction responses for a dropped item. */
	void ConfigureDroppedCollision(UPrimitiveComponent* PrimitiveComponent);

	// Tracks what mesh state is currently visible to avoid spamming updates
	EItemTimeline CurrentCachedTimeline = EItemTimeline::Both;
public:	
	virtual void Tick(float DeltaTime) override;
	UStaticMeshComponent* GetItemMesh() const { return ItemMesh; }

};
