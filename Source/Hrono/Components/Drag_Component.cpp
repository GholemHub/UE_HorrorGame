// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Drag_Component.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Items/Drag_Item.h"
#include "HronoCharacter.h"
#include "InputCoreTypes.h"


// Sets default values for this component's properties
UDrag_Component::UDrag_Component()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UDrag_Component::BeginPlay()
{
	Super::BeginPlay();

	if (USceneComponent* MovementComponent = GetTargetMovementComponent())
	{
		CupBoardClosedLocation = MovementComponent->GetRelativeLocation();
		ShelfClosedLocation = MovementComponent->GetRelativeLocation();
	}
	
}

USceneComponent* UDrag_Component::GetTargetMovementComponent() const
{
	if (IsValid(TargetMovementComponentOverride))
	{
		return TargetMovementComponentOverride;
	}

	if (const ADrag_Item* DragItem = Cast<ADrag_Item>(GetOwner()))
	{
		return DragItem->GetPrimaryDoorMovementComponent();
	}

	return nullptr;
}

UPrimitiveComponent* UDrag_Component::GetInteractionPrimitive() const
{
	if (IsValid(InteractionPrimitiveOverride))
	{
		return InteractionPrimitiveOverride;
	}

	if (const ADrag_Item* DragItem = Cast<ADrag_Item>(GetOwner()))
	{
		return DragItem->ItemMesh;
	}

	return nullptr;
}

bool UDrag_Component::MatchesHitComponent(const UPrimitiveComponent* HitComponent) const
{
	if (!IsValid(HitComponent))
	{
		return false;
	}

	const UPrimitiveComponent* InteractionPrimitive = GetInteractionPrimitive();
	return InteractionPrimitive == HitComponent
		|| (InteractionPrimitive && HitComponent->IsAttachedTo(InteractionPrimitive));
}


// Called every frame
void UDrag_Component::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!bIsRotating || !RotatingController)
	{
		return;
	}

	if (!RotatingController->IsInputKeyDown(EKeys::LeftMouseButton))
	{
		StopDrag();
		return;
	}

	
	if (bIsCupBoard)
	{
		CupBoardDrag();
	}
	else if (bIsShelf)
	{
		ShelfDrag();
	}
	else
	{
		XDrag();
	}
	
	
}

