#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

class ABase_Item;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HRONO_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

protected:
	virtual void BeginPlay() override;
	// Required to register replicated properties
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	void DoNextItem();
	void DoPreviousItem();

	bool AddItem(ABase_Item* Item);
	void RemoveItem(ABase_Item* Item);

	UFUNCTION(BlueprintCallable)
	ABase_Item* GetCurrentItem() const;


	static constexpr int32 MaxItems = 3;

	// Replicated state array
	UPROPERTY(ReplicatedUsing = OnRep_Items)
	TArray<ABase_Item*> Items;

	// Replicated index tracking active slot
	UPROPERTY(ReplicatedUsing = OnRep_CurrentItemIndex)
	int32 CurrentItemIndex = INDEX_NONE;


	void SelectItem(int32 NewIndex);
	void UpdateItemVisibility();

	// RepNotify functions to handle visuals on clients when server updates values
	UFUNCTION()
	void OnRep_Items();

	UFUNCTION()
	void OnRep_CurrentItemIndex();
};