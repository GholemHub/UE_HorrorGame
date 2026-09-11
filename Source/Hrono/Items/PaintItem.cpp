#include "Items/PaintItem.h"

#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HronoCharacter.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

APaintItem::APaintItem()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	SetReplicateMovement(false);

	if (IsValid(ItemMesh))
	{
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		ItemMesh->SetGenerateOverlapEvents(false);
		ItemMesh->SetSimulatePhysics(false);

		static ConstructorHelpers::FObjectFinder<UStaticMesh> FrameMesh(
			TEXT("/Game/Painting_Portraits_1/Geometry/Paints/Hrono_SM_PictureFrame_3X4-1.Hrono_SM_PictureFrame_3X4-1"));
		if (FrameMesh.Succeeded())
		{
			ItemMesh->SetStaticMesh(FrameMesh.Object);
		}
	}

	LeftEyeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftEyeMesh"));
	LeftEyeMesh->SetupAttachment(ItemMesh);
	LeftEyeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftEyeMesh->SetGenerateOverlapEvents(false);
	LeftEyeMesh->SetCastShadow(false);
	LeftEyeMesh->SetRelativeLocation(FVector(2.0, -12.0, 8.0));
	LeftEyeMesh->SetRelativeRotation(FRotator(0.0, 90.0, 0.0));

	RightEyeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightEyeMesh"));
	RightEyeMesh->SetupAttachment(ItemMesh);
	RightEyeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightEyeMesh->SetGenerateOverlapEvents(false);
	RightEyeMesh->SetCastShadow(false);
	RightEyeMesh->SetRelativeLocation(FVector(2.0, 12.0, 8.0));
	RightEyeMesh->SetRelativeRotation(FRotator(0.0, 90.0, 0.0));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> EyeMesh(
		TEXT("/Game/Fab/low_poly_eye/low_poly_eye/StaticMeshes/low_poly_eye.low_poly_eye"));
	if (EyeMesh.Succeeded())
	{
		LeftEyeMesh->SetStaticMesh(EyeMesh.Object);
		RightEyeMesh->SetStaticMesh(EyeMesh.Object);
	}

	TentacleEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TentacleEffect"));
	TentacleEffect->SetupAttachment(ItemMesh);
	TentacleEffect->SetAutoActivate(false);
	TentacleEffect->SetAutoDestroy(false);

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> TentacleSystem(
		TEXT("/Game/_Alex/Materials/Niagra/N_Tentacle1.N_Tentacle1"));
	if (TentacleSystem.Succeeded())
	{
		TentacleEffect->SetAsset(TentacleSystem.Object);
	}
}

void APaintItem::BeginPlay()
{
	Super::BeginPlay();

	// Native component transforms may be overridden independently in a Blueprint
	// child. Cache those final authored rotations once, before Tick starts applying
	// world-space tracking rotations. Locations are never overwritten at runtime.
	LeftEyeAuthoredRotationOffset = LeftEyeMesh->GetRelativeRotation().Quaternion();
	RightEyeAuthoredRotationOffset = RightEyeMesh->GetRelativeRotation().Quaternion();
	ApplyAnomalyVisibility();
}

void APaintItem::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (PaintAnomalyType != EPaintAnomalyType::Eyes)
	{
		SetActorTickEnabled(false);
		return;
	}

	AHronoCharacter* Viewer = FindLocalViewer();
	if (!IsValid(Viewer)
		|| (ItemTimeline != EItemTimeline::Both && ItemTimeline != Viewer->GetTimeline()))
	{
		return;
	}

	const UCameraComponent* Camera = Viewer->GetFirstPersonCameraComponent();
	const FVector TargetLocation = IsValid(Camera)
		? Camera->GetComponentLocation()
		: Viewer->GetActorLocation();
	RotateEyeToward(
		LeftEyeMesh,
		LeftEyeAuthoredRotationOffset,
		TargetLocation,
		DeltaSeconds);
	RotateEyeToward(
		RightEyeMesh,
		RightEyeAuthoredRotationOffset,
		TargetLocation,
		DeltaSeconds);
}

