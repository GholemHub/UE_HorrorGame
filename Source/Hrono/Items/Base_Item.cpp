// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Base_Item.h"
#include "Items/Clock.h"
#include "Items/Drag_Item.h"
#include "Items/Rune_Item.h"
#include "Audio/HronoAudioPolicy.h"
#include "Net/UnrealNetwork.h"
#include "GameplayTagsManager.h"
#include "HronoCharacter.h"
#include "AI/MannequinDemon.h"
#include "Ritual/TableRitualGate.h"
#include "Camera/CameraComponent.h"
#include "Components/MeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SceneComponent.h"
#include "Components/HeldItemInertiaComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
TAutoConsoleVariable<int32> CVarHeldTransformLogging(
	TEXT("hrono.Debug.HeldTransform"), 0,
	TEXT("Log detailed held-item transforms during pickup; 0 by default."));
}

FBaseItemServerDroppedSignature ABase_Item::OnServerDropped;

// Sets default values
ABase_Item::ABase_Item()
{

	bReplicates = true;
	SetReplicateMovement(true);

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;
	DefaultSceneRoot->SetMobility(EComponentMobility::Movable);

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(DefaultSceneRoot);
	ItemMesh->SetMobility(EComponentMobility::Movable);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ItemMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	HeldItemInertia = CreateDefaultSubobject<UHeldItemInertiaComponent>(TEXT("HeldItemInertia"));

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> InteractionOverlayFinder(
		TEXT("/Game/_Alex/Materials/M_InteractionOverlay.M_InteractionOverlay"));
	if (InteractionOverlayFinder.Succeeded())
	{
		InteractionOverlayMaterial = InteractionOverlayFinder.Object;
	}

}

void ABase_Item::SetInteractionOverlayAllowed(bool bAllowed)
{
	bAllowInteractionOverlay = bAllowed;
	RefreshInteractionHighlight();
}

void ABase_Item::SetInteractionHighlighted(bool bHighlighted)
{
	bInteractionHovered = bHighlighted;
	RefreshInteractionHighlight();
}

void ABase_Item::SetInteractionContextHighlighted(bool bHighlighted)
{
	bInteractionContextHighlighted = bHighlighted;
	RefreshInteractionHighlight();
}

void ABase_Item::SetInteractionHighlightForced(bool bForced)
{
	if (!HasAuthority() || bForceInteractionHighlight == bForced)
	{
		return;
	}

	bForceInteractionHighlight = bForced;
	RefreshInteractionHighlight();
	ForceNetUpdate();
}

void ABase_Item::OnRep_ForceInteractionHighlight()
{
	RefreshInteractionHighlight();
}

void ABase_Item::RefreshInteractionHighlight()
{
	const bool bHighlightRequested = bForceInteractionHighlight
		|| bInteractionContextHighlighted
		|| (bInteractionHovered && AllowsAimInteractionHighlight());
	const bool bShouldHighlight = bAllowInteractionOverlay
		&& bHighlightRequested
		&& IsValid(InteractionOverlayMaterial)
		&& !bIsPickedUp
		&& !IsHidden();
	TInlineComponentArray<UMeshComponent*> MeshComponents(this);
	if (bShouldHighlight)
	{
		if (!bInteractionHighlighted)
		{
			PreviousOverlayMaterials.Reset();
		}

		for (UMeshComponent* MeshComponent : MeshComponents)
		{
			if (!IsValid(MeshComponent))
			{
				continue;
			}

			// Blueprints can replace meshes or clear their overlay while the player is
			// still aiming at the actor. Cache newly discovered components and repair
			// the render state instead of trusting only the previous boolean state.
			if (!PreviousOverlayMaterials.Contains(MeshComponent))
			{
				PreviousOverlayMaterials.Add(MeshComponent, MeshComponent->GetOverlayMaterial());
			}
			if (MeshComponent->GetOverlayMaterial() != InteractionOverlayMaterial)
			{
				MeshComponent->SetOverlayMaterial(InteractionOverlayMaterial);
			}
		}
	}
	else if (bInteractionHighlighted || !PreviousOverlayMaterials.IsEmpty())
	{
		for (UMeshComponent* MeshComponent : MeshComponents)
		{
			if (!IsValid(MeshComponent))
			{
				continue;
			}

			const TWeakObjectPtr<UMaterialInterface>* PreviousMaterial =
				PreviousOverlayMaterials.Find(MeshComponent);
			MeshComponent->SetOverlayMaterial(
				PreviousMaterial ? PreviousMaterial->Get() : nullptr);
		}
		PreviousOverlayMaterials.Reset();
	}

	bInteractionHighlighted = bShouldHighlight;
}

bool ABase_Item::AllowsAimInteractionHighlight() const
{
	// Regular Base_Item actors are pickup items by default. Clocks and the current
	// painting Blueprints share the Clock item category and must always be eligible.
	return bUseInteractionHighlight || ItemType == EItemType::Clock;
}

void ABase_Item::EnsureInteractionOverlayMaterial()
{
	if (IsValid(InteractionOverlayMaterial))
	{
		return;
	}

	// Some existing Blueprint CDOs can retain a serialized null value for a native
	// property added later. Resolve the project default at runtime as a safe fallback.
	InteractionOverlayMaterial = LoadObject<UMaterialInterface>(
		nullptr,
		TEXT("/Game/_Alex/Materials/M_InteractionOverlay.M_InteractionOverlay"));
}

