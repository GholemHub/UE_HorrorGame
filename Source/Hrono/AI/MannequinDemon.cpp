#include "AI/MannequinDemon.h"

#include "AIController.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "HronoCharacter.h"
#include "HronoCollisionChannels.h"
#include "Items/Base_Item.h"
#include "Items/Drag_Item.h"
#include "NavigationPath.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "Net/UnrealNetwork.h"
#include "ScareDirector.h"

namespace
{
	bool IsPointVisibleFromView(const AHronoCharacter* Viewer, const AActor* Subject, const FVector& Point,
		const FVector& From, const FVector& ViewForward, float HalfAngle, float MaxDistance)
	{
		UWorld* World = Viewer ? Viewer->GetWorld() : nullptr;
		if (!IsValid(Viewer) || !World) return false;
		const FVector Delta = Point - From;
		const float Distance = Delta.Size();
		if (Distance < 1.0f || Distance > MaxDistance) return false;
		if (FVector::DotProduct(ViewForward, Delta / Distance)
			< FMath::Cos(FMath::DegreesToRadians(FMath::Clamp(HalfAngle, 1.0f, 89.0f)))) return false;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(MannequinObservation), false);
		Params.AddIgnoredActor(Viewer);
		if (Subject) Params.AddIgnoredActor(Subject);
		if (const ABase_Item* Held = Viewer->GetHeldItem()) Params.AddIgnoredActor(Held);
		return !World->LineTraceTestByChannel(From, Point,
			COLLISION_CHANNEL_PAWN_FUTURE, Params);
	}

	bool IsPointInView(const AHronoCharacter* Viewer, const AActor* Subject, const FVector& Point,
		float HalfAngle, float MaxDistance)
	{
		const UCameraComponent* Camera = Viewer ? Viewer->GetFirstPersonCameraComponent() : nullptr;
		return IsValid(Camera) && IsPointVisibleFromView(Viewer, Subject, Point,
			Camera->GetComponentLocation(), Viewer->GetControlRotation().Vector(), HalfAngle, MaxDistance);
	}
}

AMannequinDemon::AMannequinDemon()
{
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(true);
	SetNetUpdateFrequency(20.0f);
	SetMinNetUpdateFrequency(5.0f);
	PrimaryActorTick.bCanEverTick = false;
	AIControllerClass = AAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	GetCharacterMovement()->MaxWalkSpeed = StalkingSpeed;
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();
	MonocleVisual = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MonocleVisual"));
	MonocleVisual->SetupAttachment(GetCapsuleComponent());
	MonocleVisual->SetRelativeTransform(GetMesh()->GetRelativeTransform());
	MonocleVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MonocleVisual->SetVisibleInSceneCaptureOnly(true);
	MonocleVisual->SetVisibility(false);
}

AMannequinDemon* AMannequinDemon::SpawnAndActivateMannequin(const UObject* WorldContextObject,
	TSubclassOf<AMannequinDemon> MannequinClass)
{
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject,
		EGetWorldErrorMode::ReturnNull) : nullptr;
	if (!World || World->GetNetMode() == NM_Client || !MannequinClass) return nullptr;
	for (TActorIterator<AMannequinDemon> It(World); It; ++It)
		if (It->State != EMannequinState::Dormant && It->State != EMannequinState::Disabled) return nullptr;
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AMannequinDemon* Demon = World->SpawnActor<AMannequinDemon>(MannequinClass,
		FVector::ZeroVector, FRotator::ZeroRotator, Params);
	if (!Demon) return nullptr;
	Demon->bFactorySpawnPending = true;
	if (Demon->ActivateMannequin()) return Demon;
	Demon->Destroy();
	return nullptr;
}

void AMannequinDemon::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority()) MannequinTimeline = EItemTimeline::Future;
	// Blueprint defaults are applied after the native constructor. Keep the
	// capture-only body aligned with the authored physical body in placed BPs.
	MonocleVisual->SetRelativeTransform(GetMesh()->GetRelativeTransform());
	ApplyPhysicalState();
	if (HasAuthority())
	{
		Director = AScareDirector::GetHuntDirector(this);
		if (Director.IsValid())
		{
			Director->OnHuntStateChanged.AddDynamic(this, &AMannequinDemon::HandleHuntStateChanged);
		}
		GetWorldTimerManager().SetTimer(EvaluationTimer, this, &AMannequinDemon::Evaluate,
			FMath::Max(0.05f, ObservationCheckInterval), true);
		if (Director.IsValid() && IsBabaiDangerous(Director->GetHuntState())) EnterBabaiSubmissive();
	}
	GetWorldTimerManager().SetTimer(LocalPresentationTimer, this, &AMannequinDemon::RefreshLocalPresentation, 0.2f, true);
	RefreshLocalPresentation();
	OnStateSnapshotApplied(State);
}

void AMannequinDemon::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Director.IsValid()) Director->OnHuntStateChanged.RemoveDynamic(this, &AMannequinDemon::HandleHuntStateChanged);
	GetWorldTimerManager().ClearAllTimersForObject(this);
	Super::EndPlay(EndPlayReason);
}

void AMannequinDemon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMannequinDemon, State);
	DOREPLIFETIME(AMannequinDemon, CurrentTarget);
	DOREPLIFETIME(AMannequinDemon, CurrentPartner);
	DOREPLIFETIME(AMannequinDemon, MannequinTimeline);
	DOREPLIFETIME(AMannequinDemon, Observer);
	DOREPLIFETIME(AMannequinDemon, FutureSight);
	DOREPLIFETIME(AMannequinDemon, PastMonocleSight);
	DOREPLIFETIME(AMannequinDemon, FutureSightBlocker);
	DOREPLIFETIME(AMannequinDemon, PastSightBlocker);
	DOREPLIFETIME(AMannequinDemon, BlockingDoor);
	DOREPLIFETIME(AMannequinDemon, WarningStartedAt);
	DOREPLIFETIME(AMannequinDemon, WarningEndsAt);
}

void AMannequinDemon::OnRep_State()
{
	ApplyPhysicalState();
	RefreshLocalPresentation();
	OnStateSnapshotApplied(State);
}

void AMannequinDemon::ApplyPhysicalState()
{
	if (!bMeshCollisionCaptured)
	{
		AuthoredMeshCollision = GetMesh()->GetCollisionEnabled();
		bMeshCollisionCaptured = true;
	}
	const bool bActive = State != EMannequinState::Dormant && State != EMannequinState::Disabled;
	GetCapsuleComponent()->SetCollisionEnabled(bActive
		? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(bActive ? AuthoredMeshCollision : ECollisionEnabled::NoCollision);
	if (bActive)
	{
		GetCapsuleComponent()->SetCollisionObjectType(COLLISION_CHANNEL_PAWN_FUTURE);
		GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, ECR_Block);
		GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, ECR_Ignore);
	}
	if (!bActive || (State != EMannequinState::Spawned && State != EMannequinState::Stalking
		&& State != EMannequinState::Approaching && State != EMannequinState::ApproachingDoor))
	{
		StopMotion();
	}
	else if (GetCharacterMovement()->MovementMode == MOVE_None)
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
}

