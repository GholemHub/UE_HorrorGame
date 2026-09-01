#include "Items/RitualGoatSkull.h"

#include "Components/StaticMeshComponent.h"
#include "GameplayTagContainer.h"

ARitualGoatSkull::ARitualGoatSkull()
{
	ItemTimeline = EItemTimeline::Past;
	ItemTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Item.Ritual.GoatSkull"), false));
}

bool ARitualGoatSkull::TryPickUp(AHronoCharacter* Character)
{
	return !bRitualLocked && Super::TryPickUp(Character);
}

void ARitualGoatSkull::SetRitualLocked(bool bLocked)
{
	bRitualLocked = bLocked;
	SetReplicateMovement(!bLocked);
	if (!IsValid(ItemMesh) || bIsPickedUp)
	{
		return;
	}

	ItemMesh->SetSimulatePhysics(!bLocked);
	ItemMesh->SetEnableGravity(!bLocked);
	ItemMesh->SetCollisionEnabled(
		bLocked ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics);

	if (!bLocked)
	{
		SetActorEnableCollision(true);
		ConfigureDroppedCollision(ItemMesh);
	}
}
