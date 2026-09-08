#include "Components/MirrorOptimizationSubsystem.h"

#include "Camera/PlayerCameraManager.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"

namespace MirrorOptimization
{
	static TAutoConsoleVariable<int32> CVarEnabled(
		TEXT("hrono.Mirror.Culling"),
		1,
		TEXT("Enable distance, viewport and occlusion-based Blueprint Tick culling for BP_Mirror."));

	static TAutoConsoleVariable<float> CVarAlwaysActiveDistance(
		TEXT("hrono.Mirror.AlwaysActiveDistance"),
		300.0f,
		TEXT("Distance in cm at which BP_Mirror keeps ticking even outside the viewport."));

	static TAutoConsoleVariable<float> CVarMaxVisibleDistance(
		TEXT("hrono.Mirror.MaxVisibleDistance"),
		2000.0f,
		TEXT("Maximum distance in cm at which a visible BP_Mirror may tick."));

	static TAutoConsoleVariable<float> CVarCheckInterval(
		TEXT("hrono.Mirror.CheckInterval"),
		0.10f,
		TEXT("Seconds between BP_Mirror visibility checks."));

	static TAutoConsoleVariable<float> CVarVisibilityGracePeriod(
		TEXT("hrono.Mirror.VisibilityGracePeriod"),
		0.25f,
		TEXT("Seconds to keep BP_Mirror ticking after it leaves the visible region."));

	static TAutoConsoleVariable<int32> CVarCheckLineOfSight(
		TEXT("hrono.Mirror.CheckLineOfSight"),
		1,
		TEXT("Use a visibility trace to suspend BP_Mirror when level geometry occludes it."));

	static bool IsBPProjectMirror(const AActor* Actor)
	{
		if (!IsValid(Actor))
		{
			return false;
		}

		// Target only /Game/_Alex/BP_Mirror. Other decorative mirror assets and
		// similarly named gameplay actors are intentionally left untouched.
		const UClass* ActorClass = Actor->GetClass();
		return IsValid(ActorClass)
			&& (ActorClass->GetName().Equals(TEXT("BP_Mirror_C"), ESearchCase::CaseSensitive)
				|| ActorClass->GetPathName().Equals(
					TEXT("/Game/_Alex/BP_Mirror.BP_Mirror_C"),
					ESearchCase::CaseSensitive));
	}
}

bool UMirrorOptimizationSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UMirrorOptimizationSubsystem::Deinitialize()
{
	RestoreOriginalTickStates();
	MirrorActors.Empty();
	Super::Deinitialize();
}

void UMirrorOptimizationSubsystem::Tick(float DeltaTime)
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	ScanCountdown -= DeltaTime;
	if (ScanCountdown <= 0.0f)
	{
		RefreshMirrorActors();
		ScanCountdown = 2.0f;
	}

	VisibilityCheckCountdown -= DeltaTime;
	if (VisibilityCheckCountdown > 0.0f)
	{
		return;
	}
	VisibilityCheckCountdown = FMath::Max(
		MirrorOptimization::CVarCheckInterval.GetValueOnGameThread(),
		0.05f);

	const bool bCullingEnabled = MirrorOptimization::CVarEnabled.GetValueOnGameThread() != 0;
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);
	APlayerCameraManager* CameraManager = IsValid(PlayerController) && PlayerController->IsLocalController()
		? PlayerController->PlayerCameraManager
		: nullptr;

	for (auto Iterator = MirrorActors.CreateIterator(); Iterator; ++Iterator)
	{
		AActor* MirrorActor = Iterator.Key().Get();
		if (!IsValid(MirrorActor))
		{
			Iterator.RemoveCurrent();
			continue;
		}

		const bool bShouldTick = Iterator.Value().bOriginalTickEnabled
			&& (!bCullingEnabled
				|| (IsValid(CameraManager)
					&& ShouldTickMirror(MirrorActor, Iterator.Value(), PlayerController, CameraManager)));
		MirrorActor->SetActorTickEnabled(bShouldTick);
	}
}

TStatId UMirrorOptimizationSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UMirrorOptimizationSubsystem, STATGROUP_Tickables);
}