void AMannequinDemon::OnRep_Targets()
{
	ApplyPhysicalState();
	RefreshLocalPresentation();
	OnTargetChanged(CurrentTarget, CurrentPartner);
}

void AMannequinDemon::RefreshLocalPresentation()
{
	const AHronoCharacter* Viewer = nullptr;
	if (const UWorld* World = GetWorld())
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			const APlayerController* PC = It->Get();
			if (PC && PC->IsLocalController())
			{
				Viewer = Cast<AHronoCharacter>(PC->GetPawn());
				if (Viewer) break;
			}
		}
		if (!Viewer && World->GetNetMode() == NM_Standalone)
		{
			const APlayerController* PC = World->GetFirstPlayerController();
			Viewer = PC ? Cast<AHronoCharacter>(PC->GetPawn()) : nullptr;
		}
	}
	const bool bPhysical = Viewer && Viewer->GetTimeline() == EItemTimeline::Future;
	const ABase_Item* Held = Viewer ? Viewer->GetHeldItem() : nullptr;
	const bool bThroughMonocle = Viewer && Viewer->GetTimeline() == EItemTimeline::Past && IsValid(Held)
		&& Held->bCanRepelMannequin && Held->OwningCharacter == Viewer && Held->bIsPickedUp
		&& Held->FindComponentByClass<USceneCaptureComponent2D>();
	SetActorHiddenInGame(false);
	GetMesh()->SetHiddenInGame(false);
	GetMesh()->SetVisibleInSceneCaptureOnly(false);
	GetMesh()->SetVisibility(bPhysical);
	MonocleVisual->SetHiddenInGame(false);
	MonocleVisual->SetVisibleInSceneCaptureOnly(true);
	MonocleVisual->SetVisibility(bThroughMonocle);
	if (bLocalDebugOverlay) DrawLocalDebug(Viewer);
	if (State == EMannequinState::GrabWarning) OnGrabWarningProgress(GetGrabWarningProgress());
}

float AMannequinDemon::GetGrabWarningProgress() const
{
	if (State != EMannequinState::GrabWarning || WarningEndsAt <= WarningStartedAt) return 0.0f;
	const AGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState() : nullptr;
	const float Now = GS ? GS->GetServerWorldTimeSeconds() : (GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f);
	return FMath::Clamp((Now - WarningStartedAt) / (WarningEndsAt - WarningStartedAt), 0.0f, 1.0f);
}

bool AMannequinDemon::IsEligibleTarget(const AHronoCharacter* Player) const
{
	return IsValid(Player) && !Player->IsActorBeingDestroyed() && !Player->IsSafeInHidingWardrobe()
		&& !Player->IsDeathTimelineTransitionPending()
		&& Player->GetTimeline() == EItemTimeline::Future
		&& IsValid(Player->GetController());
}

bool AMannequinDemon::SelectTarget(AHronoCharacter* Preferred)
{
	if (!HasAuthority()) return false;
	AHronoCharacter* Chosen = IsEligibleTarget(Preferred) ? Preferred : nullptr;
	if (!Chosen)
	{
		AHronoCharacter* OldTarget = CurrentTarget;
		AHronoCharacter* Fallback = nullptr;
		float Best = TNumericLimits<float>::Max();
		float BestFallback = TNumericLimits<float>::Max();
		if (const AGameStateBase* GS = GetWorld()->GetGameState())
		{
			for (APlayerState* PS : GS->PlayerArray)
			{
				AHronoCharacter* Candidate = PS ? Cast<AHronoCharacter>(PS->GetPawn()) : nullptr;
				if (!IsEligibleTarget(Candidate)) continue;
				const float Distance = FVector::DistSquared(GetActorLocation(), Candidate->GetActorLocation());
				if (Distance < BestFallback) { Fallback = Candidate; BestFallback = Distance; }
				if (Candidate != OldTarget && Distance < Best) { Chosen = Candidate; Best = Distance; }
			}
		}
		if (!Fallback)
		{
			for (TActorIterator<AHronoCharacter> It(GetWorld()); It; ++It)
			{
				if (IsEligibleTarget(*It)) { Fallback = *It; break; }
			}
		}
		if (!Chosen) Chosen = Fallback;
	}
	if (!Chosen) return false;
	const bool bChanged = Chosen != CurrentTarget;
	const AHronoCharacter* PreviousPartner = CurrentPartner;
	CurrentTarget = Chosen;
	ApplyPhysicalState();
	CurrentPartner = nullptr;
	if (const AGameStateBase* GS = GetWorld()->GetGameState())
	{
		for (APlayerState* PS : GS->PlayerArray)
		{
			AHronoCharacter* Candidate = PS ? Cast<AHronoCharacter>(PS->GetPawn()) : nullptr;
			if (Candidate && Candidate != Chosen && Candidate->GetTimeline() == EItemTimeline::Past
				&& IsValid(Candidate->GetController())) { CurrentPartner = Candidate; break; }
		}
	}
	if (!CurrentPartner)
	{
		for (TActorIterator<AHronoCharacter> It(GetWorld()); It; ++It)
		{
			if (*It != Chosen && It->GetTimeline() == EItemTimeline::Past && IsValid(It->GetController()))
			{ CurrentPartner = *It; break; }
		}
	}
	if (bChanged)
	{
		BlockingDoor = nullptr;
		UpdateObservationSnapshot(EMannequinSight::NotEvaluated,
			EMannequinSight::NotEvaluated, NAME_None, NAME_None);
		NextTargetSwitchAt = GetWorld()->GetTimeSeconds() + TargetSwitchCooldown;
	}
	if (bChanged || CurrentPartner != PreviousPartner)
	{
		OnTargetChanged(CurrentTarget, CurrentPartner);
		ForceNetUpdate();
	}
	return true;
}

bool AMannequinDemon::ForceTarget(AHronoCharacter* NewTarget)
{
	if (!HasAuthority() || !IsEligibleTarget(NewTarget)) return false;
	if (!SelectTarget(NewTarget)) return false;
	if (State != EMannequinState::Dormant && State != EMannequinState::Disabled)
	{
		StopMotion();
		CompletedStalkSteps = 0;
		SetState(EMannequinState::Spawned);
	}
	return true;
}

