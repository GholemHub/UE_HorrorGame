#pragma once

#include "CoreMinimal.h"
#include "Items/Base_Item.h"
#include "TimelineTransferItem.generated.h"

class UBoxComponent;

/**
 * A placed item-transfer station. Any other ABase_Item entering TransferBox is
 * moved from Past to Future or Future to Past. Items assigned Both are unchanged.
 */
UCLASS()
class HRONO_API ATimelineTransferItem : public ABase_Item
{
	GENERATED_BODY()

public:
	ATimelineTransferItem();

	/** Volume in which dropped or carried base items are transferred. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> TransferBox;

	/** If enabled, an item assigned Both is transferred to Past. Otherwise it is left unchanged. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline Transfer")
	bool bTransferBothTimelineItems = false;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTransferBoxBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnTransferBoxEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

private:
	TSet<TWeakObjectPtr<ABase_Item>> ItemsInTransferBox;

	void TransferItem(ABase_Item& Item);
};