bool ABase_Item::CanHighlightFor(const AHronoCharacter* Viewer) const
{
	return IsValid(Viewer)
		&& CanBePickedUp()
		&& bAllowInteractionOverlay
		&& AllowsAimInteractionHighlight()
		&& IsValid(InteractionOverlayMaterial)
		&& UsableValid
		&& !bIsPickedUp
		&& !IsHidden()
		&& (ItemTimeline == EItemTimeline::Both || ItemTimeline == Viewer->GetTimeline());
}


void ABase_Item::Use_Implementation(AActor* Character)
{
	//UE_LOG(LogTemp, Warning, TEXT("Default Use"));
}

void ABase_Item::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABase_Item, OwningCharacter);
	DOREPLIFETIME(ABase_Item, MannequinCarrier);
	DOREPLIFETIME(ABase_Item, ItemTimeline);
	DOREPLIFETIME(ABase_Item, MirrorTransferState);
	DOREPLIFETIME(ABase_Item, bDroppedPhysicsEnabled);
	DOREPLIFETIME(ABase_Item, bFloatingPickupEnabled);
	DOREPLIFETIME(ABase_Item, bForceInteractionHighlight);
}

void ABase_Item::SetMirrorTransferState(EMirrorItemTransferState NewState)
{
	if (!HasAuthority() || MirrorTransferState == NewState)
	{
		return;
	}

	MirrorTransferState = NewState;
	OnMirrorTransferStateChanged(MirrorTransferState);
	ForceNetUpdate();
}

void ABase_Item::OnRep_MirrorTransferState()
{
	OnMirrorTransferStateChanged(MirrorTransferState);
}

void ABase_Item::SetItemTimeline(EItemTimeline NewTimeline)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Item] %s SetItemTimeline attempted on non-authority, ignoring"), *GetName());
		return;
	}

	if (ItemTimeline == NewTimeline)
	{
		// Deferred Blueprint spawning can assign the same value as the class
		// default. The components still need their local visibility/collision
		// initialized before the actor is finished spawning.
		ApplyItemTimelineState();
		return;
	}

	ItemTimeline = NewTimeline;
	CurrentCachedTimeline = EItemTimeline::Both;
	ApplyItemTimelineState();
	ForceNetUpdate();
}

void ABase_Item::OnRep_ItemTimeline()
{
	CurrentCachedTimeline = EItemTimeline::Both;
	ApplyItemTimelineState();
}

void ABase_Item::EnableDroppedPhysics()
{
	if (!HasAuthority() || IsPlacementLocked() || IsValid(OwningCharacter) || bIsPickedUp)
	{
		return;
	}

	bDroppedPhysicsEnabled = true;
	bFloatingPickupEnabled = false;
	ApplyWorldItemState();
	ForceNetUpdate();
}

void ABase_Item::EnableFloatingPickup()
{
	if (!HasAuthority() || IsPlacementLocked() || IsValid(OwningCharacter) || bIsPickedUp)
	{
		return;
	}

	bDroppedPhysicsEnabled = false;
	bFloatingPickupEnabled = true;
	ApplyWorldItemState();
	ForceNetUpdate();
}

void ABase_Item::OnRep_FloatingPickupEnabled()
{
	ApplyWorldItemState();
}

void ABase_Item::OnRep_DroppedPhysicsEnabled()
{
	ApplyWorldItemState();
}

void ABase_Item::UpdateMeshForLocalPlayer()
{
	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC) return;

	auto Character = Cast<AHronoCharacter>(PC->GetPawn());
	if (!Character) return;

	EItemTimeline TargetTimeline = Character->GetTimeline();
	if (CurrentCachedTimeline == TargetTimeline)
	{
		return;
	}

	UpdateVisibilityForLocalPlayer(TargetTimeline);
}

void ABase_Item::UpdateVisibilityForLocalPlayer(EItemTimeline ViewerTimeline)
{
	const AHronoCharacter* Viewer = nullptr;
	if (const APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
		Viewer = Cast<AHronoCharacter>(PC->GetPawn());
	const ABase_Item* ViewerHeld = IsValid(Viewer) ? Viewer->GetHeldItem() : nullptr;
	const bool bThroughMannequinLens = IsValid(MannequinCarrier) && IsValid(Viewer)
		&& ViewerTimeline != MannequinCarrier->MannequinTimeline && IsValid(ViewerHeld)
		&& ViewerHeld->bCanRepelMannequin && ViewerHeld->OwningCharacter == Viewer
		&& ViewerHeld->bIsPickedUp
		&& ViewerHeld->FindComponentByClass<USceneCaptureComponent2D>();
	const bool bShouldBeVisible = (ItemTimeline == EItemTimeline::Both || ItemTimeline == ViewerTimeline)
		|| bThroughMannequinLens;

	// Do not rely on root propagation here. A primitive that starts with physics
	// enabled can be detached from the scene root, and Blueprint item classes can
	// also contain scene components outside the native root hierarchy. Updating
	// every scene component keeps spawned and placed items consistent.
	TInlineComponentArray<USceneComponent*> SceneComponents(this);
	for (USceneComponent* SceneComponent : SceneComponents)
	{
		if (IsValid(SceneComponent))
		{
			SceneComponent->SetVisibility(bShouldBeVisible, /*bPropagateToChildren=*/false);
			if (IsValid(MannequinCarrier) || bLastMannequinCarryPresentation)
				if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(SceneComponent))
					Primitive->SetVisibleInSceneCaptureOnly(bThroughMannequinLens);
		}
	}

	CurrentCachedTimeline = ViewerTimeline;
	bLastMannequinCarryPresentation = IsValid(MannequinCarrier);
}