bool AMannequinDemon::ActivateMannequin()
{
	const auto FailActivation = [this](const TCHAR* Reason)
	{
		LastActivationFailure = Reason;
		UE_LOG(LogTemp, Warning, TEXT("[Mannequin] %s activation rejected: %s"),
			*GetName(), *LastActivationFailure);
		return false;
	};
	if (!HasAuthority()) return FailActivation(TEXT("Call ActivateMannequin on the server"));
	if (State != EMannequinState::Dormant && State != EMannequinState::Disabled)
		return FailActivation(TEXT("This Mannequin is already active"));
	for (TActorIterator<AMannequinDemon> It(GetWorld()); It; ++It)
		if (*It != this && It->State != EMannequinState::Dormant && It->State != EMannequinState::Disabled)
			return FailActivation(TEXT("Another Mannequin is already active"));
	if (!SelectTarget()) return FailActivation(TEXT("No eligible player target with a controller"));
	UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!Nav || !Nav->GetDefaultNavDataInstance())
		return FailActivation(TEXT("No runtime NavMesh data; build navigation and check NavMeshBoundsVolume"));
	FVector SpawnLocation;
	if (bFactorySpawnPending)
	{
		if (!FindValidLocation(CurrentTarget->GetActorLocation(), MinSpawnDistance, MaxSpawnDistance,
			true, SpawnLocation, true))
			return FailActivation(TEXT("No hidden, unblocked and reachable NavMesh point in the spawn distance range"));
	}
	else
	{
		FNavLocation Projected;
		const float Radius = GetCapsuleComponent()->GetScaledCapsuleRadius();
		const float HalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		if (!Nav->ProjectPointToNavigation(GetActorLocation(), Projected,
			FVector(FMath::Max(100.0f, Radius * 2.0f), FMath::Max(100.0f, Radius * 2.0f), HalfHeight + 100.0f))
			|| FVector::Dist2D(GetActorLocation(), Projected.Location) > FMath::Max(100.0f, Radius * 2.0f))
			return FailActivation(TEXT("Placed Mannequin is outside the reachable NavMesh; move it onto the floor"));
		SpawnLocation = Projected.Location + FVector(0, 0, HalfHeight + 2.0f);
		if (GetWorld()->OverlapBlockingTestByChannel(SpawnLocation, FQuat::Identity, ECC_Pawn,
			FCollisionShape::MakeCapsule(Radius, HalfHeight), FCollisionQueryParams(NAME_None, false, this)))
			return FailActivation(TEXT("Placed Mannequin capsule is blocked at its NavMesh position"));
		UNavigationPath* Path = Nav->FindPathToLocationSynchronously(GetWorld(),
			Projected.Location, CurrentTarget->GetActorLocation(), CurrentTarget);
		if (!Path || !Path->IsValid() || Path->IsPartial())
		{
			FHitResult DoorHit;
			if (!FindClosedDoorBetweenTarget(DoorHit))
				return FailActivation(TEXT("Placed Mannequin has no NavMesh path or closed door to the target"));
		}
	}
	SetActorLocation(SpawnLocation, false, nullptr, ETeleportType::TeleportPhysics);
	bFactorySpawnPending = false;
	ActivatedAt = GetWorld()->GetTimeSeconds();
	CompletedStalkSteps = 0;
	LastActivationFailure.Reset();
	SetState(EMannequinState::Spawned);
	if (Director.IsValid() && IsBabaiDangerous(Director->GetHuntState())) EnterBabaiSubmissive();
	return true;
}

void AMannequinDemon::DeactivateMannequin()
{
	if (!HasAuthority()) return;
	if (State == EMannequinState::GrabWarning || State == EMannequinState::Grab)
		MulticastSpecialEvent(1);
	StopMotion();
	GetWorldTimerManager().ClearTimer(StateTimer);
	GetWorldTimerManager().ClearTimer(ContainmentWarningTimer);
	GetWorldTimerManager().ClearTimer(BabaiTimer);
	CurrentTarget = nullptr;
	CurrentPartner = nullptr;
	BlockingDoor = nullptr;
	UpdateObservationSnapshot(EMannequinSight::NotEvaluated, EMannequinSight::NotEvaluated,
		NAME_None, NAME_None);
	SetState(EMannequinState::Dormant);
	ForceNetUpdate();
}

bool AMannequinDemon::WakeMannequin()
{
	if (!HasAuthority()) return false;
	if (State == EMannequinState::Dormant || State == EMannequinState::Disabled) return ActivateMannequin();
	if (State == EMannequinState::Contained || State == EMannequinState::SubmissiveToBabai) return false;
	GetWorldTimerManager().ClearTimer(StateTimer);
	SetState(EMannequinState::Spawned);
	return true;
}

EMannequinSight AMannequinDemon::EvaluateSight(const AHronoCharacter* Player,
	bool bRequireMonocle, FName& OutBlocker) const
{
	OutBlocker = NAME_None;
	if (!IsValid(Player)) return EMannequinSight::NoViewer;
	if (bRequireMonocle && (Player != CurrentPartner || Player->GetTimeline() != EItemTimeline::Past))
		return EMannequinSight::WrongTimeline;
	if (!bRequireMonocle && (Player != CurrentTarget || Player->GetTimeline() != EItemTimeline::Future))
		return EMannequinSight::WrongTimeline;
	const UCameraComponent* Camera = Player->GetFirstPersonCameraComponent();
	const USceneCaptureComponent2D* Capture = nullptr;
	if (bRequireMonocle)
	{
		const ABase_Item* Held = Player->GetHeldItem();
		if (!IsValid(Held) || !Held->bCanRepelMannequin || Held->OwningCharacter != Player
			|| !Held->bIsPickedUp) return EMannequinSight::NoMonocle;
		Capture = Held->FindComponentByClass<USceneCaptureComponent2D>();
		if (!IsValid(Capture)) return EMannequinSight::NoCapture;
	}
	else if (!IsValid(Camera)) return EMannequinSight::NoCapture;
	const FVector From = bRequireMonocle ? Capture->GetComponentLocation() : Camera->GetComponentLocation();
	const FVector Forward = bRequireMonocle ? Capture->GetForwardVector() : Player->GetControlRotation().Vector();
	const float Aspect = bRequireMonocle && IsValid(Capture->TextureTarget) && Capture->TextureTarget->SizeY > 0
		? static_cast<float>(Capture->TextureTarget->SizeX) / Capture->TextureTarget->SizeY : 1.0f;
	const float TanHorizontal = bRequireMonocle
		? FMath::Tan(FMath::DegreesToRadians(FMath::Clamp(Capture->FOVAngle, 1.0f, 170.0f) * 0.5f)) : 0.0f;
	const FTransform CaptureTransform = bRequireMonocle ? Capture->GetComponentTransform() : FTransform::Identity;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(MannequinObservation), false);
	Params.AddIgnoredActor(Player);
	Params.AddIgnoredActor(this);
	if (const ABase_Item* Held = Player->GetHeldItem()) Params.AddIgnoredActor(Held);
	bool bInRange = false;
	bool bInView = false;
	for (const float Height : { -30.0f, 20.0f, VisibilitySampleHeight })
	{
		const FVector Point = GetActorLocation() + FVector(0, 0, Height);
		const FVector Delta = Point - From;
		const float Distance = Delta.Size();
		if (Distance < 1.0f || Distance > ObservationDistance) continue;
		bInRange = true;
		if (bRequireMonocle)
		{
			const FVector Local = CaptureTransform.InverseTransformVectorNoScale(Delta);
			if (Local.X <= 0.0f || FMath::RadiansToDegrees(FMath::Atan2(
				FMath::Sqrt(FMath::Square(Local.Y) + FMath::Square(Local.Z)), Local.X)) > MonocleHalfAngle)
				continue;
			const float ScreenX = Local.Y / (Local.X * TanHorizontal);
			const float ScreenY = Local.Z * Aspect / (Local.X * TanHorizontal);
			if (FMath::Square(ScreenX) + FMath::Square(ScreenY)
				> FMath::Square(MonocleApertureRadius)) continue;
		}
		else if (FVector::DotProduct(Forward, Delta / Distance)
			< FMath::Cos(FMath::DegreesToRadians(ObservationAngle))) continue;
		bInView = true;
		FHitResult Hit;
		if (!GetWorld()->LineTraceSingleByChannel(Hit, From, Point,
			COLLISION_CHANNEL_PAWN_FUTURE, Params))
			return EMannequinSight::Visible;
		if (OutBlocker.IsNone())
		{
			const UObject* Obstacle = Hit.GetActor()
				? static_cast<const UObject*>(Hit.GetActor()) : Hit.GetComponent();
			OutBlocker = FName(*GetNameSafe(Obstacle));
		}
	}
	return !bInRange ? EMannequinSight::OutOfRange
		: !bInView ? EMannequinSight::OutsideView : EMannequinSight::Occluded;
}

