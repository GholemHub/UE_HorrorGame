#pragma once

#include "CoreMinimal.h"
#include "Items/Base_Item.h"
#include "TimelineTransferItem.generated.h"

class AHronoCharacter;
class AMirrorTransferPreview;
class UBoxComponent;
class UNiagaraComponent;
class USceneComponent;

/**
 * Server-authoritative item transfer surface for a bound mirror.
 * The real item remains attached to the source player during Preview. A
 * replicated, non-pickup proxy is placed at LinkedTransfer for interaction.
 */
UCLASS(Blueprintable)
class HRONO_API ATimelineTransferItem : public ABase_Item
{
	GENERATED_BODY()

public:
	ATimelineTransferItem();

	/** Thin detection box placed immediately in front of the mirror plane. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mirror Transfer|Components",
		meta = (DisplayName = "TransferVolume"))
	TObjectPtr<UBoxComponent> TransferBox;

	/** Preview origin/orientation, normally centered on the mirror surface. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mirror Transfer|Components")
	TObjectPtr<USceneComponent> TransferPoint;

	/** Niagara effect shown for the complete transfer, from preview creation until completion/cancellation. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mirror Transfer|Components")
	TObjectPtr<UNiagaraComponent> TransferVFX;

	/** Manually assigned surface on the other side. Pair both actors reciprocally. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Replicated, Category = "Mirror Transfer")
	TObjectPtr<ATimelineTransferItem> LinkedTransfer;

	/** Timeline/world represented by this side of the bound mirror. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Mirror Transfer")
	EItemTimeline TransferTimeline = EItemTimeline::Both;

	/** Which local X side of TransferVolume the source character must stand on. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Mirror Transfer")
	bool bSourceOnPositiveX = true;

	/** Also activates the Niagara component on the paired destination surface. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mirror Transfer|VFX")
	bool bActivateVFXOnLinkedTransfer = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_TransferVFXActive,
		Category = "Mirror Transfer|VFX")
	bool bTransferVFXActive = false;

	/** Lightweight replicated visual/interaction proxy class. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mirror Transfer")
	TSubclassOf<AMirrorTransferPreview> PreviewClass;

	/** Idle held-item scan rate. No item actors tick for this system. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mirror Transfer|Performance",
		meta = (ClampMin = "0.05", Units = "s"))
	float IdleScanInterval = 0.10f;

	/** Transform replication rate used only while a preview exists. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mirror Transfer|Performance",
		meta = (ClampMin = "0.02", Units = "s"))
	float ActiveUpdateInterval = 0.05f;

	/** Maximum server-authoritative distance from target player to preview. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mirror Transfer|Validation",
		meta = (ClampMin = "50.0", Units = "cm"))
	float MaxTakeDistance = 350.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mirror Transfer|Debug")
	bool bDrawDebugTransferVolume = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "Mirror Transfer|Runtime")
	TObjectPtr<ABase_Item> ActiveItem;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "Mirror Transfer|Runtime")
	TObjectPtr<AMirrorTransferPreview> ActivePreview;

	/** Called only by the server-side preview interaction. */
	bool CompleteMirrorTransfer(AHronoCharacter* TargetCharacter, AMirrorTransferPreview* RequestingPreview);

	/** Stable mirrored/local mapping from this surface to LinkedTransfer. */
	FTransform MapItemTransformToLinked(const FTransform& ItemWorldTransform) const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool TryPickUp(AHronoCharacter* Character) override { return false; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnRep_TransferVFXActive();

	UFUNCTION()
	void OnTransferVFXSystemFinished(UNiagaraComponent* FinishedComponent);

private:
	UPROPERTY()
	TObjectPtr<AHronoCharacter> SourceCharacter;

	/** Prevents a just-received item from immediately previewing back through this side. */
	UPROPERTY()
	TObjectPtr<ABase_Item> CompletedItemAwaitingExit;

	FTimerHandle MonitorTimerHandle;
	bool bLocalVFXRequested = false;
	bool bLinkedVFXRequested = false;

	void MonitorTransferVolume();
	void BeginMirrorPreview(ABase_Item* Item, AHronoCharacter* InSourceCharacter);
	void UpdateActivePreview();
	void CancelMirrorTransfer(const TCHAR* Reason);
	void SetMonitorInterval(float Interval);
	void SetLocalTransferVFXActive(bool bActive);
	void SetLinkedTransferVFXActive(bool bActive);
	void RefreshTransferVFXState();
	void ApplyTransferVFXState();

	bool IsItemInsideTransferVolume(const ABase_Item& Item) const;
	bool IsSourceOnCorrectSide(const AHronoCharacter& Character) const;
	bool IsValidTransferPair() const;
	bool IsValidPreviewSource(const ABase_Item& Item, const AHronoCharacter& Character) const;
	AHronoCharacter* FindTargetCharacter(const AHronoCharacter& InSourceCharacter) const;
};
