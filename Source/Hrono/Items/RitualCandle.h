#pragma once

#include "CoreMinimal.h"
#include "Interface/Enviroment_Interface.h"
#include "Items/Base_Item.h"
#include "RitualCandle.generated.h"

class ACursedRoomRitual;

/**
 * Existing-style pickup item and the Future player's ritual activation surface.
 * If the cooperative placement is not ready, normal E-key pickup still occurs.
 */
UCLASS(BlueprintType, Blueprintable)
class HRONO_API ARitualCandle : public ABase_Item, public IEnviroment_Interface
{
	GENERATED_BODY()

public:
	ARitualCandle();

	virtual bool TryPickUp(AHronoCharacter* Character) override;
	virtual void Interact_Implementation(AActor* Interactor) override;

	void SetRitualLocked(bool bLocked);

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursed Room Ritual|Candle")
	void BP_OnCandleIgnited();

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursed Room Ritual|Candle")
	void BP_OnCandleFlameIncreased();

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursed Room Ritual|Candle")
	void BP_OnCandleExtinguished();

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursed Room Ritual|Candle")
	void BP_OnWrongRoomReaction();

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursed Room Ritual|Candle")
	void BP_OnCandleRitualCompleted();

	UFUNCTION(BlueprintPure, Category = "Cursed Room Ritual|Candle")
	bool IsRitualLocked() const { return bRitualLocked; }

private:
	bool bRitualLocked = false;
};
