#include "Items/TimelineTransferItem.h"

#include "HronoCharacter.h"
#include "Items/MirrorTransferPreview.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "TimerManager.h"

namespace MirrorTransfer
{
	static const TCHAR* LogPrefix = TEXT("[MirrorTransfer]");
}

ATimelineTransferItem::ATimelineTransferItem()
{
	bReplicates = true;
	SetReplicateMovement(false);
	PrimaryActorTick.bCanEverTick = false;

	TransferBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TransferVolume"));
	TransferBox->SetupAttachment(DefaultSceneRoot);
	TransferBox->SetBoxExtent(FVector(8.0f, 100.0f, 100.0f));
	TransferBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TransferBox->SetGenerateOverlapEvents(false);
	TransferBox->SetHiddenInGame(true);
	TransferBox->bDrawOnlyIfSelected = true;

	TransferPoint = CreateDefaultSubobject<USceneComponent>(TEXT("TransferPoint"));
	TransferPoint->SetupAttachment(DefaultSceneRoot);

	TransferVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TransferVFX"));
	TransferVFX->SetupAttachment(TransferPoint);
	TransferVFX->SetAutoActivate(false);

	ItemTimeline = EItemTimeline::Both;
	PreviewClass = AMirrorTransferPreview::StaticClass();
}

void ATimelineTransferItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATimelineTransferItem, LinkedTransfer);
	DOREPLIFETIME(ATimelineTransferItem, ActiveItem);
	DOREPLIFETIME(ATimelineTransferItem, ActivePreview);
	DOREPLIFETIME(ATimelineTransferItem, bTransferVFXActive);
}

void ATimelineTransferItem::BeginPlay()
{
	Super::BeginPlay();
	if (!HasAuthority())
	{
		ApplyTransferVFXState();
		return;
	}
	ApplyTransferVFXState();

	if (!IsValidTransferPair())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s Invalid LinkedTransfer on %s"),
			MirrorTransfer::LogPrefix, *GetNameSafe(this));
	}
	SetMonitorInterval(IdleScanInterval);
}