void APaintItem::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APaintItem, PaintAnomalyType);
}

void APaintItem::SetPaintAnomalyType(EPaintAnomalyType NewType)
{
	if (!HasAuthority())
	{
		return;
	}

	if (PaintAnomalyType != NewType)
	{
		PaintAnomalyType = NewType;
		ForceNetUpdate();
	}
	ApplyAnomalyVisibility();
}

void APaintItem::OnRep_PaintAnomalyType()
{
	ApplyAnomalyVisibility();
}

void APaintItem::UpdateVisibilityForLocalPlayer(EItemTimeline ViewerTimeline)
{
	LocalViewerTimeline = ViewerTimeline;
	Super::UpdateVisibilityForLocalPlayer(ViewerTimeline);
	ApplyAnomalyVisibility();
}

void APaintItem::ApplyAnomalyVisibility()
{
	if (const AHronoCharacter* Viewer = FindLocalViewer())
	{
		LocalViewerTimeline = Viewer->GetTimeline();
	}

	const bool bVisibleInTimeline = ItemTimeline == EItemTimeline::Both
		|| LocalViewerTimeline == EItemTimeline::Both
		|| ItemTimeline == LocalViewerTimeline;
	const bool bShowEyes = bVisibleInTimeline && PaintAnomalyType == EPaintAnomalyType::Eyes;
	const bool bShowTentacles = bVisibleInTimeline
		&& PaintAnomalyType == EPaintAnomalyType::Tentacles;

	for (UStaticMeshComponent* Eye : {LeftEyeMesh.Get(), RightEyeMesh.Get()})
	{
		if (IsValid(Eye))
		{
			Eye->SetVisibility(bShowEyes, false);
			Eye->SetHiddenInGame(!bShowEyes);
		}
	}

	if (IsValid(TentacleEffect))
	{
		TentacleEffect->SetVisibility(bShowTentacles, false);
		TentacleEffect->SetHiddenInGame(!bShowTentacles);
		if (bShowTentacles)
		{
			// Activate only on the visibility transition. Activate(true) reset the
			// Niagara simulation on every refresh and replayed all spawn bursts,
			// which looked like a new VFX instance was created every frame.
			if (!bTentacleActivationIssued)
			{
				TentacleEffect->Activate(false);
				bTentacleActivationIssued = true;
			}
		}
		else if (bTentacleActivationIssued || TentacleEffect->IsActive())
		{
			TentacleEffect->DeactivateImmediate();
			bTentacleActivationIssued = false;
		}
	}

	SetActorTickEnabled(bShowEyes && GetNetMode() != NM_DedicatedServer);
}

AHronoCharacter* APaintItem::FindLocalViewer() const
{
	if (const UWorld* World = GetWorld())
	{
		if (const APlayerController* Controller = World->GetFirstPlayerController())
		{
			return Cast<AHronoCharacter>(Controller->GetPawn());
		}
	}
	return nullptr;
}

void APaintItem::RotateEyeToward(
	UStaticMeshComponent* Eye,
	const FQuat& AuthoredRotationOffset,
	const FVector& TargetLocation,
	float DeltaSeconds) const
{
	if (!IsValid(Eye))
	{
		return;
	}

	const FVector ToTarget = TargetLocation - Eye->GetComponentLocation();
	if (ToTarget.IsNearlyZero())
	{
		return;
	}

	// Look rotation assumes the mesh looks along +X. Multiplying by the relative
	// rotation authored on this component turns its actual pupil axis toward the
	// same target without losing the per-eye Blueprint adjustment.
	const FQuat DesiredRotationQuat = ToTarget.ToOrientationQuat() * AuthoredRotationOffset;
	const FRotator DesiredRotation = DesiredRotationQuat.Rotator();
	const FRotator AppliedRotation = EyeRotationInterpSpeed > 0.0f
		? FMath::RInterpTo(Eye->GetComponentRotation(), DesiredRotation, DeltaSeconds,
			EyeRotationInterpSpeed)
		: DesiredRotation;
	Eye->SetWorldRotation(AppliedRotation);
}
