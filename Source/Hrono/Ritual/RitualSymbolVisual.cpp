#include "Ritual/RitualSymbolVisual.h"

#include "Components/SceneComponent.h"

ARitualSymbolVisual::ARitualSymbolVisual()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;
	SetReplicateMovement(false);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

void ARitualSymbolVisual::InitializeSymbol(
	int32 NewSlotIndex,
	const FString& NewSymbol,
	EItemTimeline NewVisibleTimeline)
{
	SlotIndex = NewSlotIndex;
	Symbol = NewSymbol;
	VisibleTimeline = NewVisibleTimeline;
	BP_OnSymbolInitialized(SlotIndex, Symbol, VisibleTimeline);
}