bool ABase_Item::TryPickUp(AHronoCharacter* Character)
{
	if (!HasAuthority() || !CanBePickedUp() || !IsValid(Character)
		|| IsActorBeingDestroyed() || IsPlacementLocked())
	{
		return false;
	}

	// Pickup never transfers an occupied item. Server RPCs are processed serially,
	// so the first accepted request claims it before a competing request can run.
	if (bIsPickedUp || OwningCharacter != nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("[Item] Pickup rejected for %s: %s is already held or reserved by %s"),
			*GetNameSafe(Character), *GetName(), *GetNameSafe(OwningCharacter));
		return false;
	}

	// PickupItem reserves this hand before attachment can invoke Blueprint events.
	// Also reject direct native calls when another item already occupies the hand.
	if ((IsValid(Character->GetHeldItem()) && Character->GetHeldItem() != this)
		|| (ItemTimeline != EItemTimeline::Both && ItemTimeline != Character->GetTimeline()))
	{
		return false;
	}
	if (IsValid(MannequinCarrier))
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		MannequinCarrier = nullptr;
		CurrentCachedTimeline = EItemTimeline::Both;
		bDroppedPhysicsEnabled = false;
		SetReplicateMovement(true);
	}

	OnPickedUp(Character);
	if (!bIsPickedUp && !IsValid(OwningCharacter))
	{
		bDroppedPhysicsEnabled = true;
		ApplyWorldItemState();
	}

	return bIsPickedUp && OwningCharacter == Character;
}

bool ABase_Item::TryCarryByMannequin(AMannequinDemon* Carrier, USceneComponent* HandPoint)
{
	if (!HasAuthority() || !IsValid(Carrier) || !IsValid(HandPoint) || !CanBePickedUp()
		|| Carrier->State == EMannequinState::Dormant || Carrier->State == EMannequinState::Disabled
		|| IsValid(Carrier->CarriedItem)
		|| IsActorBeingDestroyed() || IsPlacementLocked() || bIsPickedUp
		|| IsValid(OwningCharacter) || IsValid(MannequinCarrier)
		|| MirrorTransferState != EMirrorItemTransferState::None
		|| (ItemTimeline != EItemTimeline::Both && ItemTimeline != Carrier->MannequinTimeline)) return false;
	MannequinCarrier = Carrier;
	bDroppedPhysicsEnabled = false;
	bFloatingPickupEnabled = false;
	if (ItemTimeline == EItemTimeline::Both) SetItemTimeline(Carrier->MannequinTimeline);
	ApplyMannequinCarryState();
	if (GetAttachParentActor() != Carrier)
	{
		MannequinCarrier = nullptr;
		bDroppedPhysicsEnabled = true;
		ApplyWorldItemState();
		return false;
	}
	ForceNetUpdate();
	return true;
}

void ABase_Item::DropFromMannequin(const FVector& ThrowVelocity)
{
	if (!HasAuthority() || !IsValid(MannequinCarrier)) return;
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	MannequinCarrier = nullptr;
	CurrentCachedTimeline = EItemTimeline::Both;
	bDroppedPhysicsEnabled = true;
	SetReplicateMovement(true);
	ApplyWorldItemState();
	if (IsValid(ItemMesh) && ItemMesh->IsSimulatingPhysics() && !ThrowVelocity.IsNearlyZero())
	{
		ItemMesh->AddImpulse(ThrowVelocity * ItemMesh->GetMass());
	}
	ForceNetUpdate();
}

void ABase_Item::OnRep_MannequinCarrier()
{
	CurrentCachedTimeline = EItemTimeline::Both;
	ApplyWorldItemState();
}