bool AMannequinDemon::IsObserved(const AHronoCharacter* Player, bool bRequireMonocle) const
{
	if (State == EMannequinState::Dormant || State == EMannequinState::Disabled) return false;
	FName Blocker;
	return EvaluateSight(Player, bRequireMonocle, Blocker) == EMannequinSight::Visible;
}

bool AMannequinDemon::IsMannequinObservedByPlayer(const AHronoCharacter* Player) const
{
	return IsObserved(Player, false);
}

bool AMannequinDemon::IsPlayerObservingMannequinThroughMonocle(const AHronoCharacter* Player) const
{
	return IsObserved(Player, true);
}

void AMannequinDemon::UpdateObservationSnapshot(EMannequinSight NewFuture,
	EMannequinSight NewPast, FName NewFutureBlocker, FName NewPastBlocker)
{
	const bool bFuture = NewFuture == EMannequinSight::Visible;
	const bool bPast = NewPast == EMannequinSight::Visible;
	const EMannequinObserver NewObserver = bFuture && bPast ? EMannequinObserver::Both
		: bFuture ? EMannequinObserver::FutureDirect
		: bPast ? EMannequinObserver::PastMonocle : EMannequinObserver::None;
	if (Observer == NewObserver && FutureSight == NewFuture && PastMonocleSight == NewPast
		&& FutureSightBlocker == NewFutureBlocker && PastSightBlocker == NewPastBlocker) return;
	Observer = NewObserver;
	FutureSight = NewFuture;
	PastMonocleSight = NewPast;
	FutureSightBlocker = NewFutureBlocker;
	PastSightBlocker = NewPastBlocker;
	ForceNetUpdate();
	if (bDebugEnabled)
		UE_LOG(LogTemp, Log, TEXT("[MannequinSight] %s observer=%s future=%s block=%s pastLens=%s block=%s"),
			*GetName(), *StaticEnum<EMannequinObserver>()->GetNameStringByValue(static_cast<int64>(Observer)),
			*StaticEnum<EMannequinSight>()->GetNameStringByValue(static_cast<int64>(FutureSight)),
			*FutureSightBlocker.ToString(),
			*StaticEnum<EMannequinSight>()->GetNameStringByValue(static_cast<int64>(PastMonocleSight)),
			*PastSightBlocker.ToString());
}

void AMannequinDemon::ToggleLocalDebugOverlay()
{
	bLocalDebugOverlay = !bLocalDebugOverlay;
	if (!bLocalDebugOverlay && GEngine)
		GEngine->AddOnScreenDebugMessage(static_cast<uint64>(GetUniqueID()), 0.01f,
			FColor::White, TEXT(""));
}

void AMannequinDemon::DrawLocalDebug(const AHronoCharacter* Viewer)
{
	if (!GEngine || !IsValid(Viewer)) return;
	FName LocalBlocker;
	const bool bLens = Viewer->GetTimeline() == EItemTimeline::Past;
	const EMannequinSight LocalSight = EvaluateSight(Viewer, bLens, LocalBlocker);
	const UCameraComponent* Camera = Viewer->GetFirstPersonCameraComponent();
	const ABase_Item* Held = Viewer->GetHeldItem();
	const USceneCaptureComponent2D* Capture = IsValid(Held)
		? Held->FindComponentByClass<USceneCaptureComponent2D>() : nullptr;
	const FVector CameraFrom = IsValid(Camera) ? Camera->GetComponentLocation()
		: Viewer->GetActorLocation();
	const FVector CameraDirection = Viewer->GetControlRotation().Vector();
	const FVector From = bLens && IsValid(Capture) ? Capture->GetComponentLocation()
		: CameraFrom;
	const FVector Direction = bLens && IsValid(Capture) ? Capture->GetForwardVector()
		: CameraDirection;
	const FVector ToMannequin = (GetActorLocation() + FVector(0, 0, 20.0f) - From).GetSafeNormal();
	const float AimError = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(
		FVector::DotProduct(Direction, ToMannequin), -1.0f, 1.0f)));
	float LensRadius = -1.0f;
	if (bLens && IsValid(Capture) && IsValid(Capture->TextureTarget)
		&& Capture->TextureTarget->SizeY > 0)
	{
		const FVector Local = Capture->GetComponentTransform().InverseTransformVectorNoScale(
			GetActorLocation() + FVector(0, 0, 20.0f) - From);
		if (Local.X > 0.0f)
		{
			const float TanHalfFov = FMath::Tan(FMath::DegreesToRadians(
				FMath::Clamp(Capture->FOVAngle, 1.0f, 170.0f) * 0.5f));
			const float Aspect = static_cast<float>(Capture->TextureTarget->SizeX)
				/ Capture->TextureTarget->SizeY;
			LensRadius = FVector2D(Local.Y / (Local.X * TanHalfFov),
				Local.Z * Aspect / (Local.X * TanHalfFov)).Size();
		}
	}
	const FColor RayColor = LocalSight == EMannequinSight::Visible ? FColor::Green
		: LocalSight == EMannequinSight::Occluded ? FColor::Red : FColor::Yellow;
	DrawDebugDirectionalArrow(GetWorld(), From, From + Direction * 400.0f, 25.0f,
		RayColor, false, 0.25f, 0, 2.0f);
	const FString Message = FString::Printf(
		TEXT("MANNEQUIN [L] %s | observer=%s | target=%s | door=%s\n")
		TEXT("SERVER: Future camera=%s blocker=%s | Past lens=%s blocker=%s\n")
		TEXT("LOCAL: %s %s blocker=%s | aim error=%.1f deg distance=%.0f cm\n")
		TEXT("Camera: origin=%s forward=%s\n")
		TEXT("Capture: origin=%s forward=%s | FOV=%.1f circle=%.2f/%.2f"),
		*StaticEnum<EMannequinState>()->GetNameStringByValue(static_cast<int64>(State)),
		*StaticEnum<EMannequinObserver>()->GetNameStringByValue(static_cast<int64>(Observer)),
		*GetNameSafe(CurrentTarget), *GetNameSafe(BlockingDoor),
		*StaticEnum<EMannequinSight>()->GetNameStringByValue(static_cast<int64>(FutureSight)),
		*FutureSightBlocker.ToString(),
		*StaticEnum<EMannequinSight>()->GetNameStringByValue(static_cast<int64>(PastMonocleSight)),
		*PastSightBlocker.ToString(),
		bLens ? TEXT("Past lens") : TEXT("Future camera"),
		*StaticEnum<EMannequinSight>()->GetNameStringByValue(static_cast<int64>(LocalSight)),
		*LocalBlocker.ToString(), AimError, FVector::Dist(From, GetActorLocation()),
		*CameraFrom.ToCompactString(), *CameraDirection.ToCompactString(),
		IsValid(Capture) ? *Capture->GetComponentLocation().ToCompactString() : TEXT("none"),
		IsValid(Capture) ? *Capture->GetForwardVector().ToCompactString() : TEXT("none"),
		IsValid(Capture) ? Capture->FOVAngle : 0.0f, LensRadius, MonocleApertureRadius);
	GEngine->AddOnScreenDebugMessage(static_cast<uint64>(GetUniqueID()), 0.25f,
		LocalSight == EMannequinSight::Visible ? FColor::Green : FColor::Yellow, Message);
}

