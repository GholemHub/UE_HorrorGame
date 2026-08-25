#include "Items/MirrorTransferPreview.h"

#include "HronoCharacter.h"
#include "HronoCollisionChannels.h"
#include "Items/Base_Item.h"
#include "Items/TimelineTransferItem.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"

AMirrorTransferPreview::AMirrorTransferPreview()
{
	bReplicates = true;
	bAlwaysRelevant = false;
	bOnlyRelevantToOwner = true;
	SetReplicateMovement(true);
	SetNetUpdateFrequency(20.0f);
	SetMinNetUpdateFrequency(10.0f);
	PrimaryActorTick.bCanEverTick = false;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	PreviewMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMesh"));
	PreviewMesh->SetupAttachment(DefaultSceneRoot);
	PreviewMesh->SetMobility(EComponentMobility::Movable);
	PreviewMesh->SetSimulatePhysics(false);
	PreviewMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PreviewMesh->SetCollisionObjectType(COLLISION_CHANNEL_ITEM);
	PreviewMesh->SetCollisionResponseToAllChannels(ECR_Ignore);

	InteractionText = NSLOCTEXT("MirrorTransfer", "TakePrompt", "Take");
}

void AMirrorTransferPreview::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMirrorTransferPreview, SourceTransfer);
	DOREPLIFETIME(AMirrorTransferPreview, SourceItem);
	DOREPLIFETIME(AMirrorTransferPreview, TargetCharacter);
	DOREPLIFETIME(AMirrorTransferPreview, TargetTimeline);
}

void AMirrorTransferPreview::BeginPlay()
{
	Super::BeginPlay();
	RefreshPreviewVisuals();
	RefreshLocalVisibility();
}

void AMirrorTransferPreview::InitializePreview(
	ATimelineTransferItem* InSourceTransfer,
	ABase_Item* InSourceItem,
	AHronoCharacter* InTargetCharacter,
	EItemTimeline InTargetTimeline)
{
	if (!HasAuthority())
	{
		return;
	}

	SourceTransfer = InSourceTransfer;
	SourceItem = InSourceItem;
	TargetCharacter = InTargetCharacter;
	TargetTimeline = InTargetTimeline;
	SetOwner(InTargetCharacter);
	RefreshPreviewVisuals();
	RefreshLocalVisibility();
	ForceNetUpdate();
}

void AMirrorTransferPreview::OnRep_PreviewData()
{
	RefreshPreviewVisuals();
	RefreshLocalVisibility();
}

void AMirrorTransferPreview::RefreshPreviewVisuals()
{
	if (!IsValid(PreviewMesh) || !IsValid(SourceItem) || !IsValid(SourceItem->GetItemMesh()))
	{
		return;
	}

	UStaticMeshComponent* SourceMesh = SourceItem->GetItemMesh();
	PreviewMesh->SetStaticMesh(SourceMesh->GetStaticMesh());
	PreviewMesh->SetRelativeTransform(SourceMesh->GetRelativeTransform());
	for (int32 MaterialIndex = 0; MaterialIndex < SourceMesh->GetNumMaterials(); ++MaterialIndex)
	{
		PreviewMesh->SetMaterial(MaterialIndex, SourceMesh->GetMaterial(MaterialIndex));
	}

	PreviewMesh->SetSimulatePhysics(false);
	PreviewMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PreviewMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	if (TargetTimeline == EItemTimeline::Past)
	{
		PreviewMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, ECR_Block);
	}
	else if (TargetTimeline == EItemTimeline::Future)
	{
		PreviewMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, ECR_Block);
	}
}

void AMirrorTransferPreview::RefreshLocalVisibility()
{
	APlayerController* LocalController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	AHronoCharacter* LocalCharacter = LocalController
		? Cast<AHronoCharacter>(LocalController->GetPawn())
		: nullptr;
	const bool bVisibleToLocalPlayer = IsValid(LocalCharacter)
		&& LocalCharacter == TargetCharacter
		&& LocalCharacter->GetTimeline() == TargetTimeline;
	PreviewMesh->SetVisibility(bVisibleToLocalPlayer, true);
}

void AMirrorTransferPreview::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority())
	{
		return;
	}

	AHronoCharacter* InteractingCharacter = Cast<AHronoCharacter>(Interactor);
	if (!IsValid(SourceTransfer)
		|| !IsValid(InteractingCharacter)
		|| InteractingCharacter != TargetCharacter)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[MirrorTransfer] Invalid target player interacted with preview: Interactor=%s Target=%s"),
			*GetNameSafe(Interactor), *GetNameSafe(TargetCharacter));
		return;
	}

	SourceTransfer->CompleteMirrorTransfer(InteractingCharacter, this);
}