void ABase_Item::ApplyMannequinCarryState()
{
	if (!IsValid(MannequinCarrier) || IsPlacementLocked()) return;
	USceneComponent* HandPoint = MannequinCarrier->ItemHandPoint;
	if (!IsValid(HandPoint)) return;
	if (IsValid(ItemMesh))
	{
		ItemMesh->SetSimulatePhysics(false);
		RestoreItemMeshAttachment();
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		ConfigureDroppedCollision(ItemMesh);
	}
	SetActorEnableCollision(true);
	SetReplicateMovement(false);
	AttachToComponent(HandPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	SetActorHiddenInGame(false);
	SetHeldSceneCapturesEnabled(false);
	RefreshItemTickEnabled(false);
	CurrentCachedTimeline = EItemTimeline::Both;
	UpdateMeshForLocalPlayer();
}

bool ABase_Item::CanEnterGravityAnomaly() const
{
	return HasAuthority() && CanBePickedUp() && !IsActorBeingDestroyed()
		&& !IsA<ADrag_Item>() && !IsA<AClock>() && !IsA<ARune_Item>()
		&& !IsPlacementLocked() && !IsValid(OwningCharacter) && !IsValid(MannequinCarrier) && !bIsPickedUp
		&& !bFloatingPickupEnabled && MirrorTransferState == EMirrorItemTransferState::None
		&& IsValid(ItemMesh) && ItemMesh != GetRootComponent()
		&& ItemMesh->GetStaticMesh() != nullptr;
}

bool ABase_Item::AttachToCharacter()
{
	if (IsPlacementLocked())
	{
		ApplyWorldItemState();
		return false;
	}
	bInteractionHovered = false;
	SetHeldSceneCapturesEnabled(false);

	if (HasAuthority())
	{
		bDroppedPhysicsEnabled = false;
		bFloatingPickupEnabled = false;
	}

	TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents(this);
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (!IsValid(PrimitiveComponent))
		{
			continue;
		}

		PrimitiveComponent->SetSimulatePhysics(false);
		PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PrimitiveComponent->SetMobility(EComponentMobility::Movable);
	}

	auto Player = Cast<AHronoCharacter>(OwningCharacter);
	if (!Player) {
		UE_LOG(LogTemp, Warning, TEXT("[Item] %s has no owning character"), *GetName());

		return false;
	}

	// While held, transforms are cosmetic and are derived independently on each
	// machine. Dropped movement replication is restored in DetachFromCharacter.
	SetReplicateMovement(false);

	// Chaos detaches a simulated child mesh and leaves its relative transform in
	// world space. Restore the authored pose before applying the held offset.
	RestoreItemMeshAttachment();

	if (!RefreshHeldAttachmentPoint())
	{
		SetReplicateMovement(true);
		return false;
	}

	bIsPickedUp = true;
	RefreshItemTickEnabled(bTemporaryItemTickActive);
	RefreshInteractionHighlight();
	UpdateMeshForLocalPlayer();
	if (HasAuthority())
	{
		// Replicate the held owner and disabled dropped-physics state together as
		// soon as possible. This is important for runtime-spawned physics items.
		ForceNetUpdate();
	}
	OnHeldStateChanged(true, Player);
	SetHeldSceneCapturesEnabled(Player->IsLocallyControlled());

	TableRitualGate::NotifySuccessfulPickup(*this, *Player);
	return true;
}

bool ABase_Item::RefreshHeldAttachmentPoint()
{
	AHronoCharacter* Player = Cast<AHronoCharacter>(OwningCharacter);
	USceneComponent* TargetPoint = Player ? Player->GetHeldItemInteractionPoint(this) : nullptr;
	if (!IsValid(Player) || !IsValid(TargetPoint))
	{
		UE_LOG(LogTemp, Error,
			TEXT("[Item] Failed to resolve held attachment point for %s (Owner=%s)"),
			*GetName(), *GetNameSafe(OwningCharacter));
		return false;
	}

	LogHeldTransformState(TEXT("BeforeAttach"));

	const bool bAttached = AttachToComponent(
		TargetPoint,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	if (!bAttached)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[Item] Failed to attach %s to %s"), *GetName(), *GetNameSafe(TargetPoint));
		return false;
	}
	LogHeldTransformState(TEXT("AfterSnap"));

	// HoldOffset controls only the held pose. Preserve the item's scale so it
	// cannot inherit a different scale from the character or change after drop.
	if (USceneComponent* Root = GetRootComponent())
	{
		FVector HoldLocation = bUseCenteredInteractionPoint
			? FVector::ZeroVector : HoldOffset.GetLocation();
		if (!bUseCenteredInteractionPoint && Player->GetTimeline() == EItemTimeline::Past)
		{
			// The past view is mirrored, so mirror the held item's longitudinal
			// location offset as well (for example, Future X=-10 becomes Past X=+10).
			HoldLocation.Y *= -1.0f;
		}

		Root->SetRelativeLocationAndRotation(
			HoldLocation,
			HoldOffset.GetRotation().Rotator());

		if (IsValid(HeldItemInertia))
		{
			if (bUseCenteredInteractionPoint) HeldItemInertia->EndHeld(false);
			else HeldItemInertia->BeginHeld(Player, Root->GetRelativeTransform());
		}
	}
	LogHeldTransformState(TEXT("AfterHoldPose"));

	UE_LOG(LogTemp, Log,
		TEXT("[Item] Attached %s to held point %s"), *GetName(), *GetNameSafe(TargetPoint));
	return true;
}

