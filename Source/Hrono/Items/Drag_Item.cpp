// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Drag_Item.h"
#include "Components/Drag_Component.h"
#include "HronoCharacter.h"
#include "HronoCollisionChannels.h"
#include "Net/UnrealNetwork.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"



// Sets default values
ADrag_Item::ADrag_Item()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true; // Door open/closed state must replicate so server collision matches clients

    SceneRoot =
        CreateDefaultSubobject<USceneComponent>(
            TEXT("SceneRoot"));

    RootComponent = SceneRoot;

    FrameMesh =
        CreateDefaultSubobject<UStaticMeshComponent>(
            TEXT("FrameMesh"));

    FrameMesh->SetupAttachment(SceneRoot);

	ItemMesh->SetupAttachment(FrameMesh);

	DragComponent = CreateDefaultSubobject<UDrag_Component>(TEXT("DragComponent"));

	// Editable set of points authored in the viewport / Blueprint. Attached to the
	// root so its control points move with the actor.
	PointSet = CreateDefaultSubobject<USceneComponent>(TEXT("PointSet"));
	PointSet->SetupAttachment(ItemMesh);

	// Looping audio source that follows the moving panel. The actual sound is set
	// at runtime (door vs shelf) and the component is only activated while dragging.
	MoveAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MoveAudioComponent"));
	MoveAudioComponent->SetupAttachment(ItemMesh);
	MoveAudioComponent->bAutoActivate = false;
}

void ADrag_Item::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ADrag_Item, DoorRotation);
    DOREPLIFETIME(ADrag_Item, bIsClosed);
    DOREPLIFETIME(ADrag_Item, ShelfPosition);
    DOREPLIFETIME(ADrag_Item, bIsShelfOpen);
	DOREPLIFETIME(ADrag_Item, bNeedKeyActor);
	DOREPLIFETIME(ADrag_Item, RequiredKeyTag);
	DOREPLIFETIME(ADrag_Item, PastBarricadeCount);
	DOREPLIFETIME(ADrag_Item, FutureBarricadeCount);
	DOREPLIFETIME(ADrag_Item, TriggerLockCount);

}

bool ADrag_Item::IsDoorBlockedForTimeline(EItemTimeline Timeline) const
{
	switch (Timeline)
	{
	case EItemTimeline::Past:
		return PastBarricadeCount > 0;
	case EItemTimeline::Future:
		return FutureBarricadeCount > 0;
	case EItemTimeline::Both:
	default:
		return PastBarricadeCount > 0 || FutureBarricadeCount > 0;
	}
}

void ADrag_Item::RegisterDoorBarricade(EItemTimeline Timeline, bool bRegister)
{
	if (!HasAuthority())
	{
		return;
	}

	const int32 Delta = bRegister ? 1 : -1;
	if (Timeline == EItemTimeline::Past || Timeline == EItemTimeline::Both)
	{
		PastBarricadeCount = FMath::Max(0, PastBarricadeCount + Delta);
	}
	if (Timeline == EItemTimeline::Future || Timeline == EItemTimeline::Both)
	{
		FutureBarricadeCount = FMath::Max(0, FutureBarricadeCount + Delta);
	}

	OnRep_BarricadeCounts();
	ForceNetUpdate();
}

void ADrag_Item::OnRep_BarricadeCounts()
{
	APlayerController* LocalController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	const AHronoCharacter* LocalCharacter = LocalController
		? Cast<AHronoCharacter>(LocalController->GetPawn())
		: nullptr;
	if (!LocalCharacter || !IsDoorBlockedForTimeline(LocalCharacter->GetTimeline()))
	{
		return;
	}

	TInlineComponentArray<UDrag_Component*> DragComponents(this);
	for (UDrag_Component* Component : DragComponents)
	{
		if (IsValid(Component) && Component->bIsRotating)
		{
			Component->StopDrag();
		}
	}

	// Snap away any client-predicted movement that began before the replicated
	// barricade count arrived.
	OnRep_DoorRotation();
}