bool AMannequinDemon::FindValidLocation(const FVector& Center, float MinDistance, float MaxDistance,
	bool bRequireHidden, FVector& OutLocation, bool bLogFailure) const
{
	if (!IsValid(CurrentTarget) || MaxDistance < MinDistance) return false;
	UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!Nav) return false;
	const FVector Facing = CurrentTarget->GetControlRotation().Vector().GetSafeNormal2D();
	int32 NoSample = 0;
	int32 WrongDistance = 0;
	int32 WrongAngle = 0;
	int32 Blocked = 0;
	int32 Visible = 0;
	int32 NoPath = 0;
	for (int32 Attempt = 0; Attempt < 72; ++Attempt)
	{
		FNavLocation NavPoint;
		if (!Nav->GetRandomReachablePointInRadius(Center, MaxDistance, NavPoint))
		{ ++NoSample; continue; }
		// Navigation returns floor height. An ACharacter location is the centre of its
		// capsule; testing the floor point as a capsule centre always overlaps the floor.
		const FVector Candidate = NavPoint.Location + FVector(0, 0,
			GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 2.0f);
		const float Distance = FVector::Dist2D(Candidate, Center);
		if (Distance < MinDistance || Distance > MaxDistance)
		{ ++WrongDistance; continue; }
		const FVector Direction = (Candidate - Center).GetSafeNormal2D();
		// Prefer the rear cone, then accept any truly unseen candidate in a cramped room.
		if (bRequireHidden && Attempt < 36 && FVector::DotProduct(Facing, Direction)
			> FMath::Cos(FMath::DegreesToRadians(PreferredBehindAngle)))
		{ ++WrongAngle; continue; }
		if (GetWorld()->OverlapBlockingTestByChannel(Candidate, FQuat::Identity, ECC_Pawn,
			FCollisionShape::MakeCapsule(GetCapsuleComponent()->GetScaledCapsuleRadius(),
				GetCapsuleComponent()->GetScaledCapsuleHalfHeight()), FCollisionQueryParams(NAME_None, false, this)))
		{ ++Blocked; continue; }
		if (bRequireHidden && IsPointInView(CurrentTarget, this,
			Candidate + FVector(0, 0, VisibilitySampleHeight), ObservationAngle, ObservationDistance))
		{ ++Visible; continue; }
		UNavigationPath* Path = Nav->FindPathToLocationSynchronously(GetWorld(), Center, Candidate, CurrentTarget);
		if (!Path || !Path->IsValid() || Path->IsPartial())
		{ ++NoPath; continue; }
		OutLocation = Candidate;
		return true;
	}
	if (bLogFailure)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Mannequin] %s spawn search failed: nav=%d distance=%d rear=%d blocked=%d visible=%d path=%d; range=[%.0f,%.0f] target=%s"),
			*GetName(), NoSample, WrongDistance, WrongAngle, Blocked, Visible, NoPath,
			MinDistance, MaxDistance, *GetNameSafe(CurrentTarget));
	}
	return false;
}

float AMannequinDemon::GetCloseApproachCenterDistance() const
{
	const UCapsuleComponent* TargetCapsule = IsValid(CurrentTarget)
		? CurrentTarget->GetCapsuleComponent() : nullptr;
	return GetCapsuleComponent()->GetScaledCapsuleRadius()
		+ (TargetCapsule ? TargetCapsule->GetScaledCapsuleRadius() : 0.0f)
		+ FMath::Max(0.0f, ApproachClearance);
}

bool AMannequinDemon::FindBehindLocation(FVector& OutLocation) const
{
	if (!IsValid(CurrentTarget)) return false;
	UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!Nav) return false;
	const FVector Desired = CurrentTarget->GetActorLocation()
		- CurrentTarget->GetControlRotation().Vector().GetSafeNormal2D()
			* GetCloseApproachCenterDistance();
	FNavLocation Projected;
	if (!Nav->ProjectPointToNavigation(Desired, Projected, FVector(65, 65, 120))) return false;
	if (FVector::Dist2D(Desired, Projected.Location) > 70.0f) return false;
	const FVector Candidate = Projected.Location + FVector(0, 0,
		GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 2.0f);
	if (GetWorld()->OverlapBlockingTestByChannel(Candidate, FQuat::Identity, ECC_Pawn,
		FCollisionShape::MakeCapsule(GetCapsuleComponent()->GetScaledCapsuleRadius(),
			GetCapsuleComponent()->GetScaledCapsuleHalfHeight()), FCollisionQueryParams(NAME_None, false, this))) return false;
	if (IsPointInView(CurrentTarget, this, Candidate + FVector(0, 0, VisibilitySampleHeight),
		ObservationAngle, ObservationDistance)) return false;
	UNavigationPath* Path = Nav->FindPathToLocationSynchronously(GetWorld(), GetActorLocation(), Candidate, CurrentTarget);
	if (!Path || !Path->IsValid() || Path->IsPartial()
		|| Path->GetPathLength() > BehindPlayerDistance * 3.0f) return false;
	OutLocation = Candidate;
	return true;
}

ADrag_Item* AMannequinDemon::FindClosedDoorBetweenTarget(FHitResult& OutHit) const
{
	if (!IsValid(CurrentTarget)) return nullptr;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(MannequinDoor), false, this);
	Params.AddIgnoredActor(CurrentTarget);
	for (const float Height : { -30.0f, 20.0f, VisibilitySampleHeight })
	{
		const FVector From = GetActorLocation() + FVector(0, 0, Height);
		const FVector To = CurrentTarget->GetActorLocation() + FVector(0, 0, Height);
		FHitResult Hit;
		if (!GetWorld()->LineTraceSingleByChannel(Hit, From, To,
			COLLISION_CHANNEL_PAWN_FUTURE, Params)) continue;
		ADrag_Item* Door = Cast<ADrag_Item>(Hit.GetActor());
		if (IsValid(Door) && Door->bIsClosed
			&& (Door->ItemTimeline == EItemTimeline::Future || Door->ItemTimeline == EItemTimeline::Both))
		{
			OutHit = Hit;
			return Door;
		}
	}
	return nullptr;
}

