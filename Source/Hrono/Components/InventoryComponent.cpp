#include "Components/InventoryComponent.h"
#include "Items/Base_Item.h"
#include "Net/UnrealNetwork.h" // Required for DOREPLIFETIME

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	// CRITICAL FIX: Prevent garbage data values from breaking array checks
	CurrentItemIndex = INDEX_NONE;
}
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Register variables for lifetime network replication
	DOREPLIFETIME(UInventoryComponent, Items);
	DOREPLIFETIME(UInventoryComponent, CurrentItemIndex);
}

void UInventoryComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UInventoryComponent::AddItem(ABase_Item* Item)
{
	// Ensure modifications only originate on Authority (Server)
	if (!GetOwner()->HasAuthority()) return false;

	if (!Item || Items.Num() >= MaxItems)
	{
		return false;
	}

	Items.Add(Item);

	if (CurrentItemIndex == INDEX_NONE)
	{
		CurrentItemIndex = 0;
	}

	// Server updates locally
	UpdateItemVisibility();
	return true;
}

void UInventoryComponent::RemoveItem(ABase_Item* Item)
{
	// Ensure modifications only originate on Authority (Server)
	if (!GetOwner()->HasAuthority()) return;

	if (!Item) return;

	const int32 RemovedIndex = Items.Find(Item);
	if (RemovedIndex == INDEX_NONE) return;

	Items.RemoveAt(RemovedIndex);

	if (Items.Num() == 0)
	{
		CurrentItemIndex = INDEX_NONE;
		UpdateItemVisibility();
		return;
	}

	if (CurrentItemIndex >= Items.Num())
	{
		CurrentItemIndex = 0;
	}

	UpdateItemVisibility();
}

void UInventoryComponent::DoNextItem()
{
	if (Items.Num() <= 1) return;

	const int32 NewIndex = (CurrentItemIndex + 1) % Items.Num();
	SelectItem(NewIndex);
}

void UInventoryComponent::DoPreviousItem()
{
	if (Items.Num() <= 1) return;

	const int32 NewIndex = (CurrentItemIndex - 1 + Items.Num()) % Items.Num();
	SelectItem(NewIndex);
}

ABase_Item* UInventoryComponent::GetCurrentItem() const
{
	if (!Items.IsValidIndex(CurrentItemIndex))
	{
		return nullptr;
	}
	return Items[CurrentItemIndex];
}

void UInventoryComponent::SelectItem(int32 NewIndex)
{
	// If input is triggered from local client, we should request server switch
	if (!GetOwner()->HasAuthority())
	{
		// To make slot-switching bulletproof across networks, call a Server RPC from the Character!
		return;
	}

	if (!Items.IsValidIndex(NewIndex)) return;

	CurrentItemIndex = NewIndex;
	UpdateItemVisibility();
}

void UInventoryComponent::UpdateItemVisibility()
{
	for (int32 i = 0; i < Items.Num(); i++)
	{
		ABase_Item* Item = Items[i];
		if (!Item) continue;

		const bool bIsCurrentItem = (i == CurrentItemIndex);

		Item->SetActorHiddenInGame(!bIsCurrentItem);
		Item->SetActorEnableCollision(false);
	}
}

// Network RepNotifies running on clients when Server changes data fields
void UInventoryComponent::OnRep_Items()
{
	UpdateItemVisibility();
}

void UInventoryComponent::OnRep_CurrentItemIndex()
{
	UpdateItemVisibility();
}