FGameplayTag ADrag_Item::GetRequiredKeyTag() const
{
	return RequiredKeyTag.IsValid()
		? RequiredKeyTag
		: FGameplayTag::RequestGameplayTag(TEXT("Item.Key"));
}

bool ADrag_Item::CanUnlockWithItem(const ABase_Item* Item) const
{
	return IsValid(Item) && Item->ItemTags.HasTag(GetRequiredKeyTag());
}

void ADrag_Item::RegisterDoorTriggerLock(bool bRegister)
{
	if (!HasAuthority())
	{
		return;
	}

	const int32 PreviousCount = TriggerLockCount;
	TriggerLockCount = FMath::Max(0, TriggerLockCount + (bRegister ? 1 : -1));
	if (TriggerLockCount == PreviousCount)
	{
		return;
	}

	OnRep_TriggerLockCount();
	ForceNetUpdate();
}

void ADrag_Item::OnRep_TriggerLockCount()
{
	if (!IsLockedByTrigger())
	{
		return;
	}

	// A player may have started dragging just before the replicated lock arrived.
	// Stop every panel on multi-door actors immediately.
	TInlineComponentArray<UDrag_Component*> DragComponents(this);
	for (UDrag_Component* Component : DragComponents)
	{
		if (IsValid(Component) && Component->bIsRotating)
		{
			Component->StopDrag();
		}
	}

	OnRep_DoorRotation();
}

void ADrag_Item::OnRep_DoorRotation()
{
    // Runs on remote clients only. Skip while this client is actively dragging so we
    // don't fight local prediction or the synchronized automatic animation.
    if ((DragComponent && DragComponent->bIsRotating) || bDoorAnimationActive)
    {
        return;
    }

	if (USceneComponent* DoorMovementComponent = GetPrimaryDoorMovementComponent())
	{
		// Apply the exact authoritative rotation so every machine matches the server.
		DoorMovementComponent->SetRelativeRotation(DoorRotation);
	}
}

void ADrag_Item::AnimateDoor(bool bOpen)
{
    // A placed door is not owned by a client, so only the authority may fan this
    // action out to every machine. Blueprint triggers should call this on Authority.
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Warning,
            TEXT("AnimateDoor ignored for %s because it was not called on the server"),
            *GetName());
        return;
    }

	if (!bAllowAnimateDoorOpenClose)
	{
		UE_LOG(LogTemp, Log,
			TEXT("AnimateDoor ignored for %s because Allow Animate Door Open Close is disabled"),
			*GetName());
		return;
	}

	// Automatic opening has no initiating player timeline. Keep a barricaded door
	// closed if any timeline is protected; closing requests are always allowed.
	if (bOpen && IsDoorBlockedForTimeline(EItemTimeline::Both))
	{
		UE_LOG(LogTemp, Log,
			TEXT("AnimateDoor ignored for %s because an intact barricade blocks it"),
			*GetName());
		return;
	}

	USceneComponent* DoorMovementComponent = GetPrimaryDoorMovementComponent();
	if (!DoorMovementComponent)
	{
		return;
	}

	const FRotator StartRotation = DoorMovementComponent->GetRelativeRotation();
    FRotator TargetRotation = StartRotation;

    if (bOpen)
    {
        const float Direction = ItemType == EItemType::DraggableInvertLeft ? 1.0f : -1.0f;
        TargetRotation.Yaw = Direction * FMath::Abs(AnimatedDoorOpenAngle);
    }
    else
    {
        TargetRotation.Yaw = 0.0f;
    }

    if (StartRotation.Equals(TargetRotation, 0.01f))
    {
        DoorRotation = TargetRotation;
        RefreshDoorClosedState();
        ForceNetUpdate();
        return;
    }

    MulticastStartDoorAnimation(
        TargetRotation,
        FMath::Max(DoorAnimationDuration, KINDA_SMALL_NUMBER));
}