void ABase_Item::LogHeldTransformState(const TCHAR* Context) const
{
#if !UE_BUILD_SHIPPING
	if (CVarHeldTransformLogging.GetValueOnGameThread() == 0) return;
	const AHronoCharacter* Player = Cast<AHronoCharacter>(OwningCharacter);
	const USceneComponent* Root = GetRootComponent();
	const USceneComponent* Anchor = Player ? Player->GetHeldItemInteractionPoint(this) : nullptr;
	const USceneComponent* FuturePoint = Player ? Player->InteractionPoint.Get() : nullptr;
	const USceneComponent* PastPoint = Player ? Player->PastInteractionPoint.Get() : nullptr;
	const UCameraComponent* Camera = Player ? Player->GetFirstPersonCameraComponent() : nullptr;

	const FVector RootWorld = Root ? Root->GetComponentLocation() : FVector::ZeroVector;
	const FVector AnchorWorld = Anchor ? Anchor->GetComponentLocation() : FVector::ZeroVector;
	const FVector MeshWorld = ItemMesh ? ItemMesh->GetComponentLocation() : FVector::ZeroVector;
	const FVector BoundsWorld = ItemMesh ? ItemMesh->Bounds.Origin : FVector::ZeroVector;
	const FVector CameraWorld = Camera ? Camera->GetComponentLocation() : FVector::ZeroVector;
	const FVector FutureWorld = FuturePoint ? FuturePoint->GetComponentLocation() : FVector::ZeroVector;
	const FVector PastWorld = PastPoint ? PastPoint->GetComponentLocation() : FVector::ZeroVector;

	UE_LOG(LogTemp, Warning,
		TEXT("[HeldTransform] %s Item=%s Authority=%d LocalOwner=%d ItemTimeline=%s Player=%s PlayerTimeline=%s "
			"RootParent=%s MeshParent=%s MeshPhysics=%d"),
		Context,
		*GetName(),
		HasAuthority() ? 1 : 0,
		Player && Player->IsLocallyControlled() ? 1 : 0,
		*StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(ItemTimeline)),
		*GetNameSafe(Player),
		Player
			? *StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(Player->GetTimeline()))
			: TEXT("None"),
		*GetNameSafe(Root ? Root->GetAttachParent() : nullptr),
		*GetNameSafe(ItemMesh ? ItemMesh->GetAttachParent() : nullptr),
		ItemMesh && ItemMesh->IsSimulatingPhysics() ? 1 : 0);

	UE_LOG(LogTemp, Warning,
		TEXT("[HeldTransform] %s World Root=%s Anchor=%s DeltaRootAnchor=%s Mesh=%s DeltaMeshAnchor=%s "
			"Bounds=%s DeltaBoundsAnchor=%s"),
		Context,
		*RootWorld.ToCompactString(),
		*AnchorWorld.ToCompactString(),
		*(RootWorld - AnchorWorld).ToCompactString(),
		*MeshWorld.ToCompactString(),
		*(MeshWorld - AnchorWorld).ToCompactString(),
		*BoundsWorld.ToCompactString(),
		*(BoundsWorld - AnchorWorld).ToCompactString());

	UE_LOG(LogTemp, Warning,
		TEXT("[HeldTransform] %s Relative Root=%s Mesh=%s HoldOffset=%s | Camera=%s FuturePoint=%s "
			"PastPoint=%s PastMinusFuture=%s AnchorMinusCamera=%s"),
		Context,
		Root ? *Root->GetRelativeLocation().ToCompactString() : TEXT("None"),
		ItemMesh ? *ItemMesh->GetRelativeLocation().ToCompactString() : TEXT("None"),
		*HoldOffset.GetLocation().ToCompactString(),
		*CameraWorld.ToCompactString(),
		*FutureWorld.ToCompactString(),
		*PastWorld.ToCompactString(),
		*(PastWorld - FutureWorld).ToCompactString(),
		*(AnchorWorld - CameraWorld).ToCompactString());
#endif
}
#include "Items/Dozimetr.h"

void ABase_Item::OnPickedUp(AHronoCharacter* Character)
{
	SetInteractionHighlightForced(false);
	OwningCharacter = Character;

	// Set native network ownership to allow safe attachment replication
	SetOwner(Character);

	if (!AttachToCharacter())
	{
		SetOwner(nullptr);
		OwningCharacter = nullptr;
		return;
	}
	MulticastItemSound(true, ItemTimeline, GetActorLocation());
	UE_LOG(LogTemp, Warning, TEXT("PickUp"));
	auto Dozimetr = Cast<ADozimetr>(this);
	if (Dozimetr) {
		Dozimetr->On();
	}

}


void ABase_Item::MulticastItemSound_Implementation(bool bPickup,
	EItemTimeline EventTimeline, FVector_NetQuantize Location)
{
	if (HronoAudioPolicy::CanHear(this, EventTimeline))
	{
		UGameplayStatics::PlaySoundAtLocation(this, bPickup ? PickupSound : DropSound, Location);
	}
}

void ABase_Item::OnRep_OwningCharacter(AHronoCharacter* PreviousOwningCharacter)
{
	if (IsValid(OwningCharacter) && !IsPlacementLocked())
	{
		if (IsValid(PreviousOwningCharacter) && PreviousOwningCharacter != OwningCharacter)
		{
			OnHeldStateChanged(false, PreviousOwningCharacter);
		}
		AttachToCharacter();
		return;
	}

	if (IsValid(HeldItemInertia))
	{
		HeldItemInertia->EndHeld(false);
	}
	bIsPickedUp = false;
	SetReplicateMovement(true);

	// Attachment replication may already have attached the rune to its slot.
	// Only remove the old hand attachment; never detach a placed rune's parent.
	if (!IsPlacementLocked() && IsValid(PreviousOwningCharacter)
		&& GetAttachParentActor() == PreviousOwningCharacter)
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}
	SetActorHiddenInGame(false);
	ApplyWorldItemState();
	OnHeldStateChanged(false, PreviousOwningCharacter);
	SetHeldSceneCapturesEnabled(false);
}

void ABase_Item::Drop()
{
	if (!HasAuthority() || !bIsPickedUp || !IsValid(OwningCharacter) || IsPlacementLocked())
	{
		return;
	}

	AHronoCharacter* PreviousOwner = OwningCharacter;
	DetachFromCharacter();
	bIsPickedUp = false;
	SetOwner(nullptr);
	OwningCharacter = nullptr;
	bDroppedPhysicsEnabled = true;
	bFloatingPickupEnabled = false;
	ApplyWorldItemState();

	if (ADozimetr* Dozimetr = Cast<ADozimetr>(this))
	{
		Dozimetr->Off();
	}
	MulticastItemSound(false, ItemTimeline, GetActorLocation());
	OnHeldStateChanged(false, PreviousOwner);
	// Blueprint callbacks may reactivate a capture. The native dropped state wins.
	SetHeldSceneCapturesEnabled(false);
	ForceNetUpdate();
	OnServerDropped.Broadcast(this);
}

