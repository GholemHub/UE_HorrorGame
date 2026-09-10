// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Drag_Item.h"
#include "Components/Drag_Component.h"
#include "HronoCharacter.h"
#include "HronoCollisionChannels.h"
#include "Net/UnrealNetwork.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"



// Sets default values
ADrag_Item::ADrag_Item()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	bReplicates = true; // Door open/closed state must replicate so server collision matches clients
	bUseInteractionHighlight = false;

    SceneRoot =
        CreateDefaultSubobject<USceneComponent>(
            TEXT("SceneRoot"));

    RootComponent = SceneRoot;
	SceneRoot->SetMobility(EComponentMobility::Movable);

    FrameMesh =
        CreateDefaultSubobject<UStaticMeshComponent>(
            TEXT("FrameMesh"));

    FrameMesh->SetupAttachment(SceneRoot);
	FrameMesh->SetMobility(EComponentMobility::Movable);

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
	DOREPLIFETIME(ADrag_Item, bUseAutomaticOpenClose);

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
	RefreshActiveTickState();

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
	RefreshActiveTickState();
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

UDrag_Component* ADrag_Item::FindDragComponentForInteractionName(
	FName InteractionComponentName) const
{
	if (InteractionComponentName.IsNone())
	{
		return nullptr;
	}

	TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents(this);
	for (UPrimitiveComponent* Primitive : PrimitiveComponents)
	{
		if (IsValid(Primitive) && Primitive->GetFName() == InteractionComponentName)
		{
			TInlineComponentArray<UDrag_Component*> DragComponents(this);
			for (UDrag_Component* Candidate : DragComponents)
			{
				if (IsValid(Candidate) && Candidate->MatchesHitComponent(Primitive))
				{
					return Candidate;
				}
			}
			return nullptr;
		}
	}

	return nullptr;
}

bool ADrag_Item::ToggleAutomaticOpenClose(FName InteractionComponentName)
{
	if (!HasAuthority() || !bUseAutomaticOpenClose)
	{
		return false;
	}

	UDrag_Component* SelectedDrag =
		FindDragComponentForInteractionName(InteractionComponentName);
	USceneComponent* MovementComponent = SelectedDrag
		? SelectedDrag->GetTargetMovementComponent()
		: nullptr;
	if (!IsValid(SelectedDrag) || !IsValid(MovementComponent))
	{
		return false;
	}

	const bool bLinear = SelectedDrag->bIsShelf || SelectedDrag->bIsCupBoard;
	bool bCurrentlyOpen = false;
	bool bOpening = true;
	FVector TargetLocation = MovementComponent->GetRelativeLocation();
	FRotator TargetRotation = MovementComponent->GetRelativeRotation();

	// If E is pressed again during movement, reverse the current request immediately.
	const FDragItemAutomaticPanelAnimation* ExistingAnimation =
		AutomaticPanelAnimations.FindByPredicate(
			[MovementComponent](const FDragItemAutomaticPanelAnimation& Animation)
			{
				return Animation.MovementComponent.Get() == MovementComponent;
			});

	if (bLinear)
	{
		const FVector ClosedLocation = SelectedDrag->bIsCupBoard
			? SelectedDrag->CupBoardClosedLocation
			: SelectedDrag->ShelfClosedLocation;
		const FVector SlideAxis = (SelectedDrag->bIsCupBoard
			? SelectedDrag->CupBoardSlideAxis
			: SelectedDrag->ShelfSlideAxis).GetSafeNormal();
		const float MaximumDistance = FMath::Max(0.0f, SelectedDrag->bIsCupBoard
			? SelectedDrag->CupBoardMaxDistance
			: SelectedDrag->ShelfMaxDistance);
		if (SlideAxis.IsNearlyZero())
		{
			return false;
		}

		const float CurrentOffset = FVector::DotProduct(
			MovementComponent->GetRelativeLocation() - ClosedLocation,
			SlideAxis);
		bCurrentlyOpen = CurrentOffset > MaximumDistance * 0.5f;
		bOpening = ExistingAnimation ? !ExistingAnimation->bOpening : !bCurrentlyOpen;
		TargetLocation = ClosedLocation + SlideAxis * (bOpening ? MaximumDistance : 0.0f);
	}
	else
	{
		float OpenYaw = 0.0f;
		if (SelectedDrag->bUseCustomDoorAngleLimits)
		{
			OpenYaw = FMath::Abs(SelectedDrag->MinimumDoorYaw)
				> FMath::Abs(SelectedDrag->MaximumDoorYaw)
				? SelectedDrag->MinimumDoorYaw
				: SelectedDrag->MaximumDoorYaw;
		}
		else
		{
			const float Direction = ItemType == EItemType::DraggableInvertLeft
				? 1.0f
				: -1.0f;
			OpenYaw = Direction * FMath::Abs(AnimatedDoorOpenAngle);
		}

		const float CurrentYaw = FMath::UnwindDegrees(
			MovementComponent->GetRelativeRotation().Yaw);
		bCurrentlyOpen = FMath::Abs(CurrentYaw) > FMath::Abs(OpenYaw) * 0.5f;
		bOpening = ExistingAnimation ? !ExistingAnimation->bOpening : !bCurrentlyOpen;
		TargetRotation.Yaw = bOpening ? OpenYaw : 0.0f;
	}

	MulticastStartAutomaticPanelAnimation(
		bLinear,
		bOpening,
		MovementComponent->GetFName(),
		TargetLocation,
		TargetRotation,
		FMath::Max(AutomaticOpenCloseDuration, KINDA_SMALL_NUMBER));
	return true;
}