void ADrag_Item::MulticastStartDoorAnimation_Implementation(
    FRotator TargetRotation,
    float Duration)
{
	USceneComponent* DoorMovementComponent = GetPrimaryDoorMovementComponent();
	if (!DoorMovementComponent)
	{
		return;
    }

    if (DragComponent && DragComponent->bIsRotating)
    {
        DragComponent->StopDrag();
    }

    // Begin from the pose currently visible on this machine. A replicated rotator
    // may encode -90 degrees as 270 degrees; normalizing both ends prevents that
    // equivalent representation from becoming a visible full revolution.
	DoorAnimationStartRotation = DoorMovementComponent->GetRelativeRotation().GetNormalized();
    DoorAnimationTargetRotation = TargetRotation.GetNormalized();
    DoorAnimationElapsed = 0.0f;
    ActiveDoorAnimationDuration = FMath::Max(Duration, KINDA_SMALL_NUMBER);
    bDoorAnimationActive = true;

	DoorMovementComponent->SetRelativeRotation(DoorAnimationStartRotation);
    DoorRotation = DoorAnimationStartRotation;
    StartMoveSound(false);
}

void ADrag_Item::UpdateDoorAnimation(float DeltaTime)
{
	USceneComponent* DoorMovementComponent = GetPrimaryDoorMovementComponent();
	if (!bDoorAnimationActive || !DoorMovementComponent)
    {
        return;
    }

    DoorAnimationElapsed += DeltaTime;
    const float Alpha = FMath::Clamp(
        DoorAnimationElapsed / ActiveDoorAnimationDuration,
        0.0f,
        1.0f);
    const float EasedAlpha = FMath::InterpEaseInOut(
        0.0f,
        1.0f,
        Alpha,
        FMath::Max(1.0f, DoorAnimationEaseExponent));

    // Never lerp raw Euler values: -90 and 270 describe the same pose, but a raw
    // lerp between 270 and 0 rotates the door 270 degrees through the wall. Find
    // the signed shortest delta for every axis instead.
    const auto LerpAngleShortestPath = [EasedAlpha](float Start, float Target)
    {
        return Start + FMath::FindDeltaAngleDegrees(Start, Target) * EasedAlpha;
    };

    const FRotator NewRotation(
        LerpAngleShortestPath(DoorAnimationStartRotation.Pitch, DoorAnimationTargetRotation.Pitch),
        LerpAngleShortestPath(DoorAnimationStartRotation.Yaw, DoorAnimationTargetRotation.Yaw),
        LerpAngleShortestPath(DoorAnimationStartRotation.Roll, DoorAnimationTargetRotation.Roll));

	DoorMovementComponent->SetRelativeRotation(NewRotation);
    DoorRotation = NewRotation;

    if (Alpha < 1.0f)
    {
        return;
    }

    bDoorAnimationActive = false;
    DoorRotation = DoorAnimationTargetRotation;
	DoorMovementComponent->SetRelativeRotation(DoorRotation);
    StopMoveSound();

    if (HasAuthority())
    {
        RefreshDoorClosedState();
        ForceNetUpdate();
    }
}

USceneComponent* ADrag_Item::GetPrimaryDoorMovementComponent() const
{
	return ItemMesh;
}

UDrag_Component* ADrag_Item::FindDragComponentForHit(const UPrimitiveComponent* HitComponent) const
{
	TInlineComponentArray<UDrag_Component*> DragComponents(this);
	for (UDrag_Component* Candidate : DragComponents)
	{
		if (IsValid(Candidate) && Candidate->MatchesHitComponent(HitComponent))
		{
			return Candidate;
		}
	}

	// Preserve the old single-door behavior if a Blueprint has unusual nested collision.
	return DragComponent;
}

USceneComponent* ADrag_Item::FindDoorMovementComponent(FName DoorComponentName) const
{
	USceneComponent* PrimaryComponent = GetPrimaryDoorMovementComponent();
	if (PrimaryComponent
		&& (DoorComponentName.IsNone() || PrimaryComponent->GetFName() == DoorComponentName))
	{
		return PrimaryComponent;
	}

	return nullptr;
}

