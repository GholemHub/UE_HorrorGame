#pragma once

#include "CoreMinimal.h"
#include "Items/Base_Item.h"
#include "RitualGoatSkull.generated.h"

/** Existing pickup/drop item used by the Past side of the cursed-room ritual. */
UCLASS(BlueprintType, Blueprintable)
class HRONO_API ARitualGoatSkull : public ABase_Item
{
	GENERATED_BODY()

public:
	ARitualGoatSkull();

	virtual bool TryPickUp(AHronoCharacter* Character) override;

	/** Called by the replicated ritual state on the server and every client. */
	void SetRitualLocked(bool bLocked);

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursed Room Ritual|Skull")
	void BP_OnWrongRoomReaction();

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursed Room Ritual|Skull")
	void BP_OnSkullStartFloating();

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursed Room Ritual|Skull")
	void BP_OnSkullStartRotating();

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursed Room Ritual|Skull")
	void BP_OnSkullStartScratching();

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursed Room Ritual|Skull")
	void BP_OnSkullRitualCompleted();

	UFUNCTION(BlueprintPure, Category = "Cursed Room Ritual|Skull")
	bool IsRitualLocked() const { return bRitualLocked; }

private:
	bool bRitualLocked = false;
};