bool ADrag_Item::ShouldUseAutomaticOpenClose(const AActor* Interactor) const
{
	return bUseAutomaticOpenClose;
}

USceneComponent* ADrag_Item::ResolveAutomaticMovementComponent(
	bool bLinear,
	FName MovementComponentName) const
{
	return bLinear
		? FindShelfMovementComponent(MovementComponentName)
		: FindDoorMovementComponent(MovementComponentName);
}

void ADrag_Item::MulticastStartAutomaticPanelAnimation_Implementation(
	bool bLinear,
	bool bOpening,
	FName MovementComponentName,
	FVector TargetLocation,
	FRotator TargetRotation,
	float Duration)
{
	USceneComponent* MovementComponent = ResolveAutomaticMovementComponent(
		bLinear,
		MovementComponentName);
	if (!IsValid(MovementComponent))
	{
		return;
	}
	CancelNativeAnimationForAutomaticInteraction(MovementComponent);

	TInlineComponentArray<UDrag_Component*> DragComponents(this);
	for (UDrag_Component* Component : DragComponents)
	{
		if (IsValid(Component)
			&& Component->GetTargetMovementComponent() == MovementComponent
			&& Component->bIsRotating)
		{
			Component->StopDrag();
		}
	}

	AutomaticPanelAnimations.RemoveAll(
		[MovementComponent](const FDragItemAutomaticPanelAnimation& Animation)
		{
			return Animation.MovementComponent.Get() == MovementComponent;
		});

	FDragItemAutomaticPanelAnimation& Animation = AutomaticPanelAnimations.AddDefaulted_GetRef();
	Animation.MovementComponent = MovementComponent;
	Animation.MovementComponentName = MovementComponentName;
	Animation.bLinear = bLinear;
	Animation.bOpening = bOpening;
	Animation.Duration = FMath::Max(Duration, KINDA_SMALL_NUMBER);
	Animation.StartLocation = MovementComponent->GetRelativeLocation();
	Animation.TargetLocation = TargetLocation;
	Animation.StartRotation = MovementComponent->GetRelativeRotation().GetNormalized();
	Animation.TargetRotation = TargetRotation.GetNormalized();
	StartMoveSound(bLinear);
	RefreshActiveTickState();
}

void ADrag_Item::CancelNativeAnimationForAutomaticInteraction(
	USceneComponent* MovementComponent)
{
	if (MovementComponent == GetPrimaryDoorMovementComponent())
	{
		bDoorAnimationActive = false;
	}
	RefreshActiveTickState();
}

void ADrag_Item::UpdateAutomaticPanelAnimations(float DeltaTime)
{
	if (AutomaticPanelAnimations.IsEmpty())
	{
		return;
	}

	for (int32 Index = AutomaticPanelAnimations.Num() - 1; Index >= 0; --Index)
	{
		FDragItemAutomaticPanelAnimation& Animation = AutomaticPanelAnimations[Index];
		USceneComponent* MovementComponent = Animation.MovementComponent.Get();
		if (!IsValid(MovementComponent))
		{
			AutomaticPanelAnimations.RemoveAtSwap(Index);
			continue;
		}

		Animation.Elapsed += DeltaTime;
		const float Alpha = FMath::Clamp(Animation.Elapsed / Animation.Duration, 0.0f, 1.0f);
		const float EasedAlpha = FMath::InterpEaseInOut(
			0.0f,
			1.0f,
			Alpha,
			FMath::Max(1.0f, AutomaticOpenCloseEaseExponent));

		if (Animation.bLinear)
		{
			MovementComponent->SetRelativeLocation(FMath::Lerp(
				Animation.StartLocation,
				Animation.TargetLocation,
				EasedAlpha));
		}
		else
		{
			const auto LerpAngle = [EasedAlpha](float Start, float Target)
			{
				return Start + FMath::FindDeltaAngleDegrees(Start, Target) * EasedAlpha;
			};
			MovementComponent->SetRelativeRotation(FRotator(
				LerpAngle(Animation.StartRotation.Pitch, Animation.TargetRotation.Pitch),
				LerpAngle(Animation.StartRotation.Yaw, Animation.TargetRotation.Yaw),
				LerpAngle(Animation.StartRotation.Roll, Animation.TargetRotation.Roll)));
		}

		if (Alpha < 1.0f)
		{
			continue;
		}

		MovementComponent->SetRelativeLocation(Animation.TargetLocation);
		MovementComponent->SetRelativeRotation(Animation.TargetRotation);
		if (HasAuthority())
		{
			if (Animation.bLinear)
			{
				ApplyShelfPositionFromServer(
					Animation.MovementComponentName,
					Animation.TargetLocation);
			}
			else
			{
				ApplyDoorRotationFromServer(
					Animation.MovementComponentName,
					Animation.TargetRotation);
			}
		}

		AutomaticPanelAnimations.RemoveAtSwap(Index);
	}

	if (AutomaticPanelAnimations.IsEmpty())
	{
		StopMoveSound();
		RefreshActiveTickState();
	}
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

	RefreshActiveTickState();
}

void ADrag_Item::RefreshActiveTickState()
{
	RefreshItemTickEnabled(
		bDoorAnimationActive
		|| !AutomaticPanelAnimations.IsEmpty()
		|| bShowDoorDebugOnScreen
		|| RequiresAdditionalActiveTick());
}

// Called every frame
void ADrag_Item::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    UpdateDoorAnimation(DeltaTime);
	UpdateAutomaticPanelAnimations(DeltaTime);

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

	RefreshActiveTickState();
}


