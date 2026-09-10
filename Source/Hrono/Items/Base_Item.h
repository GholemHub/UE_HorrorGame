#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HronoSharedTools.h"

#include "Net/UnrealNetwork.h"
#include "GameplayTagContainer.h"
#include "HronoSharedTools.h"

#include "Base_Item.generated.h"

class AHronoCharacter;
class UMaterialInterface;
class UMeshComponent;
class USceneCaptureComponent2D;
class USoundBase;
class UHeldItemInertiaComponent;

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

	/** Local-only overlay shown while the local player aims at this item. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Interaction Highlight")
	TObjectPtr<UMaterialInterface> InteractionOverlayMaterial;

	/** Allows individual item classes or placed instances to opt out of aim-based highlighting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Interaction Highlight")
	bool bUseInteractionHighlight = true;

	/** Applies or removes the local interaction overlay from every mesh owned by this item. */
	UFUNCTION(BlueprintCallable, Category = "Item|Interaction Highlight")
	void SetInteractionHighlighted(bool bHighlighted);

	/** Local-only guidance overlay controlled by gameplay context rather than aiming. */
	UFUNCTION(BlueprintCallable, Category = "Item|Interaction Highlight")
	void SetInteractionContextHighlighted(bool bHighlighted);

	/** Keeps the overlay enabled until explicitly cleared, independently of aiming. Server-authored. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Item|Interaction Highlight")
	void SetInteractionHighlightForced(bool bForced);

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_ForceInteractionHighlight,
		Category = "Item|Interaction Highlight")
	bool bForceInteractionHighlight = false;

	/** Whether this item is currently a valid highlight target for the supplied local viewer. */
	UFUNCTION(BlueprintPure, Category = "Item|Interaction Highlight")
	bool CanHighlightFor(const AHronoCharacter* Viewer) const;

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

	/** Local-only procedural lag around HoldOffset. Tune inline or assign a Data Asset profile. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UHeldItemInertiaComponent> HeldItemInertia;
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

	/** Enables replicated loose-world physics for server-spawned pickups such as ritual keys. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Item|Physics")
	void EnableDroppedPhysics();

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

	/**
	 * Keeps expensive SceneCapture components dormant while this item is in the
	 * world. A held capture is enabled only on the owning local player's machine.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Performance")
	bool bOnlyRunSceneCaptureWhileLocallyHeld = true;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OwningCharacter)
	AHronoCharacter* OwningCharacter;

	UPROPERTY(EditAnywhere, Category = "Pickup")
	FTransform HoldOffset;

	UFUNCTION()
	virtual void OnRep_OwningCharacter(AHronoCharacter* PreviousOwningCharacter);

	UFUNCTION(BlueprintCallable, Category = "Item")
	virtual void Drop();

	UPROPERTY(BlueprintReadOnly, Category = "Item")
	bool bIsPickedUp = false;

	UFUNCTION()
	void DetachFromCharacter();
	
	UFUNCTION()
	virtual void UpdateVisibilityForLocalPlayer(EItemTimeline ViewerTimeline);

	UFUNCTION()
	void OnRep_ItemTimeline();

	UFUNCTION()
	void OnRep_MirrorTransferState();

	UFUNCTION()
	void OnRep_DroppedPhysicsEnabled();

	UFUNCTION()
	void OnRep_ForceInteractionHighlight();

	/** Cosmetic hook for item-specific transfer effects. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Item|Mirror Transfer")
	void OnMirrorTransferStateChanged(EMirrorItemTransferState NewState);
	
protected:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;

	/** True only when a native subclass genuinely needs to tick while idle. */
	virtual bool RequiresContinuousItemTick() const { return false; }

	/** Keeps legacy Blueprint Event Tick graphs alive while native idle items stay dormant. */
	bool HasBlueprintTickImplementation() const;

	/** Enables Tick for a temporary native activity, a continuous subclass, or a Blueprint Tick graph. */
	void RefreshItemTickEnabled(bool bTemporaryNativeActivity = false);

	/** Pickable actors and every component in their attachment hierarchy must be movable. */
	void EnsureMovableComponentHierarchy();

	/** The mesh's authored transform under DefaultSceneRoot, restored after physics detaches it. */
	FTransform ItemMeshRelativeTransform = FTransform::Identity;

	/** Applies mesh, gameplay tag, collision, and local visibility for ItemTimeline. */
	void ApplyItemTimelineState();
	void ApplyDroppedPhysicsState();
	/** Class/category policy for local aim highlighting. Forced highlights are handled separately. */
	virtual bool AllowsAimInteractionHighlight() const;
	void EnsureInteractionOverlayMaterial();
	void RefreshInteractionHighlight();
	void LogHeldTransformState(const TCHAR* Context) const;
	void SetHeldSceneCapturesEnabled(bool bEnabled);

	/** Restores world physics and timeline interaction responses for a dropped item. */
	void ConfigureDroppedCollision(UPrimitiveComponent* PrimitiveComponent);

	// Tracks what mesh state is currently visible to avoid spamming updates
	EItemTimeline CurrentCachedTimeline = EItemTimeline::Both;

	UPROPERTY(ReplicatedUsing = OnRep_DroppedPhysicsEnabled)
	bool bDroppedPhysicsEnabled = false;

	/** Original overlays are restored instead of being blindly cleared. Local cosmetic state only. */
	TMap<TWeakObjectPtr<UMeshComponent>, TWeakObjectPtr<UMaterialInterface>> PreviousOverlayMaterials;

	bool bInteractionHovered = false;
	bool bInteractionContextHighlighted = false;
	bool bInteractionHighlighted = false;
public:	
	UStaticMeshComponent* GetItemMesh() const { return ItemMesh; }
	UHeldItemInertiaComponent* GetHeldItemInertia() const { return HeldItemInertia; }

};
