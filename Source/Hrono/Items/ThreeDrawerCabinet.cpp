#include "Items/ThreeDrawerCabinet.h"

#include "Components/Drag_Component.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "HronoCollisionChannels.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

AThreeDrawerCabinet::AThreeDrawerCabinet()
{
	bReplicates = true;
	SetReplicateMovement(true);
	ItemType = EItemType::Draggable;
	ItemTimeline = EItemTimeline::Both;
	ItemName = NSLOCTEXT("HronoItems", "ThreeDrawerCabinet", "Three Drawer Cabinet");

	// ABase_Item and ADrag_Item both create root-like scene components. Keep
	// SceneRoot as the only actual actor root and place the unused inherited root
	// beneath it so the Blueprint cannot interpret two unattached roots.
	if (DefaultSceneRoot && DefaultSceneRoot != SceneRoot)
	{
		DefaultSceneRoot->SetupAttachment(SceneRoot);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CabinetMeshFinder(
		TEXT("/Game/FreeFurniturePack/Meshes/SM_Classic_Commode.SM_Classic_Commode"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DrawerMeshFinder(
		TEXT("/Game/FreeFurniturePack/Meshes/SM_Classic_Commode_Drawer_1.SM_Classic_Commode_Drawer_1"));
	static ConstructorHelpers::FObjectFinder<USoundBase> DrawerOpenSoundFinder(
		TEXT("/Game/HorrorEngine/Audio/Interactions/S_Drawer_Open_Cue.S_Drawer_Open_Cue"));
	static ConstructorHelpers::FObjectFinder<USoundBase> DrawerCloseSoundFinder(
		TEXT("/Game/HorrorEngine/Audio/Interactions/S_Drawer_Close_Cue.S_Drawer_Close_Cue"));

	if (CabinetMeshFinder.Succeeded())
	{
		FrameMesh->SetStaticMesh(CabinetMeshFinder.Object);
	}
	FrameMesh->bEditableWhenInherited = true;
	FrameMesh->SetMobility(EComponentMobility::Movable);

	BottomDrawerMesh = ItemMesh;
	// Frame and every drawer are siblings under the actor root. This keeps a
	// visual frame transform from changing a drawer's authored slide direction.
	BottomDrawerMesh->SetupAttachment(SceneRoot);
	BottomDrawerMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 16.0f));
	BottomDrawerMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.25f));
	BottomDrawerMesh->bEditableWhenInherited = true;

	MiddleDrawerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MiddleDrawerMesh"));
	MiddleDrawerMesh->SetupAttachment(SceneRoot);
	MiddleDrawerMesh->SetRelativeLocation(MiddleDrawerPosition);
	MiddleDrawerMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.25f));
	MiddleDrawerMesh->bEditableWhenInherited = true;

	TopDrawerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TopDrawerMesh"));
	TopDrawerMesh->SetupAttachment(SceneRoot);
	TopDrawerMesh->SetRelativeLocation(TopDrawerPosition);
	TopDrawerMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.25f));
	TopDrawerMesh->bEditableWhenInherited = true;

	if (DrawerMeshFinder.Succeeded())
	{
		BottomDrawerMesh->SetStaticMesh(DrawerMeshFinder.Object);
		MiddleDrawerMesh->SetStaticMesh(DrawerMeshFinder.Object);
		TopDrawerMesh->SetStaticMesh(DrawerMeshFinder.Object);
	}

	for (UStaticMeshComponent* DrawerMesh :
		{ BottomDrawerMesh.Get(), MiddleDrawerMesh.Get(), TopDrawerMesh.Get() })
	{
		DrawerMesh->SetMobility(EComponentMobility::Movable);
		DrawerMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		DrawerMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	}

	BottomDrawerDragComponent = DragComponent;
	BottomDrawerDragComponent->bIsShelf = true;
	BottomDrawerDragComponent->TargetMovementComponentOverride = BottomDrawerMesh;
	BottomDrawerDragComponent->InteractionPrimitiveOverride = BottomDrawerMesh;
	BottomDrawerDragComponent->ShelfSlideAxis = FVector(0.0f, 1.0f, 0.0f);
	BottomDrawerDragComponent->ShelfMaxDistance = 50.0f;

	MiddleDrawerDragComponent =
		CreateDefaultSubobject<UDrag_Component>(TEXT("MiddleDrawerDragComponent"));
	MiddleDrawerDragComponent->bIsShelf = true;
	MiddleDrawerDragComponent->TargetMovementComponentOverride = MiddleDrawerMesh;
	MiddleDrawerDragComponent->InteractionPrimitiveOverride = MiddleDrawerMesh;
	MiddleDrawerDragComponent->ShelfSlideAxis = FVector(0.0f, 1.0f, 0.0f);
	MiddleDrawerDragComponent->ShelfMaxDistance = 50.0f;

	TopDrawerDragComponent =
		CreateDefaultSubobject<UDrag_Component>(TEXT("TopDrawerDragComponent"));
	TopDrawerDragComponent->bIsShelf = true;
	TopDrawerDragComponent->TargetMovementComponentOverride = TopDrawerMesh;
	TopDrawerDragComponent->InteractionPrimitiveOverride = TopDrawerMesh;
	TopDrawerDragComponent->ShelfSlideAxis = FVector(0.0f, 1.0f, 0.0f);
	TopDrawerDragComponent->ShelfMaxDistance = 50.0f;

	// ItemSpawnManagerSystem expands every tagged PointSet on a location actor
	// into an independent spawn slot. Each point follows its own moving drawer.
	static const FName DrawerItemSpawnPointTag(TEXT("ItemSpawnPoint"));

	BottomDrawerPointSet = PointSet;
	BottomDrawerPointSet->SetRelativeLocation(FVector(0.0f, 0.0f, 6.0f));
	BottomDrawerPointSet->ComponentTags.AddUnique(DrawerItemSpawnPointTag);

	MiddleDrawerPointSet =
		CreateDefaultSubobject<USceneComponent>(TEXT("MiddleDrawerPointSet"));
	MiddleDrawerPointSet->SetupAttachment(MiddleDrawerMesh);
	MiddleDrawerPointSet->SetRelativeLocation(FVector(0.0f, 0.0f, 6.0f));
	MiddleDrawerPointSet->ComponentTags.AddUnique(DrawerItemSpawnPointTag);

	TopDrawerPointSet =
		CreateDefaultSubobject<USceneComponent>(TEXT("TopDrawerPointSet"));
	TopDrawerPointSet->SetupAttachment(TopDrawerMesh);
	TopDrawerPointSet->SetRelativeLocation(FVector(0.0f, 0.0f, 6.0f));
	TopDrawerPointSet->ComponentTags.AddUnique(DrawerItemSpawnPointTag);

	if (DrawerOpenSoundFinder.Succeeded())
	{
		ShelfOpenSound = DrawerOpenSoundFinder.Object;
	}
	if (DrawerCloseSoundFinder.Succeeded())
	{
		ShelfCloseSound = DrawerCloseSoundFinder.Object;
	}
}

