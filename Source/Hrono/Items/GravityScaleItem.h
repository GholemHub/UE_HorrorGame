#pragma once

#include "CoreMinimal.h"
#include "Items/Base_Item.h"
#include "GravityScaleItem.generated.h"

class ARoom;
class UTextRenderComponent;

/** A server-authored scale reading; kilograms are mass, newtons are weight. */
USTRUCT(BlueprintType)
struct FGravityScaleReading
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Gravity Scale")
	TObjectPtr<ABase_Item> Item = nullptr;
	UPROPERTY(BlueprintReadOnly, Category = "Gravity Scale")
	float MassKg = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "Gravity Scale")
	float GravityMS2 = 9.81f;
	UPROPERTY(BlueprintReadOnly, Category = "Gravity Scale")
	float WeightNewtons = 0.0f;
	/** What a scale calibrated at 9.81 m/s² would display in kg. */
	UPROPERTY(BlueprintReadOnly, Category = "Gravity Scale")
	float ApparentKg = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "Gravity Scale")
	bool bCursedRoom = false;
};

/** Pickable scale. A loose Base_Item must actually rest on its physics platform. */
UCLASS(Blueprintable)
class HRONO_API AGravityScaleItem : public ABase_Item
{
	GENERATED_BODY()

public:
	AGravityScaleItem();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual bool TryPickUp(AHronoCharacter* Character) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "Gravity Scale")
	FGravityScaleReading GetReading() const { return Reading; }

	/** Visible readout follows the physical platform, including after a throw. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gravity Scale")
	TObjectPtr<UTextRenderComponent> Display;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Scale|Detection",
		meta = (ClampMin = "0.05", Units = "s"))
	float SampleInterval = 0.2f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Scale|Detection",
		meta = (ClampMin = "10.0", Units = "cm"))
	float MaximumItemHeight = 80.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Scale|Debug")
	bool bDebugOnScreen = true;

private:
	UPROPERTY(ReplicatedUsing = OnRep_Reading)
	FGravityScaleReading Reading;

	FTimerHandle SampleTimer;
	FTimerHandle DebugTimer;

	UFUNCTION()
	void OnRep_Reading();
	void SamplePlatform();
	void ShowLocalDebug();
	void UpdateDisplay();
	void SetReading(const FGravityScaleReading& NewReading);
	ABase_Item* FindSupportedItem() const;
	const ARoom* FindContainingRoom() const;
};
