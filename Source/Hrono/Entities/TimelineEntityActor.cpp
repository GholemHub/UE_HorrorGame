#include "Entities/TimelineEntityActor.h"

#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HronoCharacter.h"
#include "Net/UnrealNetwork.h"
#include "ScareDirector.h"

DEFINE_LOG_CATEGORY_STATIC(LogTimelineEntity, Log, All);

ATimelineEntityActor::ATimelineEntityActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(false);
	SetActorEnableCollision(false);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(SceneRoot);
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StaticMesh->SetGenerateOverlapEvents(false);
	StaticMesh->SetSimulatePhysics(false);
	StaticMesh->CanCharacterStepUpOn = ECB_No;

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(SceneRoot);
	SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SkeletalMesh->SetGenerateOverlapEvents(false);
	SkeletalMesh->SetSimulatePhysics(false);
	SkeletalMesh->CanCharacterStepUpOn = ECB_No;

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(SceneRoot);
	AudioComponent->bAutoActivate = false;
	AudioComponent->bStopWhenOwnerDestroyed = true;
}

void ATimelineEntityActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	EnforceCollisionlessComponents();
	RefreshVisualRepresentation();
}

void ATimelineEntityActor::BeginPlay()
{
	Super::BeginPlay();
	EnforceCollisionlessComponents();
	ValidateSelectedVisualAsset();

	if (AScareDirector* Director = AScareDirector::GetHuntDirector(this))
	{
		Director->RegisterTimelineEntity(this);
	}

	RefreshVisibilityForLocalPlayer();
}

#if WITH_EDITOR
void ATimelineEntityActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	RefreshVisualRepresentation();
}
#endif

void ATimelineEntityActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AScareDirector* Director = AScareDirector::GetHuntDirector(this))
	{
		Director->UnregisterTimelineEntity(this);
	}

	Super::EndPlay(EndPlayReason);
}

void ATimelineEntityActor::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATimelineEntityActor, VisualType);
	DOREPLIFETIME(ATimelineEntityActor, EntityTimeline);
}

void ATimelineEntityActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AHronoCharacter* Character = GetLocalCharacter();
	TryRespawnAfterCooldown(Character, DeltaSeconds);

	const bool bCanBePerceived = IsValid(Character)
		&& IsAvailableForCharacter(*Character)
		&& (!bIsManagedByDirector || bDirectorAllowsVisibility);
	ApplyLocalVisibility(bCanBePerceived);

	if (!bCanBePerceived)
	{
		bWasTooClose = false;
		ResetGazeState();
		return;
	}

	if (bFaceLocalPlayer)
	{
		FaceCharacter(*Character, DeltaSeconds);
	}

	const float Distance = FVector::Distance(Character->GetActorLocation(), GetActorLocation());
	const bool bTooClose = TooCloseDistance > 0.0f && Distance <= TooCloseDistance;
	if (bTooClose && !bWasTooClose)
	{
		OnPlayerTooClose(Character, Distance);

		if (bAutomaticallyDisappearWhenTooClose && !bHasLocallyDisappeared)
		{
			DisappearEntity(Character, ETimelineEntityDisappearReason::PlayerTooClose);
		}
	}
	bWasTooClose = bTooClose;

	if (bHasLocallyDisappeared)
	{
		ResetGazeState();
		return;
	}

	if (!IsCharacterLookingAtEntity(*Character))
	{
		ResetGazeState();
		return;
	}

	CurrentGazeDuration += DeltaSeconds;

	if (!bShortGazeEventSent
		&& GazeDisappearDuration > 0.0f
		&& CurrentGazeDuration >= GazeDisappearDuration)
	{
		bShortGazeEventSent = true;
		OnWatchedForDisappearDuration(Character);

		if (bAutomaticallyDisappearAfterGaze && !bHasLocallyDisappeared)
		{
			DisappearEntity(Character, ETimelineEntityDisappearReason::WatchedTooLong);
		}
	}

	if (!bHasLocallyDisappeared
		&& !bLongGazeEventSent
		&& LongGazeDuration > 0.0f
		&& CurrentGazeDuration >= LongGazeDuration)
	{
		bLongGazeEventSent = true;
		OnWatchedForLongDuration(Character);
	}
}