void AThreeDrawerCabinet::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AThreeDrawerCabinet, MiddleDrawerPosition);
	DOREPLIFETIME(AThreeDrawerCabinet, TopDrawerPosition);
	DOREPLIFETIME(AThreeDrawerCabinet, DrawerOpenMask);
}

void AThreeDrawerCabinet::BeginPlay()
{
	Super::BeginPlay();

	ConfigureDrawerCollision(FrameMesh);
	ConfigureDrawerCollision(BottomDrawerMesh);
	ConfigureDrawerCollision(MiddleDrawerMesh);
	ConfigureDrawerCollision(TopDrawerMesh);
	DrawerCollisionTimeline = ItemTimeline;

	if (HasAuthority())
	{
		ShelfPosition = BottomDrawerMesh->GetRelativeLocation();
		MiddleDrawerPosition = MiddleDrawerMesh->GetRelativeLocation();
		TopDrawerPosition = TopDrawerMesh->GetRelativeLocation();
		DrawerOpenMask = CalculateDrawerOpenMask();
		bIsShelfOpen = (DrawerOpenMask & 1u) != 0;
	}
}

void AThreeDrawerCabinet::UpdateMeshForLocalPlayer()
{
	Super::UpdateMeshForLocalPlayer();

	if (DrawerCollisionTimeline != ItemTimeline)
	{
		ConfigureDrawerCollision(FrameMesh);
		ConfigureDrawerCollision(BottomDrawerMesh);
		ConfigureDrawerCollision(MiddleDrawerMesh);
		ConfigureDrawerCollision(TopDrawerMesh);
		DrawerCollisionTimeline = ItemTimeline;
	}
}