void UDrag_Component::StartDrag(APlayerController* PC, FVector WorldGrabPoint)
{
	if (!PC) return;

	bIsRotating = true;
	RotatingController = PC;
	ActiveDoorSideMultiplier = 1.0f;
	ActiveViewRelativeYawDirection = DoorMouseInputDirection;
	bHasActiveViewRelativeYawDirection = false;
	bPlayerBehindDoor = false;
	bHasGrabPoint = false;
	GrabDistance = 0.0f;

	USceneComponent* MovementComponent = GetTargetMovementComponent();
	FVector ViewLocation = FVector::ZeroVector;
	FRotator ViewRotation = FRotator::ZeroRotator;
	PC->GetPlayerViewPoint(ViewLocation, ViewRotation);

	if (IsValid(MovementComponent) && !WorldGrabPoint.ContainsNaN())
	{
		GrabPointLocal = MovementComponent->GetComponentTransform()
			.InverseTransformPosition(WorldGrabPoint);
		GrabDistance = FVector::Distance(ViewLocation, WorldGrabPoint);
		bHasGrabPoint = GrabDistance > KINDA_SMALL_NUMBER;
	}

	float PlayerFrontDot = 0.0f;
	if (bUseWardrobeFrontBackInput)
	{
		if (const AActor* Owner = GetOwner())
		{
			if (const APawn* PlayerPawn = PC->GetPawn())
			{
				FVector DoorToPlayer = PlayerPawn->GetActorLocation() - Owner->GetActorLocation();
				DoorToPlayer.Z = 0.0f;

				if (!DoorToPlayer.IsNearlyZero())
				{
					PlayerFrontDot = FVector::DotProduct(
						Owner->GetActorForwardVector(),
						DoorToPlayer.GetSafeNormal());

					bPlayerBehindDoor = PlayerFrontDot < 0.0f;
					if (bInvertDoorFrontSide)
					{
						bPlayerBehindDoor = !bPlayerBehindDoor;
					}

					if (bPlayerBehindDoor)
					{
						ActiveDoorSideMultiplier = -1.0f;
					}
				}
			}
		}
	}
	else if (const AActor* Owner = GetOwner())
	{
		if (const APawn* PlayerPawn = PC->GetPawn())
		{
			FVector DoorToPlayer = PlayerPawn->GetActorLocation() - Owner->GetActorLocation();
			DoorToPlayer.Z = 0.0f;
			if (!DoorToPlayer.IsNearlyZero())
			{
				const float HingeSide = FVector::DotProduct(
					Owner->GetActorRightVector(),
					DoorToPlayer.GetSafeNormal());
				ActiveDoorSideMultiplier = HingeSide < 0.0f ? -1.0f : 1.0f;
			}
		}
	}

	UE_LOG(LogTemp, Log,
		TEXT("[DoorDragStart] Owner=%s DragComponent=%s HitPrimitive=%s Target=%s InputMode=%s PlayerSide=%s FrontDot=%.3f SideMultiplier=%.1f MouseDir=%.1f CustomLimits=%s Limits=[%.1f, %.1f]"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(this),
		*GetNameSafe(GetInteractionPrimitive()),
		*GetNameSafe(GetTargetMovementComponent()),
		bUseWardrobeFrontBackInput ? TEXT("WardrobeFrontBack") : TEXT("OrdinaryDoor"),
		bUseWardrobeFrontBackInput
			? (bPlayerBehindDoor ? TEXT("Behind") : TEXT("Front"))
			: (ActiveDoorSideMultiplier < 0.0f ? TEXT("HingeNegative") : TEXT("HingePositive")),
		PlayerFrontDot,
		ActiveDoorSideMultiplier,
		DoorMouseInputDirection,
		bUseCustomDoorAngleLimits ? TEXT("true") : TEXT("false"),
		MinimumDoorYaw,
		MaximumDoorYaw);

	// Start the looping movement sound on the owning door/shelf actor.
	if (ADrag_Item* Drag_Item = Cast<ADrag_Item>(GetOwner()))
	{
		const bool bLinearDrag = bIsShelf || bIsCupBoard;
		Drag_Item->StartMoveSound(bLinearDrag);

		// Notify Blueprints that a drag interaction has begun.
		Drag_Item->NotifyDragStarted(bLinearDrag);
	}

	//UE_LOG(LogTemp, Log, TEXT("Drag started"));
}

void UDrag_Component::StopDrag()
{
	bIsRotating = false;
	RotatingController = nullptr;
	bHasGrabPoint = false;

	// Stop the looping movement sound on the owning door/shelf actor.
	if (ADrag_Item* Drag_Item = Cast<ADrag_Item>(GetOwner()))
	{
		Drag_Item->StopMoveSound();
	}

	//UE_LOG(LogTemp, Log, TEXT("Drag stopped"));
}

bool UDrag_Component::UpdateGrabTarget(FVector& OutTargetPoint)
{
	if (!IsValid(RotatingController) || !bHasGrabPoint)
	{
		return false;
	}

	FVector ViewLocation = FVector::ZeroVector;
	FRotator ViewRotation = FRotator::ZeroRotator;
	RotatingController->GetPlayerViewPoint(ViewLocation, ViewRotation);

	float MouseX = 0.0f;
	float MouseY = 0.0f;
	RotatingController->GetInputMouseDelta(MouseX, MouseY);
	static_cast<void>(MouseX);

	GrabDistance = FMath::Clamp(
		GrabDistance - MouseY * VerticalGrabPullSpeed,
		FMath::Min(MinimumGrabDistance, MaximumGrabDistance),
		FMath::Max(MinimumGrabDistance, MaximumGrabDistance));
	OutTargetPoint = ViewLocation + ViewRotation.Vector() * GrabDistance;
	return !OutTargetPoint.ContainsNaN();
}

bool UDrag_Component::CalculateLinearGrabLocation(
	USceneComponent* MovementComponent,
	const FVector& ClosedLocation,
	const FVector& LocalSlideAxis,
	float MaximumDistance,
	FVector& OutRelativeLocation)
{
	if (!IsValid(MovementComponent) || !bHasGrabPoint)
	{
		return false;
	}

	const FVector SlideAxis = LocalSlideAxis.GetSafeNormal();
	if (SlideAxis.IsNearlyZero())
	{
		return false;
	}

	FVector TargetPoint = FVector::ZeroVector;
	if (!UpdateGrabTarget(TargetPoint))
	{
		return false;
	}

	const FVector CurrentGrabPoint = MovementComponent->GetComponentTransform()
		.TransformPosition(GrabPointLocal);
	FVector TargetDeltaInParentSpace = TargetPoint - CurrentGrabPoint;
	if (const USceneComponent* Parent = MovementComponent->GetAttachParent())
	{
		TargetDeltaInParentSpace = Parent->GetComponentTransform()
			.InverseTransformVector(TargetDeltaInParentSpace);
	}

	const FVector CurrentRelativeLocation = MovementComponent->GetRelativeLocation();
	const float CurrentOffset = FVector::DotProduct(
		CurrentRelativeLocation - ClosedLocation,
		SlideAxis);
	const float RequestedOffset = CurrentOffset
		+ FVector::DotProduct(TargetDeltaInParentSpace, SlideAxis);
	const float NewOffset = FMath::Clamp(
		RequestedOffset,
		0.0f,
		FMath::Max(0.0f, MaximumDistance));

	OutRelativeLocation = ClosedLocation + SlideAxis * NewOffset;
	return true;
}

void UDrag_Component::DoorGrab(float DeltaTime)
{
	ADrag_Item* DragItem = Cast<ADrag_Item>(GetOwner());
	USceneComponent* MovementComponent = GetTargetMovementComponent();
	UPrimitiveComponent* InteractionPrimitive = GetInteractionPrimitive();
	APawn* PlayerPawn = RotatingController ? RotatingController->GetPawn() : nullptr;
	if (!IsValid(DragItem)
		|| !IsValid(MovementComponent)
		|| !IsValid(InteractionPrimitive)
		|| !IsValid(PlayerPawn)
		|| !bHasGrabPoint
		|| DeltaTime <= 0.0f)
	{
		return;
	}

	FVector TargetPoint = FVector::ZeroVector;
	if (!UpdateGrabTarget(TargetPoint))
	{
		return;
	}

	// The target follows both sources of player motion: translation moves the
	// camera origin, mouse X/Y changes ViewRotation, and mouse Y also pushes or
	// pulls the target along the ray so vertical input always affects the hinge.
	const FVector HingeLocation = MovementComponent->GetComponentLocation();

	FVector HingeAxis = FVector::UpVector;
	if (const USceneComponent* Parent = MovementComponent->GetAttachParent())
	{
		HingeAxis = Parent->GetUpVector();
	}
	else if (const AActor* Owner = GetOwner())
	{
		HingeAxis = Owner->GetActorUpVector();
	}
	HingeAxis.Normalize();

	const FVector CurrentGrabPoint = MovementComponent->GetComponentTransform()
		.TransformPosition(GrabPointLocal);
	FVector CurrentRadius = FVector::VectorPlaneProject(
		CurrentGrabPoint - HingeLocation,
		HingeAxis);
	FVector TargetRadius = FVector::VectorPlaneProject(
		TargetPoint - HingeLocation,
		HingeAxis);

	if (!CurrentRadius.Normalize() || !TargetRadius.Normalize())
	{
		return;
	}

	const float SignedAngleToTarget = FMath::RadiansToDegrees(FMath::Atan2(
		FVector::DotProduct(HingeAxis, FVector::CrossProduct(CurrentRadius, TargetRadius)),
		FVector::DotProduct(CurrentRadius, TargetRadius)));
	const float FollowAlpha = FMath::Clamp(DeltaTime * DoorGrabFollowSpeed, 0.0f, 1.0f);
	const float MaximumStep = MaximumDoorAngularSpeed * DeltaTime;
	const float DoorAngleStep = FMath::Clamp(
		SignedAngleToTarget * FollowAlpha,
		-MaximumStep,
		MaximumStep);

	const FRotator OldRotation = MovementComponent->GetRelativeRotation();
	FRotator NewRotation = OldRotation;
	const float CurrentYaw = FMath::UnwindDegrees(OldRotation.Yaw);
	float MinimumYaw = -90.0f;
	float MaximumYaw = 0.0f;

	if (bUseCustomDoorAngleLimits)
	{
		MinimumYaw = FMath::Min(MinimumDoorYaw, MaximumDoorYaw);
		MaximumYaw = FMath::Max(MinimumDoorYaw, MaximumDoorYaw);
	}
	else if (DragItem->ItemType == EItemType::DraggableInvertLeft)
	{
		MinimumYaw = 0.0f;
		MaximumYaw = 90.0f;
	}
	else if (DragItem->ItemType == EItemType::DraggableInvertRight)
	{
		MinimumYaw = -90.0f;
		MaximumYaw = 0.0f;
	}

	NewRotation.Yaw = FMath::Clamp(
		CurrentYaw + DoorAngleStep,
		MinimumYaw,
		MaximumYaw);

	if (InteractionPrimitive->IsOverlappingActor(PlayerPawn))
	{
		NewRotation = OldRotation;
	}

	MovementComponent->SetRelativeRotation(NewRotation);

	if (AHronoCharacter* Character = Cast<AHronoCharacter>(PlayerPawn))
	{
		if (!Character->HasAuthority())
		{
			Character->Server_SetDoorPanelRotation(
				DragItem,
				MovementComponent->GetFName(),
				NewRotation);
		}
		else
		{
			DragItem->ApplyDoorRotationFromServer(
				MovementComponent->GetFName(),
				NewRotation);
		}
	}
}

void UDrag_Component::XDrag()
{
	if (bHasGrabPoint)
	{
		DoorGrab(GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.0f);
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner || !RotatingController) return;

	auto Drag_Item = Cast<ADrag_Item>(Owner);
	if (!Drag_Item) return;

	USceneComponent* MovementComponent = GetTargetMovementComponent();
	UPrimitiveComponent* InteractionPrimitive = GetInteractionPrimitive();
	if (!MovementComponent || !InteractionPrimitive)
	{
		return;
	}

	APawn* PlayerPawn = RotatingController->GetPawn();
	if (!PlayerPawn) return;

	float MouseX, MouseY;
	RotatingController->GetInputMouseDelta(MouseX, MouseY);
	if (const AHronoCharacter* Character = Cast<AHronoCharacter>(PlayerPawn))
	{
		if (Character->bCorrectDragInputWhenMirrored)
		{
			MouseX *= Character->GetMirroredHorizontalInputScale();
		}
	}

	float InputSideMultiplier = ActiveDoorSideMultiplier;
	if (!bUseWardrobeFrontBackInput)
	{
		// Preserve the original ordinary-door behavior. These actors were authored
		// around the hinge-side (Right axis) test, not the wardrobe Forward axis.
		FVector DoorToPlayer = PlayerPawn->GetActorLocation() - Owner->GetActorLocation();
		DoorToPlayer.Z = 0.0f;
		if (!DoorToPlayer.IsNearlyZero())
		{
			const float HingeSide = FVector::DotProduct(
				Owner->GetActorRightVector(),
				DoorToPlayer.GetSafeNormal());
			InputSideMultiplier = HingeSide < 0.0f ? -1.0f : 1.0f;
		}
	}

	// A closed hinged panel initially moves mostly into screen depth, so its camera
	// projection can be nearly zero. Wardrobes must then keep their authored -1
	// fallback: mouse-right opens the negative-Yaw/right panel and mouse-left opens
	// the positive-Yaw/left panel, regardless of which side was detected.
	float EffectiveYawInputDirection = DoorMouseInputDirection
		* (bUseWardrobeFrontBackInput ? 1.0f : InputSideMultiplier);
	if (bUseWardrobeFrontBackInput)
	{
		if (!bHasActiveViewRelativeYawDirection)
		{
			// Bounds.Origin remains the visual center even when the imported mesh pivot
			// itself is located on the hinge. Calculate the view-relative sign only once
			// per drag. Recalculating while the panel rotates can flip the projection at
			// the closed angle and make continued closing input reopen the panel.
			FVector DoorRadius = InteractionPrimitive->Bounds.Origin - MovementComponent->GetComponentLocation();
			DoorRadius.Z = 0.0f;

			FVector CameraRight = FRotationMatrix(RotatingController->GetControlRotation()).GetUnitAxis(EAxis::Y);
			CameraRight.Z = 0.0f;
			CameraRight.Normalize();

			const FVector PositiveYawTangent =
				FVector::CrossProduct(FVector::UpVector, DoorRadius).GetSafeNormal();
			const float ScreenProjection = FVector::DotProduct(PositiveYawTangent, CameraRight);
			ActiveViewRelativeYawDirection = EffectiveYawInputDirection;
			if (!PositiveYawTangent.IsNearlyZero()
				&& !CameraRight.IsNearlyZero()
				&& FMath::Abs(ScreenProjection) > 0.05f)
			{
				ActiveViewRelativeYawDirection = ScreenProjection < 0.0f ? -1.0f : 1.0f;
			}
			bHasActiveViewRelativeYawDirection = true;

			UE_LOG(LogTemp, Log,
				TEXT("[WardrobeDoorInput] Owner=%s Panel=%s ScreenProjection=%.3f LockedYawDirection=%.1f"),
				*GetNameSafe(Owner),
				*GetNameSafe(InteractionPrimitive),
				ScreenProjection,
				ActiveViewRelativeYawDirection);
		}

		EffectiveYawInputDirection = ActiveViewRelativeYawDirection;
	}

	FRotator OldRotation = MovementComponent->GetRelativeRotation();

	FRotator NewRotation = OldRotation;

	if (bUseCustomDoorAngleLimits)
	{
		const float MinYaw = FMath::Min(MinimumDoorYaw, MaximumDoorYaw);
		const float MaxYaw = FMath::Max(MinimumDoorYaw, MaximumDoorYaw);
		NewRotation.Yaw = FMath::Clamp(
			FMath::UnwindDegrees(NewRotation.Yaw)
				+ MouseX * RotationSpeed * EffectiveYawInputDirection,
			MinYaw,
			MaxYaw);
	}
	else if (Drag_Item->ItemType == EItemType::DraggableInvertLeft)
	{
		// From the front, mouse-left opens the left panel (positive Yaw).
		// Wardrobes use front/back input; ordinary doors use their hinge side.
		NewRotation.Yaw = FMath::Clamp(
			NewRotation.Yaw + MouseX * RotationSpeed
				* (bUseWardrobeFrontBackInput ? EffectiveYawInputDirection : -InputSideMultiplier),
			0.f,
			90.f
		);
	}
	else if (Drag_Item->ItemType == EItemType::DraggableInvertRight)
	{
		// From the front, mouse-right opens the right panel (negative Yaw).
		NewRotation.Yaw = FMath::Clamp(
			NewRotation.Yaw + MouseX * RotationSpeed
				* (bUseWardrobeFrontBackInput ? EffectiveYawInputDirection : -InputSideMultiplier),
			-90.f,
			0.f
			
		);
	}
	else 
	{
		NewRotation.Yaw = FMath::Clamp(
			NewRotation.Yaw + MouseX * RotationSpeed
				* (bUseWardrobeFrontBackInput ? EffectiveYawInputDirection : InputSideMultiplier),
			-90.f,
			0.f
		);
	}


	bool bOverlappingPlayer = InteractionPrimitive->IsOverlappingActor(PlayerPawn);
	if (bOverlappingPlayer)
	{
		NewRotation = OldRotation;
	}

	// Apply rotation locally for immediate feedback (prediction)
	MovementComponent->SetRelativeRotation(NewRotation);

	// Send the new rotation to the server so it updates the authoritative collision
	// body and replicates it to every other client. The door is a level actor and
	// cannot receive client RPCs directly, so we route through the owning character.
	if (AHronoCharacter* Character = Cast<AHronoCharacter>(RotatingController->GetPawn()))
	{
		if (!Character->HasAuthority())
		{
			Character->Server_SetDoorPanelRotation(
				Drag_Item,
				MovementComponent->GetFName(),
				NewRotation);
		}
		else
		{
			Drag_Item->ApplyDoorRotationFromServer(MovementComponent->GetFName(), NewRotation);
		}
	}
}



void UDrag_Component::ShelfDrag()
{
	ADrag_Item* Shelf = Cast<ADrag_Item>(GetOwner());
	USceneComponent* MovementComponent = GetTargetMovementComponent();
	UPrimitiveComponent* InteractionPrimitive = GetInteractionPrimitive();
	if (!Shelf || !MovementComponent || !InteractionPrimitive || !RotatingController)
	{
		return;
	}

	APawn* PlayerPawn = RotatingController->GetPawn();
	if (!PlayerPawn)
	{
		return;
	}

	const FVector SlideAxis = ShelfSlideAxis.GetSafeNormal();
	if (SlideAxis.IsNearlyZero())
	{
		return;
	}

	const FVector OldRelativeLocation = MovementComponent->GetRelativeLocation();
	FVector NewRelativeLocation = OldRelativeLocation;
	if (!CalculateLinearGrabLocation(
		MovementComponent,
		ShelfClosedLocation,
		SlideAxis,
		ShelfMaxDistance,
		NewRelativeLocation))
	{
		// Compatibility fallback for a drag started without a valid hit point.
		float MouseX = 0.0f;
		float MouseY = 0.0f;
		RotatingController->GetInputMouseDelta(MouseX, MouseY);
		static_cast<void>(MouseX);
		const float CurrentOffset = FVector::DotProduct(
			OldRelativeLocation - ShelfClosedLocation,
			SlideAxis);
		const float NewOffset = FMath::Clamp(
			CurrentOffset - MouseY * ShelfSpeed,
			0.0f,
			ShelfMaxDistance);
		NewRelativeLocation = ShelfClosedLocation + SlideAxis * NewOffset;
	}
	if (InteractionPrimitive->IsOverlappingActor(PlayerPawn))
	{
		NewRelativeLocation = OldRelativeLocation;
	}

	// Immediate local prediction keeps the drawer responsive for the player who
	// is holding it. The character RPC below updates the authoritative component.
	MovementComponent->SetRelativeLocation(NewRelativeLocation);

	if (AHronoCharacter* Character = Cast<AHronoCharacter>(PlayerPawn))
	{
		if (!Character->HasAuthority())
		{
			Character->Server_SetShelfPanelPosition(
				Shelf,
				MovementComponent->GetFName(),
				NewRelativeLocation);
		}
		else
		{
			Shelf->ApplyShelfPositionFromServer(
				MovementComponent->GetFName(),
				NewRelativeLocation);
		}
	}
}

void UDrag_Component::CupBoardDrag()
{
	ADrag_Item* CupBoard = Cast<ADrag_Item>(GetOwner());
	USceneComponent* MovementComponent = GetTargetMovementComponent();
	UPrimitiveComponent* InteractionPrimitive = GetInteractionPrimitive();
	if (!CupBoard || !MovementComponent || !InteractionPrimitive || !RotatingController)
	{
		return;
	}

	APawn* PlayerPawn = RotatingController->GetPawn();
	if (!PlayerPawn)
	{
		return;
	}

	const FVector SlideAxis = CupBoardSlideAxis.GetSafeNormal();
	if (SlideAxis.IsNearlyZero())
	{
		return;
	}

	const FVector OldRelativeLocation = MovementComponent->GetRelativeLocation();
	FVector NewRelativeLocation = OldRelativeLocation;
	if (!CalculateLinearGrabLocation(
		MovementComponent,
		CupBoardClosedLocation,
		SlideAxis,
		CupBoardMaxDistance,
		NewRelativeLocation))
	{
		// Compatibility fallback for a drag started without a valid hit point.
		float MouseX = 0.0f;
		float MouseY = 0.0f;
		RotatingController->GetInputMouseDelta(MouseX, MouseY);
		static_cast<void>(MouseY);
		if (const AHronoCharacter* Character = Cast<AHronoCharacter>(PlayerPawn))
		{
			if (Character->bCorrectDragInputWhenMirrored)
			{
				MouseX *= Character->GetMirroredHorizontalInputScale();
			}
		}

		const float CurrentOffset = FVector::DotProduct(
			OldRelativeLocation - CupBoardClosedLocation,
			SlideAxis);
		const float NewOffset = FMath::Clamp(
			CurrentOffset - MouseX * CupBoardSlideSpeed,
			0.0f,
			CupBoardMaxDistance);
		NewRelativeLocation = CupBoardClosedLocation + SlideAxis * NewOffset;
	}

	if (InteractionPrimitive->IsOverlappingActor(PlayerPawn))
	{
		NewRelativeLocation = OldRelativeLocation;
	}

	MovementComponent->SetRelativeLocation(NewRelativeLocation);
	CupBoard->ShelfPosition = NewRelativeLocation;

	if (AHronoCharacter* Character = Cast<AHronoCharacter>(PlayerPawn))
	{
		if (!Character->HasAuthority())
		{
			Character->Server_SetShelfPosition(CupBoard, NewRelativeLocation);
		}
		else
		{
			CupBoard->ApplyShelfPositionFromServer(
				MovementComponent->GetFName(),
				NewRelativeLocation);
		}
	}
}