void ADrag_Item::ApplyDoorRotationFromServer(FName DoorComponentName, const FRotator& NewRotation)
{
	if (!HasAuthority())
	{
		return;
	}

	USceneComponent* DoorMovementComponent = FindDoorMovementComponent(DoorComponentName);
	if (!DoorMovementComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Rejected unknown door component '%s'"),
			*GetName(), *DoorComponentName.ToString());
		return;
	}

	DoorMovementComponent->SetRelativeRotation(NewRotation);
	DoorRotation = NewRotation;
	RefreshDoorClosedState();
	ForceNetUpdate();
}

USceneComponent* ADrag_Item::FindShelfMovementComponent(FName ShelfComponentName) const
{
	if (ItemMesh
		&& (ShelfComponentName.IsNone() || ItemMesh->GetFName() == ShelfComponentName))
	{
		return ItemMesh;
	}

	return nullptr;
}

UDrag_Component* ADrag_Item::FindDragComponentForMovementComponent(
	const USceneComponent* MovementComponent) const
{
	if (!IsValid(MovementComponent))
	{
		return nullptr;
	}

	TInlineComponentArray<UDrag_Component*> DragComponents(this);
	for (UDrag_Component* Candidate : DragComponents)
	{
		if (IsValid(Candidate)
			&& (Candidate->bIsShelf || Candidate->bIsCupBoard)
			&& Candidate->GetTargetMovementComponent() == MovementComponent)
		{
			return Candidate;
		}
	}

	return nullptr;
}

FVector ADrag_Item::ClampShelfPositionForComponent(
	const USceneComponent* MovementComponent,
	const FVector& RequestedPosition) const
{
	const UDrag_Component* ShelfDragComponent =
		FindDragComponentForMovementComponent(MovementComponent);
	if (!ShelfDragComponent)
	{
		return MovementComponent
			? MovementComponent->GetRelativeLocation()
			: FVector::ZeroVector;
	}

	const bool bCupBoard = ShelfDragComponent->bIsCupBoard;
	const FVector ClosedLocation = bCupBoard
		? ShelfDragComponent->CupBoardClosedLocation
		: ShelfDragComponent->ShelfClosedLocation;
	const FVector SlideAxis = (bCupBoard
		? ShelfDragComponent->CupBoardSlideAxis
		: ShelfDragComponent->ShelfSlideAxis).GetSafeNormal();
	if (SlideAxis.IsNearlyZero())
	{
		return ClosedLocation;
	}

	const float RequestedOffset = FVector::DotProduct(
		RequestedPosition - ClosedLocation,
		SlideAxis);
	const float ClampedOffset = FMath::Clamp(
		RequestedOffset,
		0.0f,
		FMath::Max(0.0f, bCupBoard
			? ShelfDragComponent->CupBoardMaxDistance
			: ShelfDragComponent->ShelfMaxDistance));

	return ClosedLocation + SlideAxis * ClampedOffset;
}

void ADrag_Item::ApplyShelfPositionFromServer(
	FName ShelfComponentName,
	const FVector& NewPosition)
{
	if (!HasAuthority() || NewPosition.ContainsNaN())
	{
		return;
	}

	USceneComponent* ShelfMovementComponent =
		FindShelfMovementComponent(ShelfComponentName);
	if (!ShelfMovementComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Rejected unknown shelf component '%s'"),
			*GetName(), *ShelfComponentName.ToString());
		return;
	}

	const FVector ClampedPosition = ClampShelfPositionForComponent(
		ShelfMovementComponent,
		NewPosition);
	ShelfMovementComponent->SetRelativeLocation(ClampedPosition);
	ShelfPosition = ClampedPosition;
	RefreshShelfOpenState();
	ForceNetUpdate();
}

