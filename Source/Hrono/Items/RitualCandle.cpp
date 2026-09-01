#include "Items/RitualCandle.h"

#include "Components/StaticMeshComponent.h"
#include "GameplayTagContainer.h"
#include "HronoCharacter.h"
#include "Ritual/CursedRoomRitual.h"

ARitualCandle::ARitualCandle()
{
	ItemTimeline = EItemTimeline::Future;
	ItemTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Item.Ritual.Candle"), false));
}

bool ARitualCandle::TryPickUp(AHronoCharacter* Character)
{
	return !bRitualLocked && Super::TryPickUp(Character);
}

void ARitualCandle::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority() || bRitualLocked || bIsPickedUp)
	{
		return;
	}

	AHronoCharacter* Character = Cast<AHronoCharacter>(Interactor);
	if (!IsValid(Character) || Character->GetTimeline() != EItemTimeline::Future)
	{
		return;
	}

	if (ACursedRoomRitual* Ritual = ACursedRoomRitual::FindRitual(this))
	{
		Ritual->TryActivateRitual(Character, this);
	}
}

void ARitualCandle::SetRitualLocked(bool bLocked)
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
