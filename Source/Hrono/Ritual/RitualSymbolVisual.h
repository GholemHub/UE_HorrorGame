#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HronoSharedTools.h"
#include "RitualSymbolVisual.generated.h"

class USceneComponent;

/**
 * Client-local, replaceable presentation actor for one revealed ritual symbol.
 *
 * A Blueprint child may add a Decal Component, mesh, Niagara system, or any
 * other presentation. The authoritative ritual sends only the symbol that the
 * owning player is allowed to see, then initializes this non-replicated actor.
 */
UCLASS(BlueprintType, Blueprintable)
class HRONO_API ARitualSymbolVisual : public AActor
{
	GENERATED_BODY()

public:
	ARitualSymbolVisual();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ritual Symbol|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Ritual Symbol")
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Ritual Symbol")
	FString Symbol;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Ritual Symbol")
	EItemTimeline VisibleTimeline = EItemTimeline::Both;

	UFUNCTION(BlueprintCallable, Category = "Ritual Symbol")
	void InitializeSymbol(int32 NewSlotIndex, const FString& NewSymbol, EItemTimeline NewVisibleTimeline);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ritual Symbol",
		meta = (DisplayName = "On Ritual Symbol Initialized"))
	void BP_OnSymbolInitialized(int32 NewSlotIndex, const FString& NewSymbol, EItemTimeline NewVisibleTimeline);
};
