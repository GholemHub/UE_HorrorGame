#include "Items/HidingWardrobe.h"

#include "Components/Drag_Component.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HronoCharacter.h"
#include "HronoCollisionChannels.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

AHidingWardrobe::AHidingWardrobe()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	LeftDoorPivot = CreateDefaultSubobject<USceneComponent>(TEXT("LeftDoorPivot"));
	LeftDoorPivot->SetupAttachment(FrameMesh);

	RightDoorPivot = CreateDefaultSubobject<USceneComponent>(TEXT("RightDoorPivot"));
	RightDoorPivot->SetupAttachment(FrameMesh);

	// ItemMesh is inherited from Drag_Item and serves as the left door panel.
	ItemMesh->SetupAttachment(LeftDoorPivot);

	RightDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightDoorMesh"));
	RightDoorMesh->SetupAttachment(RightDoorPivot);
	RightDoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	RightDoorMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	DragComponent->TargetMovementComponentOverride = LeftDoorPivot;
	DragComponent->InteractionPrimitiveOverride = ItemMesh;
	DragComponent->bUseCustomDoorAngleLimits = true;
	DragComponent->MinimumDoorYaw = 0.0f;
	DragComponent->MaximumDoorYaw = 110.0f;
	// The left panel opens toward positive Yaw, but dragging its handle outward
	// produces negative MouseX. Invert that input so mouse-left opens the door.
	DragComponent->DoorMouseInputDirection = -1.0f;

	RightDoorDragComponent = CreateDefaultSubobject<UDrag_Component>(TEXT("RightDoorDragComponent"));
	RightDoorDragComponent->TargetMovementComponentOverride = RightDoorPivot;
	RightDoorDragComponent->InteractionPrimitiveOverride = RightDoorMesh;
	RightDoorDragComponent->bUseCustomDoorAngleLimits = true;
	RightDoorDragComponent->MinimumDoorYaw = -110.0f;
	RightDoorDragComponent->MaximumDoorYaw = 0.0f;
	RightDoorDragComponent->DoorMouseInputDirection = -1.0f;

	HidingPoint = CreateDefaultSubobject<USceneComponent>(TEXT("HidingPoint"));
	HidingPoint->SetupAttachment(SceneRoot);

	ExitPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ExitPoint"));
	ExitPoint->SetupAttachment(SceneRoot);

	// Super::AnimateDoor uses this value for the primary/left panel.
	ItemType = EItemType::DraggableInvertLeft;
	AnimatedDoorOpenAngle = 105.0f;
}

void AHidingWardrobe::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHidingWardrobe, RightDoorRotation);
	DOREPLIFETIME(AHidingWardrobe, HiddenPlayer);
}

void AHidingWardrobe::BeginPlay()
{
	Super::BeginPlay();

	// Existing Blueprint children can keep the component-template values that were
	// serialized before the native defaults changed. Apply the wardrobe contract
	// at runtime so the two panels always react like real outward-opening doors:
	// left mouse movement opens the left panel, right movement opens the right one.
	if (DragComponent)
	{
		DragComponent->TargetMovementComponentOverride = LeftDoorPivot;
		DragComponent->InteractionPrimitiveOverride = ItemMesh;
		DragComponent->bUseCustomDoorAngleLimits = true;
		DragComponent->MinimumDoorYaw = 0.0f;
		DragComponent->MaximumDoorYaw = 110.0f;
		DragComponent->DoorMouseInputDirection = -1.0f;
	}

	if (RightDoorDragComponent)
	{
		RightDoorDragComponent->TargetMovementComponentOverride = RightDoorPivot;
		RightDoorDragComponent->InteractionPrimitiveOverride = RightDoorMesh;
		RightDoorDragComponent->bUseCustomDoorAngleLimits = true;
		RightDoorDragComponent->MinimumDoorYaw = -110.0f;
		RightDoorDragComponent->MaximumDoorYaw = 0.0f;
		RightDoorDragComponent->DoorMouseInputDirection = -1.0f;
	}

	UE_LOG(LogTemp, Log,
		TEXT("[WardrobeDragConfig] %s | Left: MouseDir=%.1f Limits=[%.1f, %.1f] Target=%s | Right: MouseDir=%.1f Limits=[%.1f, %.1f] Target=%s"),
		*GetNameSafe(this),
		DragComponent ? DragComponent->DoorMouseInputDirection : 0.0f,
		DragComponent ? DragComponent->MinimumDoorYaw : 0.0f,
		DragComponent ? DragComponent->MaximumDoorYaw : 0.0f,
		*GetNameSafe(LeftDoorPivot),
		RightDoorDragComponent ? RightDoorDragComponent->DoorMouseInputDirection : 0.0f,
		RightDoorDragComponent ? RightDoorDragComponent->MinimumDoorYaw : 0.0f,
		RightDoorDragComponent ? RightDoorDragComponent->MaximumDoorYaw : 0.0f,
		*GetNameSafe(RightDoorPivot));

	ConfigureRightDoorCollision();

	if (HasAuthority())
	{
		DoorRotation = LeftDoorPivot->GetRelativeRotation();
		RightDoorRotation = RightDoorPivot->GetRelativeRotation();
		RefreshDoorClosedState();
	}
}