USceneComponent* AThreeDrawerCabinet::GetPrimaryDoorMovementComponent() const
{
	// This actor contains shelves only. Prevent inherited door-rotation
	// replication from ever overwriting the bottom drawer's authored rotation.
	return nullptr;
}

USceneComponent* AThreeDrawerCabinet::FindShelfMovementComponent(
	FName ShelfComponentName) const
{
	if (BottomDrawerMesh
		&& (ShelfComponentName.IsNone()
			|| ShelfComponentName == BottomDrawerMesh->GetFName()))
	{
		return BottomDrawerMesh;
	}

	if (MiddleDrawerMesh && ShelfComponentName == MiddleDrawerMesh->GetFName())
	{
		return MiddleDrawerMesh;
	}

	if (TopDrawerMesh && ShelfComponentName == TopDrawerMesh->GetFName())
	{
		return TopDrawerMesh;
	}

	return Super::FindShelfMovementComponent(ShelfComponentName);
}

void AThreeDrawerCabinet::ApplyShelfPositionFromServer(
	FName ShelfComponentName,
	const FVector& NewPosition)
{
	if (!HasAuthority() || NewPosition.ContainsNaN())
	{
		return;
	}

	USceneComponent* DrawerComponent = FindShelfMovementComponent(ShelfComponentName);
	if (!DrawerComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Rejected unknown drawer component '%s'"),
			*GetName(), *ShelfComponentName.ToString());
		return;
	}

	const FVector ClampedPosition = ClampShelfPositionForComponent(
		DrawerComponent,
		NewPosition);
	DrawerComponent->SetRelativeLocation(ClampedPosition);

	if (DrawerComponent == BottomDrawerMesh)
	{
		ShelfPosition = ClampedPosition;
	}
	else if (DrawerComponent == MiddleDrawerMesh)
	{
		MiddleDrawerPosition = ClampedPosition;
	}
	else if (DrawerComponent == TopDrawerMesh)
	{
		TopDrawerPosition = ClampedPosition;
	}

	RefreshDrawerOpenState();
	ForceNetUpdate();
}

void AThreeDrawerCabinet::OnRep_MiddleDrawerPosition()
{
	if (MiddleDrawerMesh
		&& (!MiddleDrawerDragComponent || !MiddleDrawerDragComponent->bIsRotating))
	{
		MiddleDrawerMesh->SetRelativeLocation(MiddleDrawerPosition);
	}
}

void AThreeDrawerCabinet::OnRep_TopDrawerPosition()
{
	if (TopDrawerMesh
		&& (!TopDrawerDragComponent || !TopDrawerDragComponent->bIsRotating))
	{
		TopDrawerMesh->SetRelativeLocation(TopDrawerPosition);
	}
}

void AThreeDrawerCabinet::OnRep_DrawerOpenMask(uint8 PreviousMask)
{
	bIsShelfOpen = (DrawerOpenMask & 1u) != 0;
	BroadcastDrawerStateChanges(PreviousMask);
}

uint8 AThreeDrawerCabinet::CalculateDrawerOpenMask() const
{
	uint8 NewMask = 0;
	const UStaticMeshComponent* DrawerMeshes[] =
	{
		BottomDrawerMesh,
		MiddleDrawerMesh,
		TopDrawerMesh
	};

	for (int32 DrawerIndex = 0; DrawerIndex < UE_ARRAY_COUNT(DrawerMeshes); ++DrawerIndex)
	{
		const UStaticMeshComponent* DrawerMesh = DrawerMeshes[DrawerIndex];
		const UDrag_Component* DrawerDrag =
			FindDragComponentForMovementComponent(DrawerMesh);
		if (!DrawerMesh || !DrawerDrag)
		{
			continue;
		}

		const FVector SlideAxis = DrawerDrag->ShelfSlideAxis.GetSafeNormal();
		const float CurrentOffset = FVector::DotProduct(
			DrawerMesh->GetRelativeLocation() - DrawerDrag->ShelfClosedLocation,
			SlideAxis);
		if (CurrentOffset > DrawerDrag->ShelfMaxDistance * 0.5f)
		{
			NewMask |= static_cast<uint8>(1u << DrawerIndex);
		}
	}

	return NewMask;
}

