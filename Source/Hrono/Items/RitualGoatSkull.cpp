#include "Items/RitualGoatSkull.h"

#include "Components/StaticMeshComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "GameplayTagContainer.h"
#include "Net/UnrealNetwork.h"
#include "Ritual/CursedRoomRitual.h"

DEFINE_LOG_CATEGORY_STATIC(LogRitualSkull, Log, All);

ARitualGoatSkull::ARitualGoatSkull()
{
	ItemTimeline = EItemTimeline::Both;
	ItemTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Item.Ritual.GoatSkull"), false));

	// Actor physics replication follows the root rigid body. This also prevents
	// this skull's mesh from detaching from a non-physical scene root on drop.
	ItemMesh->SetupAttachment(nullptr);
	SetRootComponent(ItemMesh);
	DefaultSceneRoot->SetupAttachment(ItemMesh);
	ItemMesh->SetIsReplicated(true);

	DestructibleSkull = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("DestructibleSkull"));
	DestructibleSkull->SetupAttachment(ItemMesh);
	DestructibleSkull->SetVisibility(false, true);
	DestructibleSkull->SetHiddenInGame(true, true);
	DestructibleSkull->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DestructibleSkull->SetGenerateOverlapEvents(false);
	DestructibleSkull->SetSimulatePhysics(false);
}

void ARitualGoatSkull::BeginPlay()
{
	Super::BeginPlay();
}

void ARitualGoatSkull::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ARitualGoatSkull, bDestroyedByRitual);
}

bool ARitualGoatSkull::TryPickUp(AHronoCharacter* Character)
{
	return !bRitualLocked && !bDestroyedByRitual && Super::TryPickUp(Character);
}

void ARitualGoatSkull::Drop()
{
	const bool bWasHeld = bIsPickedUp && OwningCharacter != nullptr;
	Super::Drop();

	if (bWasHeld && HasAuthority() && !bIsPickedUp)
	{
		if (ACursedRoomRitual* Ritual = ACursedRoomRitual::FindRitual(this))
		{
			Ritual->NotifySkullDropped(this);
		}
	}
}

void ARitualGoatSkull::UpdateVisibilityForLocalPlayer(EItemTimeline ViewerTimeline)
{
	Super::UpdateVisibilityForLocalPlayer(ViewerTimeline);
	if (!bDestroyedByRitual)
	{
		return;
	}

	const bool bVisibleInTimeline =
		ItemTimeline == EItemTimeline::Both || ItemTimeline == ViewerTimeline;
	if (ItemMesh)
	{
		ItemMesh->SetVisibility(false, true);
	}
	if (DestructibleSkull)
	{
		const bool bShowDebris = bChaosDestructionActivated && bVisibleInTimeline;
		DestructibleSkull->SetVisibility(bShowDebris, true);
		DestructibleSkull->SetHiddenInGame(!bShowDebris, true);
	}
}

void ARitualGoatSkull::SetRitualLocked(bool bLocked)
{
	bRitualLocked = bLocked;
	SetReplicateMovement(true);
}

void ARitualGoatSkull::SetRitualKinematic()
{
	SetRitualLocked(true);
	if (ItemMesh)
	{
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ARitualGoatSkull::SetRitualPhysics(bool bReverseGravity)
{
	SetRitualLocked(true);
	SetActorEnableCollision(true);
	if (ItemMesh)
	{
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ConfigureDroppedCollision(ItemMesh);
		ItemMesh->SetEnableGravity(!bReverseGravity);
		ItemMesh->SetSimulatePhysics(true);
		ItemMesh->WakeAllRigidBodies();
	}
}

void ARitualGoatSkull::RestoreNormalGravity()
{
	if (ItemMesh && !bDestroyedByRitual)
	{
		ItemMesh->SetEnableGravity(true);
		ItemMesh->SetSimulatePhysics(true);
		ItemMesh->WakeAllRigidBodies();
	}
}

void ARitualGoatSkull::ExplodeFromRitual()
{
	if (!HasAuthority() || bDestroyedByRitual)
	{
		return;
	}

	bDestroyedByRitual = true;
	ActivateChaosDestruction();
	ForceNetUpdate();

	UE_LOG(LogRitualSkull, Log, TEXT("[RitualSkull] %s exploded (Timeline=%s)"),
		*GetName(),
		*StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(ItemTimeline)));

	if (DebrisLifetime > 0.0f)
	{
		SetLifeSpan(DebrisLifetime);
	}
}

void ARitualGoatSkull::OnRep_DestroyedByRitual()
{
	if (bDestroyedByRitual)
	{
		ActivateChaosDestruction();
	}
}

void ARitualGoatSkull::ActivateChaosDestruction()
{
	if (bChaosDestructionActivated)
	{
		return;
	}

	bChaosDestructionActivated = true;
	SetRitualLocked(true);

	if (ItemMesh)
	{
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ItemMesh->SetVisibility(false, true);
	}

	if (DestructibleSkull && DestructibleSkull->GetRestCollection())
	{
		DestructibleSkull->SetHiddenInGame(false, true);
		DestructibleSkull->SetVisibility(true, true);
		DestructibleSkull->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ConfigureDroppedCollision(DestructibleSkull);
		DestructibleSkull->SetSimulatePhysics(true);
		DestructibleSkull->CrumbleActiveClusters();
	}
	else
	{
		UE_LOG(LogRitualSkull, Warning,
			TEXT("[RitualSkull] %s has no Geometry Collection assigned; using hidden-mesh fallback"),
			*GetName());
	}

	BP_OnRitualExplosion();
}