void AHidingWardrobe::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(HiddenPlayer))
	{
		ApplyHidingState(HiddenPlayer, false);
	}

	Super::EndPlay(EndPlayReason);
}

void AHidingWardrobe::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateRightDoorAnimation(DeltaTime);
}

USceneComponent* AHidingWardrobe::GetPrimaryDoorMovementComponent() const
{
	return LeftDoorPivot;
}

USceneComponent* AHidingWardrobe::FindDoorMovementComponent(FName DoorComponentName) const
{
	if (RightDoorPivot
		&& (DoorComponentName == RightDoorPivot->GetFName()
			|| (RightDoorMesh && DoorComponentName == RightDoorMesh->GetFName())))
	{
		return RightDoorPivot;
	}

	if (LeftDoorPivot
		&& (DoorComponentName.IsNone()
			|| DoorComponentName == LeftDoorPivot->GetFName()
			|| (ItemMesh && DoorComponentName == ItemMesh->GetFName())))
	{
		return LeftDoorPivot;
	}

	return Super::FindDoorMovementComponent(DoorComponentName);
}

void AHidingWardrobe::ApplyDoorRotationFromServer(
	FName DoorComponentName,
	const FRotator& NewRotation)
{
	if (!HasAuthority())
	{
		return;
	}

	if (FindDoorMovementComponent(DoorComponentName) == RightDoorPivot)
	{
		RightDoorPivot->SetRelativeRotation(NewRotation);
		RightDoorRotation = NewRotation;
		RefreshDoorClosedState();
		ForceNetUpdate();
		return;
	}

	Super::ApplyDoorRotationFromServer(DoorComponentName, NewRotation);
}

void AHidingWardrobe::OnRep_RightDoorRotation()
{
	if (!RightDoorPivot
		|| bRightDoorAnimationActive
		|| (RightDoorDragComponent && RightDoorDragComponent->bIsRotating))
	{
		return;
	}

	RightDoorPivot->SetRelativeRotation(RightDoorRotation);
}

void AHidingWardrobe::RefreshDoorClosedState()
{
	if (!HasAuthority())
	{
		return;
	}

	const bool bLeftClosed = FMath::Abs(FMath::UnwindDegrees(DoorRotation.Yaw)) <= DoorClosedYawTolerance;
	const bool bRightClosed = FMath::Abs(FMath::UnwindDegrees(RightDoorRotation.Yaw)) <= DoorClosedYawTolerance;
	const bool bNewClosed = bLeftClosed && bRightClosed;
	if (bNewClosed == bIsClosed)
	{
		return;
	}

	bIsClosed = bNewClosed;
	UE_LOG(LogTemp, Log, TEXT("[SERVER] Wardrobe doors %s"), bIsClosed ? TEXT("closed") : TEXT("open"));
	UGameplayStatics::PlaySoundAtLocation(
		this,
		bIsClosed ? DoorCloseSound : DoorOpenSound,
		GetActorLocation());
	OnDoorStateChanged.Broadcast(bIsClosed);
}