void ATimelineTransferItem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		CancelMirrorTransfer(TEXT("transfer surface ended play"));
		GetWorldTimerManager().ClearTimer(MonitorTimerHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void ATimelineTransferItem::OnRep_TransferVFXActive()
{
	ApplyTransferVFXState();
}

void ATimelineTransferItem::SetLocalTransferVFXActive(bool bActive)
{
	if (!HasAuthority())
	{
		return;
	}

	bLocalVFXRequested = bActive;
	RefreshTransferVFXState();

	if (bActivateVFXOnLinkedTransfer
		&& IsValid(LinkedTransfer)
		&& LinkedTransfer->LinkedTransfer == this)
	{
		LinkedTransfer->SetLinkedTransferVFXActive(bActive);
	}
}

void ATimelineTransferItem::SetLinkedTransferVFXActive(bool bActive)
{
	if (!HasAuthority())
	{
		return;
	}

	bLinkedVFXRequested = bActive;
	RefreshTransferVFXState();
}

void ATimelineTransferItem::RefreshTransferVFXState()
{
	const bool bShouldBeActive = bLocalVFXRequested || bLinkedVFXRequested;
	if (bTransferVFXActive == bShouldBeActive)
	{
		return;
	}

	bTransferVFXActive = bShouldBeActive;
	ApplyTransferVFXState();
	ForceNetUpdate();
}

void ATimelineTransferItem::ApplyTransferVFXState()
{
	if (!IsValid(TransferVFX))
	{
		return;
	}

	if (bTransferVFXActive)
	{
		TransferVFX->Activate(true);
	}
	else
	{
		TransferVFX->Deactivate();
	}
}

void ATimelineTransferItem::SetMonitorInterval(float Interval)
{
	if (!HasAuthority())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(MonitorTimerHandle);
	GetWorldTimerManager().SetTimer(
		MonitorTimerHandle,
		this,
		&ATimelineTransferItem::MonitorTransferVolume,
		FMath::Max(0.02f, Interval),
		true);
}

void ATimelineTransferItem::MonitorTransferVolume()
{
#if !UE_BUILD_SHIPPING
	if (bDrawDebugTransferVolume && IsValid(TransferBox))
	{
		DrawDebugBox(
			GetWorld(), TransferBox->GetComponentLocation(), TransferBox->GetScaledBoxExtent(),
			TransferBox->GetComponentQuat(), FColor::Cyan, false,
			FMath::Max(IdleScanInterval, ActiveUpdateInterval) * 1.2f, 0, 1.5f);
	}
#endif
	if (CompletedItemAwaitingExit && !IsValid(CompletedItemAwaitingExit))
	{
		CompletedItemAwaitingExit = nullptr;
	}
	if (IsValid(CompletedItemAwaitingExit))
	{
		if (!IsItemInsideTransferVolume(*CompletedItemAwaitingExit))
		{
			if (CompletedItemAwaitingExit->MirrorTransferState == EMirrorItemTransferState::Completed)
			{
				CompletedItemAwaitingExit->SetMirrorTransferState(EMirrorItemTransferState::None);
			}
			CompletedItemAwaitingExit = nullptr;
		}
	}

	if (IsValid(ActiveItem))
	{
		UpdateActivePreview();
		return;
	}

	if (!IsValidTransferPair())
	{
		return;
	}

	for (TActorIterator<AHronoCharacter> It(GetWorld()); It; ++It)
	{
		AHronoCharacter* Character = *It;
		ABase_Item* HeldItem = IsValid(Character) ? Character->GetHeldItem() : nullptr;
		if (IsValid(HeldItem) && IsValidPreviewSource(*HeldItem, *Character))
		{
			BeginMirrorPreview(HeldItem, Character);
			return;
		}
	}
}

bool ATimelineTransferItem::IsValidTransferPair() const
{
	return IsValid(LinkedTransfer)
		&& LinkedTransfer != this
		&& LinkedTransfer->LinkedTransfer == this
		&& TransferTimeline != EItemTimeline::Both
		&& LinkedTransfer->TransferTimeline != EItemTimeline::Both
		&& LinkedTransfer->TransferTimeline != TransferTimeline;
}

bool ATimelineTransferItem::IsSourceOnCorrectSide(const AHronoCharacter& Character) const
{
	if (!IsValid(TransferBox))
	{
		return false;
	}

	const FVector ToCharacter = Character.GetActorLocation() - TransferBox->GetComponentLocation();
	const float Side = FVector::DotProduct(ToCharacter, TransferBox->GetForwardVector());
	return bSourceOnPositiveX ? Side > 0.0f : Side < 0.0f;
}

bool ATimelineTransferItem::IsItemInsideTransferVolume(const ABase_Item& Item) const
{
	if (!IsValid(TransferBox))
	{
		return false;
	}

	const FBox WorldBounds = Item.GetComponentsBoundingBox(true);
	if (!WorldBounds.IsValid)
	{
		return false;
	}

	const FTransform BoxTransform = TransferBox->GetComponentTransform();
	FBox LocalBounds(ForceInit);
	for (int32 X = 0; X < 2; ++X)
	{
		for (int32 Y = 0; Y < 2; ++Y)
		{
			for (int32 Z = 0; Z < 2; ++Z)
			{
				const FVector Corner(
					X ? WorldBounds.Max.X : WorldBounds.Min.X,
					Y ? WorldBounds.Max.Y : WorldBounds.Min.Y,
					Z ? WorldBounds.Max.Z : WorldBounds.Min.Z);
				LocalBounds += BoxTransform.InverseTransformPosition(Corner);
			}
		}
	}

	const FVector Extent = TransferBox->GetUnscaledBoxExtent();
	return LocalBounds.Intersect(FBox(-Extent, Extent));
}

bool ATimelineTransferItem::IsValidPreviewSource(
	const ABase_Item& Item,
	const AHronoCharacter& Character) const
{
	return HasAuthority()
		&& IsValidTransferPair()
		&& Item.bCanTransferThroughMirror
		&& Item.MirrorTransferState == EMirrorItemTransferState::None
		&& Item.OwningCharacter == &Character
		&& Character.GetHeldItem() == &Item
		&& Character.GetTimeline() == TransferTimeline
		&& (Item.ItemTimeline == EItemTimeline::Both || Item.ItemTimeline == TransferTimeline)
		&& IsSourceOnCorrectSide(Character)
		&& IsItemInsideTransferVolume(Item);
}

AHronoCharacter* ATimelineTransferItem::FindTargetCharacter(
	const AHronoCharacter& InSourceCharacter) const
{
	if (!IsValid(LinkedTransfer))
	{
		return nullptr;
	}

	AHronoCharacter* ClosestTarget = nullptr;
	float ClosestDistanceSquared = TNumericLimits<float>::Max();
	for (TActorIterator<AHronoCharacter> It(GetWorld()); It; ++It)
	{
		AHronoCharacter* Candidate = *It;
		if (!IsValid(Candidate)
			|| Candidate == &InSourceCharacter
			|| Candidate->GetTimeline() != LinkedTransfer->TransferTimeline)
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(
			Candidate->GetActorLocation(), LinkedTransfer->GetActorLocation());
		if (DistanceSquared < ClosestDistanceSquared)
		{
			ClosestDistanceSquared = DistanceSquared;
			ClosestTarget = Candidate;
		}
	}
	return ClosestTarget;
}

void ATimelineTransferItem::BeginMirrorPreview(
	ABase_Item* Item,
	AHronoCharacter* InSourceCharacter)
{
	if (!IsValid(Item) || !IsValid(InSourceCharacter))
	{
		return;
	}

	AHronoCharacter* TargetCharacter = FindTargetCharacter(*InSourceCharacter);
	if (!IsValid(TargetCharacter))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s Invalid target player for item %s at %s"),
			MirrorTransfer::LogPrefix, *GetNameSafe(Item), *GetNameSafe(this));
		return;
	}

	const TSubclassOf<AMirrorTransferPreview> ClassToSpawn = PreviewClass
		? PreviewClass
		: TSubclassOf<AMirrorTransferPreview>(AMirrorTransferPreview::StaticClass());
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = TargetCharacter;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AMirrorTransferPreview* Preview = GetWorld()->SpawnActor<AMirrorTransferPreview>(
		ClassToSpawn, MapItemTransformToLinked(Item->GetActorTransform()), SpawnParameters);
	if (!IsValid(Preview))
	{
		UE_LOG(LogTemp, Error, TEXT("%s Failed to create preview for %s"),
			MirrorTransfer::LogPrefix, *GetNameSafe(Item));
		return;
	}

	ActiveItem = Item;
	SourceCharacter = InSourceCharacter;
	ActivePreview = Preview;
	Item->SetMirrorTransferState(EMirrorItemTransferState::Preview);
	SetLocalTransferVFXActive(true);
	Preview->InitializePreview(this, Item, TargetCharacter, LinkedTransfer->TransferTimeline);
	Preview->SetActorTransform(MapItemTransformToLinked(Item->GetActorTransform()), false, nullptr,
		ETeleportType::TeleportPhysics);

	UE_LOG(LogTemp, Log, TEXT("%s Item entered transfer: Item=%s Source=%s Surface=%s"),
		MirrorTransfer::LogPrefix, *GetNameSafe(Item), *GetNameSafe(InSourceCharacter), *GetNameSafe(this));
	UE_LOG(LogTemp, Log, TEXT("%s Preview created: Preview=%s Target=%s Linked=%s"),
		MirrorTransfer::LogPrefix, *GetNameSafe(Preview), *GetNameSafe(TargetCharacter),
		*GetNameSafe(LinkedTransfer));

	ForceNetUpdate();
	SetMonitorInterval(ActiveUpdateInterval);
}

