#pragma once

#include "CoreMinimal.h"
#include "HronoTutorialTypes.generated.h"

/** Equipment pages supported by the in-game tutorial journal. */
UENUM(BlueprintType)
enum class EHronoTutorialItem : uint8
{
	None UMETA(Hidden),
	Monocle UMETA(DisplayName = "Monocle"),
	Dosimeter UMETA(DisplayName = "Dosimeter"),
	Skull UMETA(DisplayName = "Skull"),
	Axe UMETA(DisplayName = "Axe"),
	// Appended to preserve serialized numeric values of the existing item entries.
	Clock UMETA(DisplayName = "Clock")
};