void ADrag_Item::RefreshDoorClosedState()
{
    // Authority is the single source of truth for the replicated bIsClosed flag.
    if (!HasAuthority())
    {
        return;
    }

    // The door is closed when its Yaw is (almost) zero.
    const bool bNewClosed = FMath::Abs(DoorRotation.Yaw) <= DoorClosedYawTolerance;
    if (bNewClosed == bIsClosed)
    {
        return;
    }

    bIsClosed = bNewClosed;

    // OnRep_IsClosed only fires on remote clients, so broadcast here for the
    // server/listen-server host as well.
    UE_LOG(LogTemp, Log, TEXT("[SERVER] Door %s"), bIsClosed ? TEXT("closed") : TEXT("open"));
    UGameplayStatics::PlaySoundAtLocation(this, bIsClosed ? DoorCloseSound : DoorOpenSound, GetActorLocation());
    OnDoorStateChanged.Broadcast(bIsClosed);
}

void ADrag_Item::RefreshShelfOpenState()
{
    if (!ItemMesh || !DragComponent) return;

    const FVector CurrentPosition = ItemMesh->GetRelativeLocation();
    const bool bCupBoard = DragComponent && DragComponent->bIsCupBoard;
    const float CurrentDistance = bCupBoard
        ? FVector::Distance(CurrentPosition, DragComponent->CupBoardClosedLocation)
        : FMath::Abs(CurrentPosition.Y);
    const float MaxDistance = bCupBoard
        ? DragComponent->CupBoardMaxDistance
        : DragComponent->ShelfMaxDistance;

    // Determine if shelf is open or closed
    const bool bIsNowOpen = CurrentDistance > (MaxDistance * 0.5f);  // More than 50% open

    // Only trigger state changes if it changed
    if (bIsNowOpen != bIsShelfOpen)
    {
        bIsShelfOpen = bIsNowOpen;

        // Play sound/animation based on state
        if (bIsNowOpen)
        {
            OnShelfOpened();
        }
        else
        {
            OnShelfClosed();
        }
    }

    // A cupboard panel must continue blocking pawns after it slides sideways.
    if (!bCupBoard)
    {
        UpdateShelfCollision();
    }
}

void ADrag_Item::OnRep_ShelfPosition()
{
    // Keep local prediction responsive for the player currently dragging, while
    // applying the authoritative position to every other client.
    if (!ItemMesh || (DragComponent && DragComponent->bIsRotating))
    {
        return;
    }

    ItemMesh->SetRelativeLocation(ShelfPosition);
}

void ADrag_Item::OnShelfOpened()
{
    // Broadcast event for animations, sounds, etc.
    if (OnShelfOpen.IsBound())
    {
        OnShelfOpen.Broadcast();
    }

    UGameplayStatics::PlaySoundAtLocation(this, ShelfOpenSound, GetActorLocation());
    UE_LOG(LogTemp, Log, TEXT("Shelf opened"));
}

void ADrag_Item::OnShelfClosed()
{
    // Broadcast event when shelf closes
    if (OnShelfClose.IsBound())
    {
        OnShelfClose.Broadcast();
    }

    UGameplayStatics::PlaySoundAtLocation(this, ShelfCloseSound, GetActorLocation());
    UE_LOG(LogTemp, Log, TEXT("Shelf closed"));
}

void ADrag_Item::StartMoveSound(bool bShelf)
{
    if (!MoveAudioComponent)
    {
        return;
    }

    USoundBase* MoveSound = bShelf ? ShelfMoveSound : DoorMoveSound;
    if (!MoveSound)
    {
        return;
    }

    MoveAudioComponent->SetSound(MoveSound);

    if (!MoveAudioComponent->IsPlaying())
    {
        MoveAudioComponent->Play();
    }
}

void ADrag_Item::StopMoveSound()
{
    if (MoveAudioComponent && MoveAudioComponent->IsPlaying())
    {
        // Small fade avoids an abrupt cut when the player releases the door.
        MoveAudioComponent->FadeOut(0.15f, 0.0f);
    }
}