void AHidingWardrobe::AnimateDoor(bool bOpen)
{
	if (!HasAuthority() || !bAllowAnimateDoorOpenClose || !RightDoorPivot)
	{
		return;
	}

	Super::AnimateDoor(bOpen);

	FRotator TargetRotation = RightDoorPivot->GetRelativeRotation();
	TargetRotation.Yaw = bOpen ? -FMath::Abs(RightDoorOpenAngle) : 0.0f;
	if (RightDoorPivot->GetRelativeRotation().Equals(TargetRotation, 0.01f))
	{
		RightDoorRotation = TargetRotation;
		RefreshDoorClosedState();
		ForceNetUpdate();
		return;
	}

	MulticastStartRightDoorAnimation(
		TargetRotation,
		FMath::Max(DoorAnimationDuration, KINDA_SMALL_NUMBER));
}

void AHidingWardrobe::MulticastStartRightDoorAnimation_Implementation(
	FRotator TargetRotation,
	float Duration)
{
	if (!RightDoorPivot)
	{
		return;
	}

	if (RightDoorDragComponent && RightDoorDragComponent->bIsRotating)
	{
		RightDoorDragComponent->StopDrag();
	}

	RightDoorAnimationStartRotation = RightDoorPivot->GetRelativeRotation().GetNormalized();
	RightDoorAnimationTargetRotation = TargetRotation.GetNormalized();
	RightDoorAnimationElapsed = 0.0f;
	ActiveRightDoorAnimationDuration = FMath::Max(Duration, KINDA_SMALL_NUMBER);
	bRightDoorAnimationActive = true;
	RightDoorPivot->SetRelativeRotation(RightDoorAnimationStartRotation);
	RightDoorRotation = RightDoorAnimationStartRotation;
}

void AHidingWardrobe::UpdateRightDoorAnimation(float DeltaTime)
{
	if (!bRightDoorAnimationActive || !RightDoorPivot)
	{
		return;
	}

	RightDoorAnimationElapsed += DeltaTime;
	const float Alpha = FMath::Clamp(
		RightDoorAnimationElapsed / ActiveRightDoorAnimationDuration,
		0.0f,
		1.0f);
	const float EasedAlpha = FMath::InterpEaseInOut(
		0.0f,
		1.0f,
		Alpha,
		FMath::Max(1.0f, DoorAnimationEaseExponent));

	const auto LerpAngle = [EasedAlpha](float Start, float Target)
	{
		return Start + FMath::FindDeltaAngleDegrees(Start, Target) * EasedAlpha;
	};

	const FRotator NewRotation(
		LerpAngle(RightDoorAnimationStartRotation.Pitch, RightDoorAnimationTargetRotation.Pitch),
		LerpAngle(RightDoorAnimationStartRotation.Yaw, RightDoorAnimationTargetRotation.Yaw),
		LerpAngle(RightDoorAnimationStartRotation.Roll, RightDoorAnimationTargetRotation.Roll));

	RightDoorPivot->SetRelativeRotation(NewRotation);
	RightDoorRotation = NewRotation;

	if (Alpha < 1.0f)
	{
		return;
	}

	bRightDoorAnimationActive = false;
	RightDoorRotation = RightDoorAnimationTargetRotation;
	RightDoorPivot->SetRelativeRotation(RightDoorRotation);
	if (HasAuthority())
	{
		RefreshDoorClosedState();
		ForceNetUpdate();
	}
}

bool AHidingWardrobe::AreDoorsOpenForHiding() const
{
	if (!bRequireBothDoorsOpenToHide)
	{
		return true;
	}

	return FMath::Abs(FMath::UnwindDegrees(DoorRotation.Yaw)) >= MinimumDoorOpenAngleToHide
		&& FMath::Abs(FMath::UnwindDegrees(RightDoorRotation.Yaw)) >= MinimumDoorOpenAngleToHide;
}

bool AHidingWardrobe::CanPlayerHide(const AHronoCharacter* Player) const
{
	return bAllowHiding
		&& IsValid(Player)
		&& !IsValid(HiddenPlayer)
		&& (ItemTimeline == EItemTimeline::Both || ItemTimeline == Player->GetTimeline())
		&& AreDoorsOpenForHiding();
}