void UMirrorOptimizationSubsystem::RefreshMirrorActors()
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
	{
		AActor* Actor = *ActorIterator;
		const TWeakObjectPtr<AActor> ActorKey(Actor);
		if (!MirrorOptimization::IsBPProjectMirror(Actor) || MirrorActors.Contains(ActorKey))
		{
			continue;
		}

		FMirrorActorState& State = MirrorActors.Add(ActorKey);
		State.bOriginalTickEnabled = Actor->IsActorTickEnabled();
	}
}

bool UMirrorOptimizationSubsystem::ShouldTickMirror(
	AActor* MirrorActor,
	FMirrorActorState& MirrorState,
	APlayerController* PlayerController,
	APlayerCameraManager* CameraManager) const
{
	const UWorld* World = GetWorld();
	if (!IsValid(MirrorActor) || !IsValid(World) || MirrorActor->IsHidden())
	{
		return false;
	}

	const FBox MirrorBounds = MirrorActor->GetComponentsBoundingBox(true);
	const FVector MirrorLocation = MirrorBounds.IsValid
		? MirrorBounds.GetCenter()
		: MirrorActor->GetActorLocation();
	const FVector CameraLocation = CameraManager->GetCameraLocation();
	const FVector CameraToMirror = MirrorLocation - CameraLocation;
	const float DistanceSquared = CameraToMirror.SizeSquared();
	const float AlwaysActiveDistance = FMath::Max(
		MirrorOptimization::CVarAlwaysActiveDistance.GetValueOnGameThread(),
		0.0f);

	if (DistanceSquared <= FMath::Square(AlwaysActiveDistance))
	{
		MirrorState.LastVisibleTime = World->GetTimeSeconds();
		return true;
	}

	const float MaxVisibleDistance = FMath::Max(
		MirrorOptimization::CVarMaxVisibleDistance.GetValueOnGameThread(),
		AlwaysActiveDistance);
	if (DistanceSquared > FMath::Square(MaxVisibleDistance))
	{
		return false;
	}

	const FVector DirectionToMirror = CameraToMirror.GetSafeNormal();
	bool bPotentiallyVisible = !DirectionToMirror.IsNearlyZero()
		&& FVector::DotProduct(CameraManager->GetCameraRotation().Vector(), DirectionToMirror) > 0.0f;

	FVector2D ScreenPosition = FVector2D::ZeroVector;
	bPotentiallyVisible = bPotentiallyVisible
		&& PlayerController->ProjectWorldLocationToScreen(MirrorLocation, ScreenPosition, true);

	int32 ViewportWidth = 0;
	int32 ViewportHeight = 0;
	PlayerController->GetViewportSize(ViewportWidth, ViewportHeight);
	constexpr float ScreenMargin = 0.15f;
	bPotentiallyVisible = bPotentiallyVisible
		&& ViewportWidth > 0
		&& ViewportHeight > 0
		&& ScreenPosition.X >= -ViewportWidth * ScreenMargin
		&& ScreenPosition.X <= ViewportWidth * (1.0f + ScreenMargin)
		&& ScreenPosition.Y >= -ViewportHeight * ScreenMargin
		&& ScreenPosition.Y <= ViewportHeight * (1.0f + ScreenMargin);

	if (bPotentiallyVisible
		&& MirrorOptimization::CVarCheckLineOfSight.GetValueOnGameThread() != 0)
	{
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BPMirrorVisibility), false, MirrorActor);
		const FVector TraceEnd = MirrorLocation - DirectionToMirror * 5.0f;
		bPotentiallyVisible = !World->LineTraceTestByChannel(
			CameraLocation,
			TraceEnd,
			ECC_Visibility,
			QueryParams);
	}

	if (bPotentiallyVisible)
	{
		MirrorState.LastVisibleTime = World->GetTimeSeconds();
		return true;
	}

	const float GracePeriod = FMath::Max(
		MirrorOptimization::CVarVisibilityGracePeriod.GetValueOnGameThread(),
		0.0f);
	return World->GetTimeSeconds() - MirrorState.LastVisibleTime <= GracePeriod;
}

void UMirrorOptimizationSubsystem::RestoreOriginalTickStates()
{
	for (const TPair<TWeakObjectPtr<AActor>, FMirrorActorState>& Pair : MirrorActors)
	{
		if (AActor* MirrorActor = Pair.Key.Get(); IsValid(MirrorActor))
		{
			MirrorActor->SetActorTickEnabled(Pair.Value.bOriginalTickEnabled);
		}
	}
}