void ATimelineEntityActor::DisappearEntity(
	AHronoCharacter* TriggeringCharacter,
	ETimelineEntityDisappearReason Reason)
{
	if (bHasLocallyDisappeared)
	{
		return;
	}

	bHasLocallyDisappeared = true;
	CurrentRespawnCooldown = 0.0f;
	ApplyLocalVisibility(false);
	ResetGazeState();

	if (IsValid(AudioComponent))
	{
		AudioComponent->Stop();
		bAudioPausedForTimeline = false;
	}

	OnEntityDisappeared(TriggeringCharacter, Reason);
}

void ATimelineEntityActor::ResetEntity()
{
	bHasLocallyDisappeared = false;
	CurrentRespawnCooldown = 0.0f;
	bWasTooClose = false;
	ResetGazeState();
	RefreshVisibilityForLocalPlayer();
}

void ATimelineEntityActor::RefreshVisibilityForLocalPlayer()
{
	AHronoCharacter* Character = GetLocalCharacter();
	const bool bShouldBeVisible = IsValid(Character)
		&& IsCharacterInEntityTimeline(*Character)
		&& !bHasLocallyDisappeared;
	ApplyLocalVisibility(bShouldBeVisible);
}

void ATimelineEntityActor::RefreshVisualRepresentation()
{
	if (!HasActorBegunPlay())
	{
		// Editor preview must never inherit a runtime Hidden In Game state.
		SetActorHiddenInGame(false);
		ApplyVisualComponentState(true);
		return;
	}

	RefreshVisibilityForLocalPlayer();
}

void ATimelineEntityActor::SetDirectorVisibility(bool bVisible)
{
	bIsManagedByDirector = true;
	bDirectorAllowsVisibility = bVisible;
	RefreshVisibilityForLocalPlayer();
}

void ATimelineEntityActor::ClearDirectorVisibilityControl()
{
	bIsManagedByDirector = false;
	bDirectorAllowsVisibility = true;
	RefreshVisibilityForLocalPlayer();
}

bool ATimelineEntityActor::IsAvailableForCharacter(AHronoCharacter& Character) const
{
	return !bHasLocallyDisappeared && IsCharacterInEntityTimeline(Character);
}

void ATimelineEntityActor::OnRep_VisualConfiguration()
{
	RefreshVisualRepresentation();
}

AHronoCharacter* ATimelineEntityActor::GetLocalCharacter() const
{
	UWorld* World = GetWorld();
	APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
	return PlayerController ? Cast<AHronoCharacter>(PlayerController->GetPawn()) : nullptr;
}

bool ATimelineEntityActor::IsCharacterInEntityTimeline(AHronoCharacter& Character) const
{
	return EntityTimeline == EItemTimeline::Both
		|| Character.GetTimeline() == EItemTimeline::Both
		|| EntityTimeline == Character.GetTimeline();
}

bool ATimelineEntityActor::IsCharacterLookingAtEntity(AHronoCharacter& Character) const
{
	const UCameraComponent* Camera = Character.GetFirstPersonCameraComponent();
	if (!IsValid(Camera))
	{
		return false;
	}

	const FVector CameraLocation = Camera->GetComponentLocation();
	const FVector TargetLocation = GetActorTransform().TransformPosition(GazeTargetOffset);
	const FVector ToTarget = TargetLocation - CameraLocation;
	const float TargetDistance = ToTarget.Size();
	if (TargetDistance <= UE_KINDA_SMALL_NUMBER)
	{
		return true;
	}

	const float MinimumDot = FMath::Cos(FMath::DegreesToRadians(
		FMath::Clamp(GazeHalfAngleDegrees, 0.0f, 90.0f)));
	if (FVector::DotProduct(Camera->GetForwardVector(), ToTarget / TargetDistance) < MinimumDot)
	{
		return false;
	}

	if (!bRequireUnobstructedView)
	{
		return true;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(TimelineEntityGaze), false);
	QueryParams.AddIgnoredActor(&Character);
	QueryParams.AddIgnoredActor(this);

	FHitResult Hit;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		CameraLocation,
		TargetLocation,
		ECC_Visibility,
		QueryParams);
	return !bHit;
}

void ATimelineEntityActor::TryRespawnAfterCooldown(
	AHronoCharacter* Character,
	float DeltaSeconds)
{
	if (!bHasLocallyDisappeared)
	{
		return;
	}

	CurrentRespawnCooldown += DeltaSeconds;
	if (CurrentRespawnCooldown < FMath::Max(0.0f, RespawnCooldown) || !IsValid(Character))
	{
		return;
	}

	const float Distance = FVector::Distance(Character->GetActorLocation(), GetActorLocation());
	if (Distance < FMath::Max(0.0f, MinimumRespawnDistance))
	{
		return;
	}

	bHasLocallyDisappeared = false;
	CurrentRespawnCooldown = 0.0f;
	bWasTooClose = false;
	ResetGazeState();
}