void ATimelineTransferItem::UpdateActivePreview()
{
	if (!IsValid(ActiveItem)
		|| !IsValid(SourceCharacter)
		|| !IsValid(ActivePreview)
		|| !IsValid(ActivePreview->TargetCharacter)
		|| !IsValidTransferPair()
		|| ActiveItem->MirrorTransferState != EMirrorItemTransferState::Preview
		|| ActiveItem->OwningCharacter != SourceCharacter
		|| SourceCharacter->GetHeldItem() != ActiveItem
		|| SourceCharacter->GetTimeline() != TransferTimeline
		|| ActivePreview->TargetCharacter->GetTimeline() != LinkedTransfer->TransferTimeline
		|| !IsSourceOnCorrectSide(*SourceCharacter)
		|| !IsItemInsideTransferVolume(*ActiveItem))
	{
		CancelMirrorTransfer(TEXT("source item left or became invalid"));
		return;
	}

	ActivePreview->SetActorTransform(
		MapItemTransformToLinked(ActiveItem->GetActorTransform()), false, nullptr,
		ETeleportType::TeleportPhysics);
	ActivePreview->ForceNetUpdate();
}

FTransform ATimelineTransferItem::MapItemTransformToLinked(
	const FTransform& ItemWorldTransform) const
{
	if (!IsValid(TransferPoint) || !IsValid(LinkedTransfer)
		|| !IsValid(LinkedTransfer->TransferPoint))
	{
		return ItemWorldTransform;
	}

	const FTransform SourceTransform = TransferPoint->GetComponentTransform();
	const FTransform TargetTransform = LinkedTransfer->TransferPoint->GetComponentTransform();
	FVector LocalLocation = SourceTransform.InverseTransformPosition(ItemWorldTransform.GetLocation());
	LocalLocation.X *= -1.0f;

	FVector LocalForward = SourceTransform.InverseTransformVectorNoScale(
		ItemWorldTransform.GetRotation().GetForwardVector());
	FVector LocalUp = SourceTransform.InverseTransformVectorNoScale(
		ItemWorldTransform.GetRotation().GetUpVector());
	LocalForward.X *= -1.0f;
	LocalUp.X *= -1.0f;

	const FVector TargetForward = TargetTransform.TransformVectorNoScale(LocalForward).GetSafeNormal();
	const FVector TargetUp = TargetTransform.TransformVectorNoScale(LocalUp).GetSafeNormal();
	const FQuat TargetRotation = FRotationMatrix::MakeFromXZ(TargetForward, TargetUp).ToQuat();

	return FTransform(TargetRotation, TargetTransform.TransformPosition(LocalLocation),
		ItemWorldTransform.GetScale3D());
}