void ADrag_Item::NotifyDragStarted(bool bShelf)
{
    // Let Blueprints react to the start of a drag interaction.
    OnDragStarted.Broadcast(bShelf);
}

void ADrag_Item::UpdateShelfCollision()
{
    // Enable/disable collision for items inside shelf based on open state
    if (ItemMesh)
    {
        // You can adjust collision channels or disable overlap based on bIsShelfOpen
        ItemMesh->SetCollisionResponseToChannel(ECC_Pawn,
            bIsShelfOpen ? ECR_Ignore : ECR_Block);
    }
}

void ADrag_Item::OnRep_IsClosed()
{
    // Runs on remote clients when the authority changes bIsClosed.
    UE_LOG(LogTemp, Log, TEXT("[CLIENT] Door %s"), bIsClosed ? TEXT("closed") : TEXT("open"));
    UGameplayStatics::PlaySoundAtLocation(this, bIsClosed ? DoorCloseSound : DoorOpenSound, GetActorLocation());
    OnDoorStateChanged.Broadcast(bIsClosed);
}

void ADrag_Item::UpdateMeshForLocalPlayer()
{
    Super::UpdateMeshForLocalPlayer();
}

// Called when the game starts or when spawned
// ADrag_Item::BeginPlay
void ADrag_Item::BeginPlay()
{
    Super::BeginPlay();

    // Preserve authored non-zero mesh offsets and make the initial linear-drag
    // position authoritative for shelves and cupboard panels.
    if (HasAuthority() && ItemMesh)
    {
        ShelfPosition = ItemMesh->GetRelativeLocation();
    }

    // Configure collision channels so the server (and client) physics correctly
    // filters which pawns can collide with this door based on timeline.
    if (ItemTimeline == EItemTimeline::Future)
    {
        FrameMesh->SetCollisionObjectType(COLLISION_CHANNEL_DOOR_FUTURE);
        FrameMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, ECR_Block);
        FrameMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, ECR_Ignore);

        ItemMesh->SetCollisionObjectType(COLLISION_CHANNEL_DOOR_FUTURE);
        ItemMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, ECR_Block);
        ItemMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, ECR_Ignore);
    }
    else if (ItemTimeline == EItemTimeline::Past)
    {
        FrameMesh->SetCollisionObjectType(COLLISION_CHANNEL_DOOR_PAST);
        FrameMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, ECR_Block);
        FrameMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, ECR_Ignore);

        ItemMesh->SetCollisionObjectType(COLLISION_CHANNEL_DOOR_PAST);
        ItemMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, ECR_Block);
        ItemMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, ECR_Ignore);

    }
    else // EItemTimeline::Both — blocks all pawns
    {
        FrameMesh->SetCollisionObjectType(COLLISION_CHANNEL_DOOR_PAST);
        FrameMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, ECR_Block);
        FrameMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, ECR_Block);

        ItemMesh->SetCollisionObjectType(COLLISION_CHANNEL_DOOR_PAST);
        ItemMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, ECR_Block);
        ItemMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, ECR_Block);
    }
}

// Called every frame
void ADrag_Item::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    UpdateMeshForLocalPlayer();
    UpdateDoorAnimation(DeltaTime);

    if (GEngine && bShowDoorDebugOnScreen)
    {
		const USceneComponent* DoorMovementComponent = GetPrimaryDoorMovementComponent();
		const FRotator Rotation = DoorMovementComponent
			? DoorMovementComponent->GetRelativeRotation()
			: FRotator::ZeroRotator;
		const FString RoleName = HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT");
		const int32 Key = HasAuthority() ? 1 : 2;

		GEngine->AddOnScreenDebugMessage(
			Key,
			0.0f,
			FColor::Yellow,
			FString::Printf(
				TEXT("[%s] Door Yaw: %.1f | Closed: %s"),
				*RoleName,
				Rotation.Yaw,
				bIsClosed ? TEXT("true") : TEXT("false")));
    }
}


