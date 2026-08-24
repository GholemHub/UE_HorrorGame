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

	/** Allows Blueprint variants to create decorative axes that cannot break boards. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axe")
	bool bCanBreakBarricades = true;

	UFUNCTION(BlueprintPure, Category = "Axe")
	bool CanBreakBarricades() const { return bCanBreakBarricades; }
};
