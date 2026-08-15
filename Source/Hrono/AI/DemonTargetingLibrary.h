#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DemonTargetingLibrary.generated.h"

class AHronoCharacter;

/** Multiplayer-safe targeting helpers used by Demon/StateTree Blueprints. */
UCLASS()
class HRONO_API UDemonTargetingLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Finds the player character currently closest to Hunter.
	 *
	 * This function deliberately recalculates the minimum on every call. It does
	 * not retain the previous frame's distance, so a newly closer player can
	 * immediately replace the current target.
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Demon Targeting",
		meta = (WorldContext = "WorldContextObject", DisplayName = "Find Closest Player Target",
			AdvancedDisplay = "bIgnoreSafePlayers"))
	static bool FindClosestPlayerTarget(
		const UObject* WorldContextObject,
		const AActor* Hunter,
		AHronoCharacter*& OutTarget,
		float& OutDistance,
		bool bIgnoreSafePlayers = false);
};
