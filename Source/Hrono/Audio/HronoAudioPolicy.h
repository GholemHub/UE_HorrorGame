#pragma once
#include "CoreMinimal.h"
#include "Items/Base_Item.h"

namespace HronoAudioPolicy
{
    /** One local gameplay listener per process. Unknown/unpossessed listeners hear no world audio. */
    HRONO_API bool CanHear(const UObject* Context, EItemTimeline EventTimeline);
}
