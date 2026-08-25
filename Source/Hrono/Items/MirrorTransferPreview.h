#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HronoSharedTools.h"
#include "Interface/Enviroment_Interface.h"
#include "MirrorTransferPreview.generated.h"

class ABase_Item;
class AHronoCharacter;
class ATimelineTransferItem;
class USceneComponent;
class UStaticMeshComponent;

/** Replicated visual + interaction proxy. It is deliberately not a pickup item. */
UCLASS(Blueprintable, NotPlaceable)
class HRONO_API AMirrorTransferPreview : public AActor, public IEnviroment_Interface
{
	GENERATED_BODY()

public:
	AMirrorTransferPreview();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mirror Transfer")
	TObjectPtr<USceneComponent> DefaultSceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mirror Transfer")
	TObjectPtr<UStaticMeshComponent> PreviewMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	FText InteractionText;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_PreviewData, Category = "Mirror Transfer")
	TObjectPtr<ABase_Item> SourceItem;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_PreviewData, Category = "Mirror Transfer")
	TObjectPtr<AHronoCharacter> TargetCharacter;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_PreviewData, Category = "Mirror Transfer")
	EItemTimeline TargetTimeline = EItemTimeline::Both;

	void InitializePreview(
		ATimelineTransferItem* InSourceTransfer,
		ABase_Item* InSourceItem,
		AHronoCharacter* InTargetCharacter,
		EItemTimeline InTargetTimeline);

	UFUNCTION(BlueprintPure, Category = "Interaction")
	FText GetInteractionText() const { return InteractionText; }

	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_PreviewData();

private:
	UPROPERTY(Replicated)
	TObjectPtr<ATimelineTransferItem> SourceTransfer;

	void RefreshPreviewVisuals();
	void RefreshLocalVisibility();
};