bool ABase_Item::ReleaseForPlacement(AHronoCharacter* Character)
{
	if (!HasAuthority() || !IsValid(Character) || OwningCharacter != Character || !bIsPickedUp)
	{
		return false;
	}

	DetachFromCharacter();
	bIsPickedUp = false;
	OwningCharacter = nullptr;
	SetOwner(nullptr);
	bDroppedPhysicsEnabled = false;
	bFloatingPickupEnabled = false;
	ApplyWorldItemState();
	if (ADozimetr* Dozimetr = Cast<ADozimetr>(this))
	{
		Dozimetr->Off();
	}
	OnHeldStateChanged(false, Character);
	SetHeldSceneCapturesEnabled(false);
	ForceNetUpdate();
	return true;
}

void ABase_Item::DetachFromCharacter()
{
	if (IsValid(HeldItemInertia))
	{
		HeldItemInertia->EndHeld(false);
	}
	SetReplicateMovement(true);
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetActorHiddenInGame(false);
	SetHeldSceneCapturesEnabled(false);
}

void ABase_Item::SetHeldSceneCapturesEnabled(bool bEnabled)
{
	if (!bOnlyRunSceneCaptureWhileLocallyHeld && !bUseCenteredInteractionPoint)
	{
		return;
	}

	const AHronoCharacter* HeldBy = Cast<AHronoCharacter>(OwningCharacter);
	const bool bShouldCapture = bEnabled && bIsPickedUp && IsValid(HeldBy)
		&& HeldBy->IsLocallyControlled() && !IsPlacementLocked()
		&& GetNetMode() != NM_DedicatedServer;
	TInlineComponentArray<USceneCaptureComponent2D*> SceneCaptures(this);
	for (USceneCaptureComponent2D* SceneCapture : SceneCaptures)
	{
		if (!IsValid(SceneCapture))
		{
			continue;
		}

		// Capture-on-movement is redundant while capturing every held frame and can
		// otherwise wake a dropped item when physics or replication moves it.
		SceneCapture->bCaptureEveryFrame = bShouldCapture;
		SceneCapture->bCaptureOnMovement = false;
		SceneCapture->SetComponentTickEnabled(bShouldCapture);

		if (bShouldCapture)
		{
			SceneCapture->Activate(true);
		}
		else
		{
			SceneCapture->Deactivate();
		}
	}
	SetSceneCaptureDisplayMaterialEnabled(bShouldCapture);
}

void ABase_Item::CacheSceneCaptureDisplayMaterial()
{
	if (bSceneCaptureDisplayMaterialCached || SceneCaptureDisplayMaterialIndex == INDEX_NONE
		|| !IsValid(ItemMesh) || !IsValid(ItemMesh->GetStaticMesh())
		|| GetNetMode() == NM_DedicatedServer)
	{
		return;
	}
	bSceneCaptureDisplayMaterialCached = true;
	if (SceneCaptureDisplayMaterialIndex < 0
		|| SceneCaptureDisplayMaterialIndex >= ItemMesh->GetNumMaterials())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Item] Invalid capture material slot %d on %s"),
			SceneCaptureDisplayMaterialIndex, *GetName());
		return;
	}
	UMaterialInterface* ActiveMaterial = ItemMesh->GetMaterial(SceneCaptureDisplayMaterialIndex);
	UMaterialInterface* DormantMaterial = ItemMesh->GetStaticMesh()->GetMaterial(SceneCaptureDisplayMaterialIndex);
	if (IsValid(ActiveMaterial) && IsValid(DormantMaterial) && ActiveMaterial != DormantMaterial)
	{
		ActiveSceneCaptureDisplayMaterial = ActiveMaterial;
	}
}

void ABase_Item::SetSceneCaptureDisplayMaterialEnabled(bool bEnabled)
{
	CacheSceneCaptureDisplayMaterial();
	if (!IsValid(ActiveSceneCaptureDisplayMaterial) || !IsValid(ItemMesh)
		|| !IsValid(ItemMesh->GetStaticMesh())) return;
	UMaterialInterface* DesiredMaterial = bEnabled
		? ActiveSceneCaptureDisplayMaterial.Get()
		: ItemMesh->GetStaticMesh()->GetMaterial(SceneCaptureDisplayMaterialIndex);
	if (IsValid(DesiredMaterial) && ItemMesh->GetMaterial(SceneCaptureDisplayMaterialIndex) != DesiredMaterial)
	{
		ItemMesh->SetMaterial(SceneCaptureDisplayMaterialIndex, DesiredMaterial);
	}
}