bool AMannequinDemon::FindDoorApproachLocation(const FHitResult& DoorHit, FVector& OutLocation) const
{
	UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!Nav || !IsValid(CurrentTarget)) return false;
	const FVector TowardDoor = (DoorHit.ImpactPoint - GetActorLocation()).GetSafeNormal2D();
	if (TowardDoor.IsNearlyZero()) return false;
	const float CapsuleOffset = GetCapsuleComponent()->GetScaledCapsuleRadius()
		+ FMath::Max(ApproachClearance, 10.0f);
	for (const float Extra : { 90.0f, 60.0f, 30.0f })
	{
		FNavLocation Projected;
		const FVector NearSide = DoorHit.ImpactPoint - TowardDoor * (CapsuleOffset + Extra);
		if (!Nav->ProjectPointToNavigation(NearSide, Projected, FVector(65, 65, 120))) continue;
		if (FVector::DotProduct((Projected.Location - DoorHit.ImpactPoint).GetSafeNormal2D(), TowardDoor) >= 0.0f)
			continue;
		UNavigationPath* Path = Nav->FindPathToLocationSynchronously(GetWorld(),
			GetActorLocation(), Projected.Location, CurrentTarget);
		if (!Path || !Path->IsValid() || Path->IsPartial()) continue;
		OutLocation = Projected.Location + FVector(0, 0,
			GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 2.0f);
		return true;
	}
	// A closed panel can split the NavMesh. Its partial path still tells us the
	// closest reachable point on our side; never cross to the player's side.
	UNavigationPath* Partial = Nav->FindPathToLocationSynchronously(GetWorld(),
		GetActorLocation(), DoorHit.ImpactPoint, CurrentTarget);
	if (Partial && Partial->IsValid() && Partial->PathPoints.Num() > 1)
	{
		const FVector End = Partial->PathPoints.Last();
		if (FVector::Dist2D(End, DoorHit.ImpactPoint) <= 250.0f
			&& FVector::Dist2D(End, DoorHit.ImpactPoint)
				< FVector::Dist2D(GetActorLocation(), DoorHit.ImpactPoint))
		{
			OutLocation = End + FVector(0, 0,
				GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 2.0f);
			return true;
		}
	}
	return false;
}

bool AMannequinDemon::StartMove(const FVector& Destination, EMannequinState NewState)
{
	AAIController* AI = Cast<AAIController>(GetController());
	if (!AI) return false;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetCharacterMovement()->MaxWalkSpeed = NewState == EMannequinState::Approaching ? MoveSpeed : StalkingSpeed;
	// UE's actor reach test includes both capsule radii and 1.1x our radius.
	// Offset its extra 0.1x so ApproachClearance remains the surface gap.
	const EPathFollowingRequestResult::Type Result = NewState == EMannequinState::Stalking
		? AI->MoveToActor(CurrentTarget,
			FMath::Max(0.0f, ApproachClearance
				- 0.1f * GetCapsuleComponent()->GetScaledCapsuleRadius()),
			true, true, false, nullptr, false)
		: AI->MoveToLocation(Destination,
			NewState == EMannequinState::ApproachingDoor ? 30.0f : 2.0f,
			false, true, true, false, nullptr, false);
	if (Result == EPathFollowingRequestResult::Failed) return false;
	MoveDestination = Destination;
	bHasMoveDestination = true;
	SetState(NewState);
	return true;
}

void AMannequinDemon::StopMotion()
{
	if (AAIController* AI = Cast<AAIController>(GetController())) AI->StopMovement();
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	bHasMoveDestination = false;
}

void AMannequinDemon::SetState(EMannequinState NewState, bool bEmitMoment)
{
	if (!HasAuthority() || State == NewState) return;
	State = NewState;
	ApplyPhysicalState();
	OnStateSnapshotApplied(State);
	RefreshLocalPresentation();
	if (bEmitMoment && NewState != EMannequinState::Repelled && NewState != EMannequinState::Dormant
		&& NewState != EMannequinState::Disabled) MulticastMoment(NewState, nullptr);
	ForceNetUpdate();
	if (bDebugEnabled) UE_LOG(LogTemp, Log, TEXT("[Mannequin] %s state=%d target=%s partner=%s"),
		*GetName(), static_cast<int32>(State), *GetNameSafe(CurrentTarget), *GetNameSafe(CurrentPartner));
}

void AMannequinDemon::MulticastMoment_Implementation(EMannequinState Moment, AHronoCharacter* CapturedPlayer)
{
	// The server already ran these events in its multicast call; OnRep handles snapshots separately.
	switch (Moment)
	{
	case EMannequinState::Spawned: OnSpawned(); break;
	case EMannequinState::Observing: OnFrozenByTarget(); break;
	case EMannequinState::Stalking: OnStalkingStarted(); break;
	case EMannequinState::Approaching: OnApproachingStarted(); break;
	case EMannequinState::BehindPlayer: OnBehindPlayer(); OnBreathingStarted(); break;
	case EMannequinState::GrabWarning: OnGrabWarningStarted(); break;
	case EMannequinState::Grab: if (CapturedPlayer) OnPlayerCaptured(CapturedPlayer); else OnGrabStarted(); break;
	case EMannequinState::Repelled: OnFrozenByMonocle(); OnRepelled(); OnRepelledByMonocle(); break;
	case EMannequinState::Contained: OnContained(); break;
	case EMannequinState::SubmissiveToBabai: OnEnterSubmissiveToBabai(); break;
	default: break;
	}
}

void AMannequinDemon::MulticastSpecialEvent_Implementation(uint8 EventCode)
{
	switch (EventCode)
	{
	case 1: OnGrabCancelled(); break;
	case 2: OnRescueSucceeded(); break;
	case 3: OnExitSubmissiveToBabai(); break;
	case 4: OnContainmentWarning(); break;
	case 5: OnContainmentExpired(); break;
	case 6: OnFrozenByMonocle(); break;
	default: break;
	}
}