void AHidingWardrobe::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority())
	{
		return;
	}

	AHronoCharacter* Player = Cast<AHronoCharacter>(Interactor);
	if (!Player)
	{
		return;
	}

	if (HiddenPlayer == Player)
	{
		ExitWardrobe(Player);
		return;
	}

	if (!TryEnterWardrobe(Player) && bOpenDoorsWhenHidingIsBlocked && !AreDoorsOpenForHiding())
	{
		AnimateDoor(true);
	}
}

bool AHidingWardrobe::TryEnterWardrobe(AHronoCharacter* Player)
{
	if (!HasAuthority() || !CanPlayerHide(Player))
	{
		UE_LOG(LogTemp, Log,
			TEXT("[%s] Hiding rejected. occupied=%s doorsReady=%s player=%s"),
			*GetName(),
			IsOccupied() ? TEXT("true") : TEXT("false"),
			AreDoorsOpenForHiding() ? TEXT("true") : TEXT("false"),
			*GetNameSafe(Player));
		return false;
	}

	AHronoCharacter* PreviousPlayer = HiddenPlayer;
	HiddenPlayer = Player;
	OnRep_HiddenPlayer(PreviousPlayer);
	ForceNetUpdate();
	return true;
}

bool AHidingWardrobe::ExitWardrobe(AHronoCharacter* Player)
{
	if (!HasAuthority() || !IsValid(Player) || HiddenPlayer != Player)
	{
		return false;
	}

	AHronoCharacter* PreviousPlayer = HiddenPlayer;
	HiddenPlayer = nullptr;
	OnRep_HiddenPlayer(PreviousPlayer);
	ForceNetUpdate();
	return true;
}

void AHidingWardrobe::OnRep_HiddenPlayer(AHronoCharacter* PreviousPlayer)
{
	if (IsValid(PreviousPlayer) && PreviousPlayer != HiddenPlayer)
	{
		ApplyHidingState(PreviousPlayer, false);
		OnPlayerExitedWardrobe.Broadcast(PreviousPlayer);
	}

	if (IsValid(HiddenPlayer))
	{
		ApplyHidingState(HiddenPlayer, true);
		OnPlayerEnteredWardrobe.Broadcast(HiddenPlayer);
	}
}

void AHidingWardrobe::ApplyHidingState(AHronoCharacter* Player, bool bEntering)
{
	if (!IsValid(Player))
	{
		return;
	}

	UCharacterMovementComponent* Movement = Player->GetCharacterMovement();
	if (bEntering)
	{
		if (HidingPoint)
		{
			Player->SetActorLocationAndRotation(
				HidingPoint->GetComponentLocation(),
				HidingPoint->GetComponentRotation(),
				false,
				nullptr,
				ETeleportType::TeleportPhysics);
		}

		Player->SetActorEnableCollision(false);
		if (Movement)
		{
			Movement->StopMovementImmediately();
			Movement->DisableMovement();
		}
	}
	else
	{
		if (ExitPoint)
		{
			Player->SetActorLocationAndRotation(
				ExitPoint->GetComponentLocation(),
				ExitPoint->GetComponentRotation(),
				false,
				nullptr,
				ETeleportType::TeleportPhysics);
		}

		Player->SetActorEnableCollision(true);
		if (Movement)
		{
			Movement->SetMovementMode(MOVE_Walking);
		}
	}
}

void AHidingWardrobe::ConfigureRightDoorCollision()
{
	if (!RightDoorMesh)
	{
		return;
	}

	RightDoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	RightDoorMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	if (ItemTimeline == EItemTimeline::Future)
	{
		RightDoorMesh->SetCollisionObjectType(COLLISION_CHANNEL_DOOR_FUTURE);
		RightDoorMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, ECR_Block);
		RightDoorMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, ECR_Ignore);
	}
	else if (ItemTimeline == EItemTimeline::Past)
	{
		RightDoorMesh->SetCollisionObjectType(COLLISION_CHANNEL_DOOR_PAST);
		RightDoorMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, ECR_Block);
		RightDoorMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, ECR_Ignore);
	}
	else
	{
		RightDoorMesh->SetCollisionObjectType(COLLISION_CHANNEL_DOOR_PAST);
		RightDoorMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, ECR_Block);
		RightDoorMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, ECR_Block);
	}
}
