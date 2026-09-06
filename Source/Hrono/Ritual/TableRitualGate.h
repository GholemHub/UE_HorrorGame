#pragma once

#include "CoreMinimal.h"

class ABase_Item;
class AChair;
class AHronoCharacter;

/** Server-authoritative match gate for the table ritual. */
namespace TableRitualGate
{
	/** Permanently unlocks the current world when BP_CursedImage_Item reaches a hand. */
	void NotifySuccessfulPickup(const ABase_Item& Item, const AHronoCharacter& Character);

	/** True after the cursed image has been picked up in this world. */
	bool IsUnlocked(const UObject* WorldContextObject);

	/** Ordinary chairs stay usable; only chairs beside BP_TableRitualManager are gated. */
	bool CanUseChair(const AChair& Chair);
}