void AMannequinDemon::Evaluate()
{
	if (!HasAuthority()) return;
	const float Now = GetWorld()->GetTimeSeconds();
	if (bDebugEnabled)
	{
		DrawDebugString(GetWorld(), GetActorLocation() + FVector(0, 0, 120),
			FString::Printf(TEXT("Mannequin %d / target %s / seen %d / monocle %d / steps %d / nav %d"),
				static_cast<int32>(State), *GetNameSafe(CurrentTarget), bTargetObserved,
				bPartnerObservingThroughMonocle, CompletedStalkSteps, bHasMoveDestination),
			nullptr, FColor::Cyan, ObservationCheckInterval + 0.1f);
		if (bHasMoveDestination) DrawDebugSphere(GetWorld(), MoveDestination, 24.0f, 12,
			FColor::Yellow, false, ObservationCheckInterval + 0.1f);
	}
	if (State == EMannequinState::Dormant || State == EMannequinState::Disabled
		|| State == EMannequinState::Contained || State == EMannequinState::SubmissiveToBabai)
	{
		UpdateObservationSnapshot(EMannequinSight::NotEvaluated,
			EMannequinSight::NotEvaluated, NAME_None, NAME_None);
		return;
	}
	if (!IsEligibleTarget(CurrentTarget) || CurrentTarget->GetTimeline() != MannequinTimeline)
	{
		if (State == EMannequinState::GrabWarning || State == EMannequinState::Grab)
			MulticastSpecialEvent(1);
		if (SelectTarget())
		{
			StopMotion();
			CompletedStalkSteps = 0;
			SetState(EMannequinState::Spawned);
		}
		else DeactivateMannequin();
		return;
	}
	if ((!IsValid(CurrentPartner) || CurrentPartner == CurrentTarget || !IsValid(CurrentPartner->GetController()))
		&& Now >= NextPartnerCheckAt)
	{
		SelectTarget(CurrentTarget);
		NextPartnerCheckAt = Now + 1.0f;
	}
	FName FutureBlocker;
	FName PastBlocker;
	const EMannequinSight NewFuture = EvaluateSight(CurrentTarget, false, FutureBlocker);
	const EMannequinSight NewPast = EvaluateSight(CurrentPartner, true, PastBlocker);
	UpdateObservationSnapshot(NewFuture, NewPast, FutureBlocker, PastBlocker);
	bTargetObserved = NewFuture == EMannequinSight::Visible;
	bPartnerObservingThroughMonocle = NewPast == EMannequinSight::Visible;
	if (bTargetObserved || bPartnerObservingThroughMonocle) LastObservedAt = Now;
	// A door closed during the warning or rescue window must cancel the grab;
	// otherwise the pending timer could capture a player through the panel.
	if (State == EMannequinState::GrabWarning || State == EMannequinState::Grab)
	{
		FHitResult DoorHit;
		if (FindClosedDoorBetweenTarget(DoorHit))
		{
			MulticastSpecialEvent(1);
			StopMotion();
			WarningStartedAt = 0.0f;
			WarningEndsAt = 0.0f;
			SetState(EMannequinState::Spawned, false);
		}
	}
	if (bPartnerObservingThroughMonocle &&
		(State == EMannequinState::BehindPlayer || State == EMannequinState::GrabWarning
			|| State == EMannequinState::Grab))
	{
		Repel(true);
		return;
	}
	if (State == EMannequinState::GrabWarning)
	{
		if (bTargetObserved && bSeeingMannequinDuringGrabWarningTriggersFinalGrab) BeginFinalGrabWindow();
		else if (Now >= WarningEndsAt) BeginFinalGrabWindow();
		return;
	}
	if (State == EMannequinState::Grab)
	{
		if (Now >= WarningEndsAt) CompleteGrab();
		return;
	}
	if (State == EMannequinState::Repelled) return;
	if (bTargetObserved || bPartnerObservingThroughMonocle
		|| Now - LastObservedAt < ObservationGraceTime)
	{
		if (State != EMannequinState::Observing)
		{
			StopMotion();
			SetState(EMannequinState::Observing, !bPartnerObservingThroughMonocle);
			if (bPartnerObservingThroughMonocle) MulticastSpecialEvent(6);
		}
		return;
	}
	if (State == EMannequinState::Observing)
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		SetState(EMannequinState::Spawned, false);
	}
	FHitResult DoorHit;
	ADrag_Item* Door = FindClosedDoorBetweenTarget(DoorHit);
	if (Door)
	{
		if (State == EMannequinState::WaitingAtDoor && BlockingDoor == Door) return;
		if (State == EMannequinState::ApproachingDoor && BlockingDoor == Door)
		{
			if (const AAIController* AI = Cast<AAIController>(GetController());
				AI && AI->GetMoveStatus() == EPathFollowingStatus::Moving) return;
			StopMotion();
			SetState(EMannequinState::WaitingAtDoor, false);
			return;
		}
		StopMotion();
		BlockingDoor = Door;
		FVector DoorApproach;
		if (FindDoorApproachLocation(DoorHit, DoorApproach)
			&& StartMove(DoorApproach, EMannequinState::ApproachingDoor)) return;
		SetState(EMannequinState::WaitingAtDoor, false);
		return;
	}
	if (BlockingDoor || State == EMannequinState::WaitingAtDoor
		|| State == EMannequinState::ApproachingDoor)
	{
		BlockingDoor = nullptr;
		StopMotion();
		SetState(EMannequinState::Spawned, false);
	}
	if (State == EMannequinState::Approaching || State == EMannequinState::Stalking)
	{
		const bool bStalking = State == EMannequinState::Stalking;
		const float GoalDistance = bStalking && IsValid(CurrentTarget)
			? FVector::Dist2D(GetActorLocation(), CurrentTarget->GetActorLocation())
			: FVector::Dist2D(GetActorLocation(), MoveDestination);
		const bool bReached = bHasMoveDestination && GoalDistance <=
			(bStalking ? GetCloseApproachCenterDistance() + 10.0f : 12.0f);
		const AAIController* AI = Cast<AAIController>(GetController());
		// Let path following finish its 10 cm surface-clearance request before
		// considering the step complete; a looser distance check stopped it early.
		if (bHasMoveDestination && AI && AI->GetMoveStatus() == EPathFollowingStatus::Moving)
			return;
		StopMotion();
		if (bReached && !bStalking)
		{
			EnterWarning();
			return;
		}
		if (bReached && bStalking)
			++CompletedStalkSteps;
		SetState(EMannequinState::Spawned, false);
		return;
	}
	if (Now - LastApproachAt < ApproachCooldown) return;
	if (CompletedStalkSteps >= RequiredStalkStepsBeforeGrab && Now - ActivatedAt >= MinTimeBeforeGrabAttempt)
	{
		FVector Behind;
		if (FindBehindLocation(Behind) && StartMove(Behind, EMannequinState::Approaching))
		{
			LastApproachAt = Now;
			return;
		}
	}
	StartMove(CurrentTarget->GetActorLocation(), EMannequinState::Stalking);
	LastApproachAt = Now;
}

