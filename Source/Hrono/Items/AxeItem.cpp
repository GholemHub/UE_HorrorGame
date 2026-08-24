#include "Items/AxeItem.h"

AAxeItem::AAxeItem()
{
	ItemType = EItemType::Tool;
	ItemName = NSLOCTEXT("HronoItems", "AxeName", "Axe");
	ItemDescription = NSLOCTEXT(
		"HronoItems",
		"AxeDescription",
		"A sturdy axe capable of breaking boards barricading doors.");

	// A useful starting pose for a Blueprint child. It remains fully editable.
	HoldOffset = FTransform(
		FRotator(0.0f, 90.0f, -20.0f),
		FVector(10.0f, 8.0f, -12.0f),
		FVector::OneVector);
}