void ABase_Item::ConfigureDroppedCollision(UPrimitiveComponent* PrimitiveComponent)
{
	if (!IsValid(PrimitiveComponent))
	{
		return;
	}

	PrimitiveComponent->SetCollisionObjectType(COLLISION_CHANNEL_ITEM);

	const bool bVisibleToPast =
		ItemTimeline == EItemTimeline::Both || ItemTimeline == EItemTimeline::Past;
	const bool bVisibleToFuture =
		ItemTimeline == EItemTimeline::Both || ItemTimeline == EItemTimeline::Future;

	// These channels are also used by PerformInteractTrace. Keep the appropriate
	// response blocking so a dropped item remains pickable. The character capsule
	// ignores COLLISION_CHANNEL_ITEM, preventing physical character/item collision.
	PrimitiveComponent->SetCollisionResponseToChannel(
		COLLISION_CHANNEL_PAWN_PAST,
		bVisibleToPast ? ECR_Block : ECR_Ignore);
	PrimitiveComponent->SetCollisionResponseToChannel(
		COLLISION_CHANNEL_PAWN_FUTURE,
		bVisibleToFuture ? ECR_Block : ECR_Ignore);
}

void ABase_Item::ApplyDroppedPhysicsState()
{
	// Physics simulation detaches a non-root mesh. A held item must always remain
	// controlled by AttachToCharacter even if replication callbacks are reordered.
	if (!bDroppedPhysicsEnabled || IsPlacementLocked() || IsValid(OwningCharacter) || bIsPickedUp)
	{
		return;
	}

	SetReplicateMovement(true);
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetActorEnableCollision(true);
	if (UStaticMeshComponent* Mesh = GetItemMesh())
	{
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ConfigureDroppedCollision(Mesh);
		Mesh->SetEnableGravity(true);
		// FRepMovement replicates the root, not a detached child rigid body.
		// Only the server simulates child meshes; clients render them attached to
		// the replicated root. Mesh-root items retain Unreal's physics replication.
		const bool bSimulate = HasAuthority() || Mesh == GetRootComponent();
		Mesh->SetSimulatePhysics(bSimulate);
		if (!bSimulate)
		{
			RestoreItemMeshAttachment();
		}
	}
	UpdateMeshForLocalPlayer();
}

void ABase_Item::RestoreItemMeshAttachment()
{
	if (IsValid(ItemMesh) && IsValid(GetRootComponent()) && ItemMesh != GetRootComponent())
	{
		if (ItemMesh->GetAttachParent() != GetRootComponent())
		{
			ItemMesh->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		}
		ItemMesh->SetRelativeTransform(ItemMeshRelativeTransform);
	}
}

