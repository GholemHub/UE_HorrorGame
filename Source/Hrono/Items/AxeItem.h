#pragma once

#include "CoreMinimal.h"
#include "Items/Base_Item.h"
#include "AxeItem.generated.h"

/**
 * Pick-up axe used to break DoorBarricadeBoard actors. Pickup, held attachment,
 * dropping, timeline filtering and replication are inherited from Base_Item.
 */
UCLASS(BlueprintType, Blueprintable)
class HRONO_API AAxeItem : public ABase_Item
{
	GENERATED_BODY()

public:
	AAxeItem();

	virtual void Tick(float DeltaSeconds) override;
	virtual void Use_Implementation(AActor* Character) override;

	/**
	 * Starts one server-authoritative swing. Returns false while the axe is still
	 * returning to its held pose or is inside the short recovery window.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Axe|Swing")
	bool TryStartSwing();

	/** Rotation around the held actor's local Y axis at the bottom of the swing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axe|Swing",
		meta = (ClampMin = "-180.0", ClampMax = "180.0", Units = "deg"))
	float SwingAngleY = 90.0f;

	/** Time used to slowly raise the axe from the swing pose back to its held pose. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axe|Swing",
		meta = (ClampMin = "0.05", Units = "s"))
	float SwingReturnDuration = 0.8f;

	/** Extra lockout after the axe reaches its held pose. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axe|Swing",
		meta = (ClampMin = "0.0", Units = "s"))
	float SwingRecoveryDuration = 0.15f;

	UFUNCTION(BlueprintPure, Category = "Axe|Swing")
	bool IsSwingAnimating() const { return bSwingAnimationActive; }

	/** Allows Blueprint variants to create decorative axes that cannot break boards. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axe")
	bool bCanBreakBarricades = true;

	UFUNCTION(BlueprintPure, Category = "Axe")
	bool CanBreakBarricades() const { return bCanBreakBarricades; }

protected:
	UFUNCTION(NetMulticast, Reliable)
	void MulticastStartSwing();

private:
	bool bSwingAnimationActive = false;
	float SwingAnimationElapsed = 0.0f;
	float NextAllowedSwingTime = 0.0f;
};