bool ATimelineTransferItem::CompleteMirrorTransfer(
	AHronoCharacter* TargetCharacter,
	AMirrorTransferPreview* RequestingPreview)
{
	if (!HasAuthority()
		|| !IsValid(TargetCharacter)
		|| !IsValid(ActiveItem)
		|| !IsValid(SourceCharacter)
		|| !IsValid(RequestingPreview)
		|| RequestingPreview != ActivePreview
		|| ActiveItem->MirrorTransferState != EMirrorItemTransferState::Preview
		|| !IsValidTransferPair()
		|| ActiveItem->OwningCharacter != SourceCharacter
		|| SourceCharacter->GetHeldItem() != ActiveItem
		|| !ActiveItem->bCanTransferThroughMirror
		|| SourceCharacter->GetTimeline() != TransferTimeline
		|| !IsSourceOnCorrectSide(*SourceCharacter)
		|| !IsItemInsideTransferVolume(*ActiveItem)
		|| TargetCharacter->GetTimeline() != LinkedTransfer->TransferTimeline
		|| RequestingPreview->GetOwner() != TargetCharacter
		|| IsValid(TargetCharacter->GetHeldItem())
		|| FVector::DistSquared(TargetCharacter->GetActorLocation(), RequestingPreview->GetActorLocation())
			> FMath::Square(MaxTakeDistance))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s Invalid target player or stale pending request: Target=%s Item=%s"),
			MirrorTransfer::LogPrefix, *GetNameSafe(TargetCharacter), *GetNameSafe(ActiveItem));
		return false;
	}

	ABase_Item* ItemToTransfer = ActiveItem;
	AMirrorTransferPreview* PreviewToDestroy = ActivePreview;
	ItemToTransfer->SetMirrorTransferState(EMirrorItemTransferState::Pending);
	UE_LOG(LogTemp, Log, TEXT("%s Transfer pending: Item=%s Source=%s Target=%s"),
		MirrorTransfer::LogPrefix, *GetNameSafe(ItemToTransfer), *GetNameSafe(SourceCharacter),
		*GetNameSafe(TargetCharacter));

	ItemToTransfer->SetItemTimeline(TargetCharacter->GetTimeline());
	if (!SourceCharacter->TransferHeldItemTo(TargetCharacter, ItemToTransfer))
	{
		ItemToTransfer->SetItemTimeline(SourceCharacter->GetTimeline());
		ItemToTransfer->SetMirrorTransferState(EMirrorItemTransferState::Preview);
		UE_LOG(LogTemp, Warning, TEXT("%s Transfer cancelled: character handoff failed for %s"),
			MirrorTransfer::LogPrefix, *GetNameSafe(ItemToTransfer));
		return false;
	}

	ItemToTransfer->SetMirrorTransferState(EMirrorItemTransferState::Completed);
	LinkedTransfer->CompletedItemAwaitingExit = ItemToTransfer;
	if (IsValid(PreviewToDestroy))
	{
		PreviewToDestroy->Destroy();
	}

	ActiveItem = nullptr;
	ActivePreview = nullptr;
	SourceCharacter = nullptr;
	SetLocalTransferVFXActive(false);
	ForceNetUpdate();
	SetMonitorInterval(IdleScanInterval);

	UE_LOG(LogTemp, Log, TEXT("%s Transfer completed: Item=%s Target=%s"),
		MirrorTransfer::LogPrefix, *GetNameSafe(ItemToTransfer), *GetNameSafe(TargetCharacter));

	return true;
}

void ATimelineTransferItem::CancelMirrorTransfer(const TCHAR* Reason)
{
	ABase_Item* ItemToReset = ActiveItem;
	AMirrorTransferPreview* PreviewToDestroy = ActivePreview;
	ActiveItem = nullptr;
	ActivePreview = nullptr;
	SourceCharacter = nullptr;
	SetLocalTransferVFXActive(false);

	if (IsValid(ItemToReset)
		&& ItemToReset->MirrorTransferState != EMirrorItemTransferState::Completed)
	{
		ItemToReset->SetMirrorTransferState(EMirrorItemTransferState::None);
	}
	if (IsValid(PreviewToDestroy))
	{
		PreviewToDestroy->Destroy();
	}

	if (IsValid(ItemToReset) || IsValid(PreviewToDestroy))
	{
		UE_LOG(LogTemp, Log, TEXT("%s Transfer cancelled: Item=%s Reason=%s"),
			MirrorTransfer::LogPrefix, *GetNameSafe(ItemToReset), Reason);
		ForceNetUpdate();
	}

	if (HasAuthority() && GetWorld())
	{
		SetMonitorInterval(IdleScanInterval);
	}
}
