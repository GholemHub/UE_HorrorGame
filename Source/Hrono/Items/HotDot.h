#pragma once

#include "CoreMinimal.h"
#include "Items/Base_Item.h"
#include "HotDot.generated.h"

/**
 * A world-space point detected by the dosimeter.
 *
 * The previous bp_HotDot asset only supplied the marker's visual components;
 * timeline matching and filtering now live here so native gameplay code can work
 * with HotDots without depending on a Blueprint-generated class.
 */
UCLASS(BlueprintType, Blueprintable)
class HRONO_API AHotDot : public ABase_Item
{
	GENERATED_BODY()

public:
	AHotDot();

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Enables or disables this detector point. Server only. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "HotDot")
	void SetHotDotActive(bool bNewActive);

	UFUNCTION(BlueprintPure, Category = "HotDot")
	bool IsHotDotActive() const { return bHotDotActive; }

	/** Runtime state assigned by the owning room. */
	UPROPERTY(ReplicatedUsing = OnRep_HotDotActive, VisibleInstanceOnly, BlueprintReadOnly,
		Category = "HotDot")
	bool bHotDotActive = true;

	/** When enabled, no dosimeter can detect this HotDot in either timeline. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "HotDot|Dosimeter")
	bool bIgnoredByDosimeter = false;

	/** True when this point can affect a dosimeter assigned to ViewerTimeline. */
	UFUNCTION(BlueprintPure, Category = "HotDot|Dosimeter")
	bool IsDetectableByDosimeter(EItemTimeline ViewerTimeline) const;

	/** Optional Blueprint hook for lights, particles, or other active-state feedback. */
	UFUNCTION(BlueprintImplementableEvent, Category = "HotDot|Events",
		meta = (DisplayName = "Receive HotDot Active Changed"))
	void ReceiveHotDotActiveChanged(bool bNewActive);

	/** HotDots are detector targets, not pickup items. */
	virtual bool TryPickUp(AHronoCharacter* Character) override;

protected:
	UFUNCTION()
	void OnRep_HotDotActive();
};