void ATimelineEntityActor::FaceCharacter(AHronoCharacter& Character, float DeltaSeconds)
{
	FVector ToCharacter = Character.GetActorLocation() - GetActorLocation();
	if (bUseYawOnly)
	{
		ToCharacter.Z = 0.0f;
	}

	if (ToCharacter.IsNearlyZero())
	{
		return;
	}

	FRotator TargetRotation = ToCharacter.Rotation() + FacingRotationOffset;
	if (bUseYawOnly)
	{
		TargetRotation.Pitch = FacingRotationOffset.Pitch;
		TargetRotation.Roll = FacingRotationOffset.Roll;
	}

	const FRotator NewRotation = FacingRotationInterpSpeed > 0.0f
		? FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaSeconds, FacingRotationInterpSpeed)
		: TargetRotation;
	SetActorRotation(NewRotation);
}

void ATimelineEntityActor::EnforceCollisionlessComponents()
{
	// Blueprint subclasses may add more primitive components. The entity contract
	// requires every one of them to remain non-colliding.
	TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents(this);
	for (UPrimitiveComponent* Primitive : PrimitiveComponents)
	{
		if (IsValid(Primitive))
		{
			Primitive->SetSimulatePhysics(false);
			Primitive->SetGenerateOverlapEvents(false);
			Primitive->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Primitive->CanCharacterStepUpOn = ECB_No;
		}
	}

	SetActorEnableCollision(false);
}

void ATimelineEntityActor::ApplyVisualComponentState(bool bActorVisible)
{
	const bool bUseStaticMesh = VisualType == ETimelineEntityVisualType::StaticMesh;
	const bool bUseSkeletalMesh = VisualType == ETimelineEntityVisualType::SkeletalMesh;

	if (IsValid(StaticMesh))
	{
		StaticMesh->SetVisibility(bUseStaticMesh, true);
		StaticMesh->SetHiddenInGame(!bUseStaticMesh, true);
		StaticMesh->MarkRenderStateDirty();
	}

	if (IsValid(SkeletalMesh))
	{
		SkeletalMesh->SetVisibility(bUseSkeletalMesh, true);
		SkeletalMesh->SetHiddenInGame(!bUseSkeletalMesh, true);
		SkeletalMesh->SetComponentTickEnabled(bUseSkeletalMesh && bActorVisible);
		SkeletalMesh->MarkRenderStateDirty();
	}
}

void ATimelineEntityActor::ValidateSelectedVisualAsset() const
{
	if (VisualType == ETimelineEntityVisualType::SkeletalMesh
		&& (!IsValid(SkeletalMesh) || !IsValid(SkeletalMesh->GetSkeletalMeshAsset())))
	{
		UE_LOG(LogTimelineEntity, Warning,
			TEXT("[%s] Visual Type is Skeletal Mesh, but the SkeletalMesh component has no asset."),
			*GetName());
	}
	else if (VisualType == ETimelineEntityVisualType::StaticMesh
		&& (!IsValid(StaticMesh) || !IsValid(StaticMesh->GetStaticMesh())))
	{
		UE_LOG(LogTimelineEntity, Warning,
			TEXT("[%s] Visual Type is Static Mesh, but the StaticMesh component has no asset."),
			*GetName());
	}
}

void ATimelineEntityActor::ApplyLocalVisibility(bool bVisible)
{
	const bool bFinalVisibility = bVisible
		&& (!bIsManagedByDirector || bDirectorAllowsVisibility);
	bIsLocallyVisible = bFinalVisibility;
	ApplyVisualComponentState(bFinalVisibility);

	// Director selection and timeline filtering are presentation-only. They must
	// not call DisappearEntity or emit OnEntityDisappeared.
	SetActorHiddenInGame(!bFinalVisibility);

	if (!IsValid(AudioComponent) || bHasLocallyDisappeared)
	{
		return;
	}

	if (!bFinalVisibility && AudioComponent->IsPlaying())
	{
		AudioComponent->SetPaused(true);
		bAudioPausedForTimeline = true;
	}
	else if (bFinalVisibility && bAudioPausedForTimeline)
	{
		AudioComponent->SetPaused(false);
		bAudioPausedForTimeline = false;
	}
}

void ATimelineEntityActor::ResetGazeState()
{
	CurrentGazeDuration = 0.0f;
	bShortGazeEventSent = false;
	bLongGazeEventSent = false;
}
