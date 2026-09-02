#pragma once

#include "CoreMinimal.h"
#include "Items/Base_Item.h"
#include "RitualGoatSkull.generated.h"

class UGeometryCollectionComponent;

/** One shared skull class; ItemTimeline selects whether an instance belongs to Past or Future. */
UCLASS(BlueprintType, Blueprintable)
class HRONO_API ARitualGoatSkull : public ABase_Item
{
	GENERATED_BODY()

public:
	ARitualGoatSkull();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool TryPickUp(AHronoCharacter* Character) override;
	virtual void Drop() override;
	virtual void UpdateVisibilityForLocalPlayer(EItemTimeline ViewerTimeline) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cursed Room Ritual|Skull")
	TObjectPtr<UGeometryCollectionComponent> DestructibleSkull;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Skull",
		meta = (ClampMin = "0.0", Units = "s"))
	float DebrisLifetime = 10.0f;

	void SetRitualLocked(bool bLocked);
	void SetRitualKinematic();
	void SetRitualPhysics(bool bReverseGravity);
	void RestoreNormalGravity();
	void ExplodeFromRitual();

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursed Room Ritual|Skull")
	void BP_OnWrongRoomReaction();

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursed Room Ritual|Skull")
	void BP_OnSkullStartFloating();

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursed Room Ritual|Skull")
	void BP_OnSkullStartScratching();

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursed Room Ritual|Skull")
	void BP_OnRitualExplosion();

	UFUNCTION(BlueprintPure, Category = "Cursed Room Ritual|Skull")
	bool IsRitualLocked() const { return bRitualLocked; }

	UFUNCTION(BlueprintPure, Category = "Cursed Room Ritual|Skull")
	bool WasDestroyedByRitual() const { return bDestroyedByRitual; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_DestroyedByRitual();

private:
	void ActivateChaosDestruction();

	UPROPERTY(ReplicatedUsing = OnRep_DestroyedByRitual)
	bool bDestroyedByRitual = false;

	bool bRitualLocked = false;
	bool bChaosDestructionActivated = false;
};