void ABase_Item::ApplyWorldItemState()
{
	if (IsPlacementLocked())
	{
		if (HasAuthority())
		{
			bDroppedPhysicsEnabled = false;
			bFloatingPickupEnabled = false;
		}
		bIsPickedUp = false;
		SetReplicateMovement(true); // Replicate the slot attachment, including to late joiners.
		SetActorHiddenInGame(false);
		SetActorEnableCollision(false);
		if (IsValid(HeldItemInertia))
		{
			HeldItemInertia->EndHeld(false);
		}
		TInlineComponentArray<UPrimitiveComponent*> Primitives(this);
		for (UPrimitiveComponent* Primitive : Primitives)
		{
			Primitive->SetSimulatePhysics(false);
			Primitive->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		RestoreItemMeshAttachment();
		SetHeldSceneCapturesEnabled(false);
	}
	else if (IsValid(MannequinCarrier))
	{
		ApplyMannequinCarryState();
		return;
	}
	else if (IsValid(OwningCharacter) || bIsPickedUp)
	{
		return; // The owner callback applies the held pose and its cosmetic event once.
	}
	else if (bFloatingPickupEnabled)
	{
		ApplyFloatingPickupState();
	}
	else if (bDroppedPhysicsEnabled)
	{
		ApplyDroppedPhysicsState();
	}
	RefreshItemTickEnabled(bTemporaryItemTickActive);
}

bool ABase_Item::NeedsDroppedPhysicsTracking() const
{
	return HasAuthority() && bDroppedPhysicsEnabled && !bFloatingPickupEnabled
		&& !IsPlacementLocked() && !IsValid(OwningCharacter) && !bIsPickedUp
		&& IsValid(ItemMesh) && ItemMesh != GetRootComponent();
}

void ABase_Item::SyncDroppedPhysicsTransform()
{
	if (!NeedsDroppedPhysicsTracking() || !ItemMesh->IsSimulatingPhysics())
	{
		return;
	}
	// Physics detached the child mesh. Derive the actor pose that reproduces the
	// authored mesh offset on clients, without moving the server's rigid body.
	const FTransform MeshWorld = ItemMesh->GetComponentTransform();
	const FQuat RootRotation = MeshWorld.GetRotation() * ItemMeshRelativeTransform.GetRotation().Inverse();
	const FVector RootScale = GetActorScale3D();
	const FVector RootLocation = MeshWorld.GetLocation()
		- RootRotation.RotateVector(RootScale * ItemMeshRelativeTransform.GetLocation());
	const FTransform RootWorld(RootRotation, RootLocation, RootScale);
	if (!GetActorTransform().Equals(RootWorld))
	{
		SetActorTransform(RootWorld, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

void ABase_Item::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	SyncDroppedPhysicsTransform();
}

void ABase_Item::GatherCurrentMovement()
{
	// Also refresh immediately before a network sample (e.g. initial relevance).
	SyncDroppedPhysicsTransform();
	Super::GatherCurrentMovement();
}

void ABase_Item::OnRep_ReplicatedMovement()
{
	// An old physics sample must not start root-body simulation after pickup or
	// placement. These states receive their pose from the hand/slot attachment.
	if (!IsPlacementLocked() && !IsValid(OwningCharacter))
	{
		Super::OnRep_ReplicatedMovement();
	}
}

void ABase_Item::OnRep_AttachmentReplication()
{
	Super::OnRep_AttachmentReplication();
	ApplyWorldItemState();
}

void ABase_Item::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	AuthoredTickGroup = PrimaryActorTick.TickGroup;
	EnsureMovableComponentHierarchy();

	// Cache the Blueprint-authored mesh pose before initial replicated properties
	// can enable Chaos physics. BeginPlay is too late for runtime-spawned items on
	// clients: physics may already have converted this relative transform into a
	// world-space value while detaching/re-attaching the non-root mesh.
	if (IsValid(ItemMesh))
	{
		ItemMeshRelativeTransform = ItemMesh->GetRelativeTransform();
	}

	// Blueprint-created captures are already registered by this point, but the
	// world has not started ticking yet. Disable them before the first game frame.
	CacheSceneCaptureDisplayMaterial();
	SetHeldSceneCapturesEnabled(false);
}

void ABase_Item::EnsureMovableComponentHierarchy()
{
	TSet<USceneComponent*> VisitedComponents;
	TFunction<void(USceneComponent*)> MakeMovableChildFirst;
	MakeMovableChildFirst = [this, &VisitedComponents, &MakeMovableChildFirst](
		USceneComponent* Component)
	{
		if (!IsValid(Component)
			|| Component->GetOwner() != this
			|| VisitedComponents.Contains(Component))
		{
			return;
		}

		VisitedComponents.Add(Component);
		TArray<USceneComponent*> Children;
		Component->GetChildrenComponents(false, Children);
		for (USceneComponent* Child : Children)
		{
			MakeMovableChildFirst(Child);
		}

		Component->SetMobility(EComponentMobility::Movable);
	};

	TInlineComponentArray<USceneComponent*> SceneComponents(this);
	for (USceneComponent* SceneComponent : SceneComponents)
	{
		MakeMovableChildFirst(SceneComponent);
	}
}

// Called when the game starts or when spawned
void ABase_Item::BeginPlay()
{
	Super::BeginPlay();
	EnsureInteractionOverlayMaterial();
	ApplyItemTimelineState();
	ApplyWorldItemState();

	const AHronoCharacter* HeldBy = Cast<AHronoCharacter>(OwningCharacter);
	SetHeldSceneCapturesEnabled(
		bIsPickedUp && IsValid(HeldBy) && HeldBy->IsLocallyControlled());
	RefreshItemTickEnabled();
}

bool ABase_Item::HasBlueprintTickImplementation() const
{
	const UFunction* TickFunction = GetClass()->FindFunctionByName(
		GET_FUNCTION_NAME_CHECKED(AActor, ReceiveTick));
	return IsValid(TickFunction)
		&& TickFunction->GetOuterUClass() != AActor::StaticClass();
}

void ABase_Item::RefreshItemTickEnabled(bool bTemporaryNativeActivity)
{
	bTemporaryItemTickActive = bTemporaryNativeActivity;
	const bool bTrackDroppedPhysics = NeedsDroppedPhysicsTracking();
	SetTickGroup(bTrackDroppedPhysics ? TG_PostPhysics : AuthoredTickGroup);
	SetActorTickEnabled(
		bTrackDroppedPhysics
		|| bTemporaryNativeActivity
		|| RequiresContinuousItemTick()
		|| HasBlueprintTickImplementation());
}

void ABase_Item::ApplyItemTimelineState()
{
	if (ItemTimeline == EItemTimeline::Future && FutureMesh)
	{
		ItemMesh->SetStaticMesh(FutureMesh);
		ItemTag = UGameplayTagsManager::Get().RequestGameplayTag(FName("Item.Future"));
	}
	else if (ItemTimeline == EItemTimeline::Past && PastMesh)
	{
		ItemMesh->SetStaticMesh(PastMesh);
		ItemTag = UGameplayTagsManager::Get().RequestGameplayTag(FName("Item.Past"));
	}


	if (ItemTimeline == EItemTimeline::Future)
	{


		ItemMesh->SetCollisionObjectType(COLLISION_CHANNEL_ITEM);
		ItemMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, ECR_Block);
		ItemMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, ECR_Ignore);
	}
	else if (ItemTimeline == EItemTimeline::Past)
	{


		ItemMesh->SetCollisionObjectType(COLLISION_CHANNEL_ITEM);
		ItemMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, ECR_Block);
		ItemMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, ECR_Ignore);

	}
	else // EItemTimeline::Both — blocks all pawns
	{


		ItemMesh->SetCollisionObjectType(COLLISION_CHANNEL_ITEM);
		ItemMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, ECR_Block);
		ItemMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, ECR_Block);
	}

	// Refresh visibility immediately after a timeline change instead of waiting for Tick.
	UpdateMeshForLocalPlayer();
}

void ABase_Item::ApplyFloatingPickupState()
{
	if (!bFloatingPickupEnabled || IsPlacementLocked() || IsValid(OwningCharacter) || bIsPickedUp)
	{
		return;
	}

	SetReplicateMovement(true);
	SetActorEnableCollision(true);
	SetActorHiddenInGame(false);
	if (UStaticMeshComponent* Mesh = GetItemMesh())
	{
		Mesh->SetSimulatePhysics(false);
		Mesh->SetEnableGravity(false);
		RestoreItemMeshAttachment();
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		ConfigureDroppedCollision(Mesh);
	}
	UpdateMeshForLocalPlayer();
}