void AMannequinDemon::EnterWarning()
{
	if (!HasAuthority() || !IsValid(CurrentTarget)) return;
	const FVector TargetToDemon = (GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal2D();
	if (FVector::Dist2D(GetActorLocation(), CurrentTarget->GetActorLocation())
		> GetCloseApproachCenterDistance() + 20.0f
		|| FVector::DotProduct(CurrentTarget->GetControlRotation().Vector().GetSafeNormal2D(), TargetToDemon)
			> FMath::Cos(FMath::DegreesToRadians(PreferredBehindAngle)))
	{
		SetState(EMannequinState::Spawned, false);
		return;
	}
	SetState(EMannequinState::BehindPlayer);
	const float Now = GetWorld()->GetTimeSeconds();
	WarningStartedAt = Now;
	WarningEndsAt = Now + GrabWarningDuration;
	SetState(EMannequinState::GrabWarning);
}

void AMannequinDemon::BeginFinalGrabWindow()
{
	if (!HasAuthority() || State != EMannequinState::GrabWarning) return;
	WarningStartedAt = GetWorld()->GetTimeSeconds();
	WarningEndsAt = WarningStartedAt + RescueWindowDuration;
	SetState(EMannequinState::Grab);
}

void AMannequinDemon::Repel(bool bRescue)
{
	if (!HasAuthority() || State == EMannequinState::Repelled) return;
	StopMotion();
	BlockingDoor = nullptr;
	GetWorldTimerManager().ClearTimer(StateTimer);
	SetState(EMannequinState::Repelled);
	MulticastMoment(EMannequinState::Repelled, nullptr);
	if (bRescue) { MulticastSpecialEvent(1); MulticastSpecialEvent(2); }
	StartCooldown(FMath::Max(RepelledDelay, bRescue ? PostGrabCooldown : 0.0f));
}

void AMannequinDemon::CompleteGrab()
{
	if (!HasAuthority() || State != EMannequinState::Grab) return;
	AHronoCharacter* Victim = CurrentTarget;
	if (IsEligibleTarget(Victim) && Victim->GetTimeline() == MannequinTimeline)
	{
		MulticastMoment(EMannequinState::Grab, Victim);
		ApplyCapturePunishment(Victim);
	}
	StopMotion();
	BlockingDoor = nullptr;
	SetState(EMannequinState::Repelled);
	StartCooldown(PostGrabCooldown);
}

bool AMannequinDemon::ApplyCapturePunishment_Implementation(AHronoCharacter* CapturedPlayer)
{
	if (!HasAuthority() || !IsValid(CapturedPlayer) || !bTransferCapturedPlayerToOppositeTimeline) return false;
	const EItemTimeline Opposite = CapturedPlayer->GetTimeline() == EItemTimeline::Past
		? EItemTimeline::Future : EItemTimeline::Past;
	return CapturedPlayer->TrySetPlayerTimelineOnAuthority(Opposite);
}

void AMannequinDemon::StartCooldown(float Duration)
{
	GetWorldTimerManager().ClearTimer(StateTimer);
	GetWorldTimerManager().SetTimer(StateTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		if (!HasAuthority() || State != EMannequinState::Repelled) return;
		if (!SelectTarget(IsEligibleTarget(CurrentTarget) && GetWorld()->GetTimeSeconds() < NextTargetSwitchAt
			? CurrentTarget.Get() : nullptr)) { DeactivateMannequin(); return; }
		CompletedStalkSteps = 0;
		ActivatedAt = GetWorld()->GetTimeSeconds();
		SetState(EMannequinState::Spawned);
	}), FMath::Max(0.1f, Duration), false);
}

bool AMannequinDemon::TryContainMannequin(AActor* ContainmentItem)
{
	if (!HasAuthority() || !IsValid(ContainmentItem) || !ContainmentItem->ActorHasTag(ContainmentItemTag)
		|| State == EMannequinState::Dormant || State == EMannequinState::Disabled
		|| State == EMannequinState::Contained || State == EMannequinState::SubmissiveToBabai
		|| FVector::Dist(GetActorLocation(), ContainmentItem->GetActorLocation()) > ContainmentInteractionDistance)
		return false;
	if (const ABase_Item* Item = Cast<ABase_Item>(ContainmentItem))
		if (Item->OwningCharacter || Item->bIsPickedUp) return false;
	if (State == EMannequinState::GrabWarning || State == EMannequinState::Grab)
		MulticastSpecialEvent(1);
	StopMotion();
	GetWorldTimerManager().ClearTimer(StateTimer);
	ContainmentEndsAt = GetWorld()->GetTimeSeconds() + ContainmentDuration;
	SetState(EMannequinState::Contained);
	GetWorldTimerManager().SetTimer(ContainmentWarningTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		if (State == EMannequinState::Contained) MulticastSpecialEvent(4);
	}), FMath::Max(0.1f, ContainmentDuration - ContainmentWarningLeadTime), false);
	GetWorldTimerManager().SetTimer(StateTimer, this, &AMannequinDemon::EndContainment, ContainmentDuration, false);
	return true;
}

void AMannequinDemon::EndContainment()
{
	if (!HasAuthority() || State != EMannequinState::Contained) return;
	MulticastSpecialEvent(5);
	SetState(EMannequinState::Spawned);
	ActivatedAt = GetWorld()->GetTimeSeconds();
	CompletedStalkSteps = 0;
}

bool AMannequinDemon::IsBabaiDangerous(EGhostHuntState HuntState) const
{
	return HuntState == EGhostHuntState::Manifestation || HuntState == EGhostHuntState::Searching
		|| HuntState == EGhostHuntState::Chasing || HuntState == EGhostHuntState::Ending;
}

void AMannequinDemon::HandleHuntStateChanged(EGhostHuntState OldState, EGhostHuntState NewState)
{
	if (!HasAuthority()) return;
	if (IsBabaiDangerous(NewState)) EnterBabaiSubmissive();
	else if (State == EMannequinState::SubmissiveToBabai)
	{
		GetWorldTimerManager().ClearTimer(BabaiTimer);
		GetWorldTimerManager().SetTimer(BabaiTimer, this, &AMannequinDemon::ResumeAfterBabai,
			FMath::Max(0.1f, BabaiRecoveryDelay), false);
	}
}

void AMannequinDemon::EnterBabaiSubmissive()
{
	if (!HasAuthority() || State == EMannequinState::Dormant || State == EMannequinState::Disabled
		|| State == EMannequinState::SubmissiveToBabai) return;
	bContainmentWasInterruptedByBabai = State == EMannequinState::Contained;
	if (State == EMannequinState::GrabWarning || State == EMannequinState::Grab)
		MulticastSpecialEvent(1);
	StopMotion();
	GetWorldTimerManager().ClearTimer(StateTimer);
	GetWorldTimerManager().ClearTimer(ContainmentWarningTimer);
	GetWorldTimerManager().ClearTimer(BabaiTimer);
	SetState(EMannequinState::SubmissiveToBabai);
}

void AMannequinDemon::ResumeAfterBabai()
{
	if (!HasAuthority() || State != EMannequinState::SubmissiveToBabai) return;
	if (Director.IsValid() && IsBabaiDangerous(Director->GetHuntState())) return;
	MulticastSpecialEvent(3);
	if (bContainmentWasInterruptedByBabai && GetWorld()->GetTimeSeconds() < ContainmentEndsAt)
	{
		const float Remaining = ContainmentEndsAt - GetWorld()->GetTimeSeconds();
		SetState(EMannequinState::Contained, false);
		if (Remaining > ContainmentWarningLeadTime)
		{
			GetWorldTimerManager().SetTimer(ContainmentWarningTimer,
				FTimerDelegate::CreateWeakLambda(this, [this]()
				{
					if (State == EMannequinState::Contained) MulticastSpecialEvent(4);
				}), Remaining - ContainmentWarningLeadTime, false);
		}
		else MulticastSpecialEvent(4);
		GetWorldTimerManager().SetTimer(StateTimer, this, &AMannequinDemon::EndContainment, Remaining, false);
	}
	else
	{
		SetState(EMannequinState::Repelled);
		StartCooldown(PostGrabCooldown);
	}
	bContainmentWasInterruptedByBabai = false;
}