void AThreeDrawerCabinet::RefreshDrawerOpenState()
{
	if (!HasAuthority())
	{
		return;
	}

	const uint8 PreviousMask = DrawerOpenMask;
	DrawerOpenMask = CalculateDrawerOpenMask();
	bIsShelfOpen = (DrawerOpenMask & 1u) != 0;
	if (PreviousMask != DrawerOpenMask)
	{
		BroadcastDrawerStateChanges(PreviousMask);
	}
}

void AThreeDrawerCabinet::BroadcastDrawerStateChanges(uint8 PreviousMask)
{
	for (int32 DrawerIndex = 0; DrawerIndex < 3; ++DrawerIndex)
	{
		const uint8 DrawerBit = static_cast<uint8>(1u << DrawerIndex);
		if ((PreviousMask & DrawerBit) == (DrawerOpenMask & DrawerBit))
		{
			continue;
		}

		const bool bDrawerOpen = (DrawerOpenMask & DrawerBit) != 0;
		OnDrawerStateChanged.Broadcast(DrawerIndex, bDrawerOpen);
		if (DrawerIndex == 0)
		{
			if (bDrawerOpen)
			{
				OnShelfOpen.Broadcast();
			}
			else
			{
				OnShelfClose.Broadcast();
			}
		}

		if (UStaticMeshComponent* DrawerMesh = GetDrawerMesh(DrawerIndex))
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				bDrawerOpen ? ShelfOpenSound : ShelfCloseSound,
				DrawerMesh->GetComponentLocation());
		}
	}
}

bool AThreeDrawerCabinet::IsDrawerOpen(int32 DrawerIndex) const
{
	return DrawerIndex >= 0
		&& DrawerIndex < 3
		&& (DrawerOpenMask & static_cast<uint8>(1u << DrawerIndex)) != 0;
}

UStaticMeshComponent* AThreeDrawerCabinet::GetDrawerMesh(int32 DrawerIndex) const
{
	switch (DrawerIndex)
	{
	case 0:
		return BottomDrawerMesh;
	case 1:
		return MiddleDrawerMesh;
	case 2:
		return TopDrawerMesh;
	default:
		return nullptr;
	}
}

USceneComponent* AThreeDrawerCabinet::GetDrawerPointSet(int32 DrawerIndex) const
{
	switch (DrawerIndex)
	{
	case 0:
		return BottomDrawerPointSet;
	case 1:
		return MiddleDrawerPointSet;
	case 2:
		return TopDrawerPointSet;
	default:
		return nullptr;
	}
}

void AThreeDrawerCabinet::ConfigureDrawerCollision(UPrimitiveComponent* Component) const
{
	if (!Component)
	{
		return;
	}

	Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Component->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	const bool bPastCanUse =
		ItemTimeline == EItemTimeline::Both || ItemTimeline == EItemTimeline::Past;
	const bool bFutureCanUse =
		ItemTimeline == EItemTimeline::Both || ItemTimeline == EItemTimeline::Future;

	Component->SetCollisionObjectType(
		ItemTimeline == EItemTimeline::Future
			? COLLISION_CHANNEL_DOOR_FUTURE
			: COLLISION_CHANNEL_DOOR_PAST);
	Component->SetCollisionResponseToChannel(
		COLLISION_CHANNEL_PAWN_PAST,
		bPastCanUse ? ECR_Block : ECR_Ignore);
	Component->SetCollisionResponseToChannel(
		COLLISION_CHANNEL_PAWN_FUTURE,
		bFutureCanUse ? ECR_Block : ECR_Ignore);
}
