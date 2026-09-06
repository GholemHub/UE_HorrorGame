// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/HronoLoadingSubsystem.h"

#include "Containers/Ticker.h"
#include "ContentStreaming.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/StreamableManager.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Misc/PackageName.h"
#include "MoviePlayer.h"
#include "Styling/CoreStyle.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/Images/SThrobber.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "WorldPartition/WorldPartitionSubsystem.h"

#include "Hrono.h"

namespace HronoLoading
{
	TSharedRef<SWidget> MakeMovieLoadingScreen()
	{
		return SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
			.BorderBackgroundColor(FLinearColor::Black)
			.Padding(0.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(20.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("DIVIDED")))
						.Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 52))
						.ColorAndOpacity(FLinearColor(0.72f, 0.07f, 0.04f, 1.0f))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(10.0f)
					[
						SNew(SThrobber)
						.NumPieces(3)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(10.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("LOADING")))
						.Font(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 18))
						.ColorAndOpacity(FLinearColor(0.82f, 0.82f, 0.84f, 1.0f))
					]
				]
			];
	}
}

void UHronoLoadingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
		this, &ThisClass::HandlePostLoadMap);
	bTickerEnabled = true;
	FTSTicker::GetCoreTicker().AddTicker(
		TEXT("Hrono loading flow"), 0.0f,
		[WeakThis = TWeakObjectPtr<UHronoLoadingSubsystem>(this)](float DeltaSeconds)
		{
			return WeakThis.IsValid() ? WeakThis->TickLoading(DeltaSeconds) : false;
		});
	if (GEngine)
	{
		TravelFailureHandle = GEngine->OnTravelFailure().AddWeakLambda(
			this,
			[this](UWorld*, ETravelFailure::Type FailureType, const FString& Error)
			{
				if (!bArmNextMapWarmup && !bWorldWarmupActive && !IsPreloading())
				{
					return;
				}
				UE_LOG(LogHrono, Error,
					TEXT("Loading flow cancelled by travel failure %d: %s"),
					static_cast<int32>(FailureType), *Error);
				CancelLoadingFlow();
			});
		NetworkFailureHandle = GEngine->OnNetworkFailure().AddWeakLambda(
			this,
			[this](UWorld*, UNetDriver*, ENetworkFailure::Type FailureType, const FString& Error)
			{
				if (!bArmNextMapWarmup && !bWorldWarmupActive && !IsPreloading())
				{
					return;
				}
				UE_LOG(LogHrono, Error,
					TEXT("Loading flow cancelled by network failure %d: %s"),
					static_cast<int32>(FailureType), *Error);
				CancelLoadingFlow();
			});
	}
	PrepareMovieLoadingScreen();
}

void UHronoLoadingSubsystem::Deinitialize()
{
	if (PostLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
		PostLoadMapHandle.Reset();
	}
	if (GEngine && TravelFailureHandle.IsValid())
	{
		GEngine->OnTravelFailure().Remove(TravelFailureHandle);
		TravelFailureHandle.Reset();
	}
	if (GEngine && NetworkFailureHandle.IsValid())
	{
		GEngine->OnNetworkFailure().Remove(NetworkFailureHandle);
		NetworkFailureHandle.Reset();
	}
	bTickerEnabled = false;
	CancelLoadingFlow();
	Super::Deinitialize();
}

void UHronoLoadingSubsystem::PreloadGameplayContent(
	TSoftObjectPtr<UWorld> GameplayMap,
	const TArray<TSoftObjectPtr<UObject>>& AdditionalAssets)
{
	if (IsPreloading())
	{
		return;
	}

	ReleasePreloadedAssets();
	bArmNextMapWarmup = false;
	PreloadProgress = 0.0f;
	ShowViewportLoadingOverlay(FText::FromString(TEXT("LOADING CONTENT")), true);
	OnPreloadProgress.Broadcast(PreloadProgress);

	TArray<FSoftObjectPath> Paths;
	if (!GameplayMap.IsNull())
	{
		// Never stream a UWorld as a regular asset immediately before OpenLevel or
		// ServerTravel. In packaged builds that can leave the destination package
		// in an intermediate async-load state when network travel begins.
		TargetGameplayMapName = UWorld::RemovePIEPrefix(
			FPackageName::GetShortName(GameplayMap.ToSoftObjectPath().GetLongPackageName()));
	}
	else
	{
		TargetGameplayMapName.Reset();
	}
	for (const TSoftObjectPtr<UObject>& Asset : AdditionalAssets)
	{
		if (!Asset.IsNull())
		{
			Paths.AddUnique(Asset.ToSoftObjectPath());
		}
	}
	UE_LOG(LogHrono, Log,
		TEXT("Loading flow armed for '%s' with %d explicit asset(s); map loading is deferred to travel."),
		*TargetGameplayMapName, Paths.Num());

	if (Paths.IsEmpty())
	{
		PreloadProgress = 1.0f;
		bArmNextMapWarmup = true;
		if (ViewportProgressBar.IsValid())
		{
			ViewportProgressBar->SetPercent(PreloadProgress);
		}
		OnPreloadProgress.Broadcast(PreloadProgress);
		// Keep this same overlay alive through session creation, travel, and world
		// warmup. Reusing it avoids the two loading-screen flashes seen in PIE.
		OnPreloadCompleted.Broadcast(true);
		return;
	}

	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
	PreloadHandle = StreamableManager.RequestAsyncLoad(
		Paths,
		FStreamableDelegate::CreateUObject(this, &ThisClass::HandlePreloadComplete),
		FStreamableManager::AsyncLoadHighPriority,
		false,
		false,
		TEXT("HronoGameplayPreload"));

	if (PreloadHandle.IsValid())
	{
		PreloadHandle->BindUpdateDelegate(
			FStreamableUpdateDelegate::CreateUObject(this, &ThisClass::HandlePreloadUpdate));
	}
	else
	{
		UE_LOG(LogHrono, Error, TEXT("Unable to create gameplay preload request."));
		HideViewportLoadingOverlay();
		OnPreloadCompleted.Broadcast(false);
	}
}

void UHronoLoadingSubsystem::CancelLoadingFlow()
{
	if (PreloadHandle.IsValid() && PreloadHandle->IsLoadingInProgress())
	{
		PreloadHandle->CancelHandle();
	}
	bArmNextMapWarmup = false;
	TargetGameplayMapName.Reset();

	if (bWorldWarmupActive)
	{
		FinishWorldWarmup(false);
	}
	else
	{
		HideViewportLoadingOverlay();
		ReleasePreloadedAssets();
	}
}

bool UHronoLoadingSubsystem::IsPreloading() const
{
	return PreloadHandle.IsValid() && PreloadHandle->IsLoadingInProgress();
}

void UHronoLoadingSubsystem::PrepareMovieLoadingScreen()
{
	if (!IsMoviePlayerEnabled())
	{
		return;
	}

	IGameMoviePlayer* MoviePlayer = GetMoviePlayer();
	if (!MoviePlayer || !MoviePlayer->IsInitialized() || MoviePlayer->IsMovieCurrentlyPlaying())
	{
		bPrepareMovieScreenWhenIdle = true;
		return;
	}

	FLoadingScreenAttributes Attributes;
	Attributes.MinimumLoadingScreenDisplayTime = 0.25f;
	Attributes.bAutoCompleteWhenLoadingCompletes = true;
	Attributes.bMoviesAreSkippable = false;
	Attributes.bWaitForManualStop = false;
	Attributes.bAllowEngineTick = false;
	Attributes.WidgetLoadingScreen = HronoLoading::MakeMovieLoadingScreen();
	MoviePlayer->SetupLoadingScreen(Attributes);
	bPrepareMovieScreenWhenIdle = false;
}

void UHronoLoadingSubsystem::HandlePostLoadMap(UWorld* LoadedWorld)
{
	bPrepareMovieScreenWhenIdle = true;
	if (!bArmNextMapWarmup || !LoadedWorld || !LoadedWorld->IsGameWorld())
	{
		return;
	}

	const FString LoadedMapName = UWorld::RemovePIEPrefix(
		FPackageName::GetShortName(LoadedWorld->GetOutermost()->GetName()));
	UE_LOG(LogHrono, Log,
		TEXT("Post-load map '%s' received; expected gameplay map is '%s'."),
		*LoadedMapName, *TargetGameplayMapName);
	if (TargetGameplayMapName.IsEmpty() || LoadedMapName.Equals(TargetGameplayMapName, ESearchCase::IgnoreCase))
	{
		bArmNextMapWarmup = false;
		BeginWorldWarmup(LoadedWorld);
	}
}

void UHronoLoadingSubsystem::HandlePreloadUpdate(TSharedRef<FStreamableHandle> Handle)
{
	PreloadProgress = FMath::Clamp(Handle->GetProgress(), 0.0f, 1.0f);
	if (ViewportProgressBar.IsValid())
	{
		ViewportProgressBar->SetPercent(PreloadProgress);
	}
	OnPreloadProgress.Broadcast(PreloadProgress);
}

void UHronoLoadingSubsystem::HandlePreloadComplete()
{
	const bool bSuccess = PreloadHandle.IsValid()
		&& !PreloadHandle->WasCanceled()
		&& !PreloadHandle->HasError();
	PreloadProgress = bSuccess ? 1.0f : PreloadProgress;
	if (ViewportProgressBar.IsValid())
	{
		ViewportProgressBar->SetPercent(PreloadProgress);
	}
	OnPreloadProgress.Broadcast(PreloadProgress);

	if (bSuccess)
	{
		bArmNextMapWarmup = true;
		// Do not remove the viewport overlay here. It remains visible while the
		// Blueprint creates/joins the session and is reused by BeginWorldWarmup.
	}
	else
	{
		UE_LOG(LogHrono, Error, TEXT("One or more gameplay assets failed to preload."));
		HideViewportLoadingOverlay();
		ReleasePreloadedAssets();
	}
	OnPreloadCompleted.Broadcast(bSuccess);
}

void UHronoLoadingSubsystem::BeginWorldWarmup(UWorld* LoadedWorld)
{
	UE_LOG(LogHrono, Log, TEXT("Beginning post-travel warmup for '%s'."),
		*GetNameSafe(LoadedWorld));
	WarmupWorld = LoadedWorld;
	WarmupStartTime = FPlatformTime::Seconds();
	StableReadyFrames = 0;
	bWorldWarmupActive = true;
	ShowViewportLoadingOverlay(FText::FromString(TEXT("PREPARING WORLD")), false);
	SetPlayerInputBlocked(LoadedWorld, true);
}

void UHronoLoadingSubsystem::FinishWorldWarmup(bool bTimedOut)
{
	if (bTimedOut)
	{
		UE_LOG(LogHrono, Warning,
			TEXT("World warmup reached its %.0f second safety timeout; gameplay was released."),
			MaximumWorldWarmupSeconds);
	}
	if (UWorld* World = WarmupWorld.Get())
	{
		SetPlayerInputBlocked(World, false);
	}
	UE_LOG(LogHrono, Log, TEXT("Post-travel warmup finished (timed out: %s)."),
		bTimedOut ? TEXT("true") : TEXT("false"));
	bWorldWarmupActive = false;
	WarmupWorld.Reset();
	TargetGameplayMapName.Reset();
	StableReadyFrames = 0;
	HideViewportLoadingOverlay();
	ReleasePreloadedAssets();
	OnWorldReady.Broadcast();
}

void UHronoLoadingSubsystem::ShowViewportLoadingOverlay(const FText& Status, bool bDeterminate)
{
	if (!GEngine || !GEngine->GameViewport)
	{
		return;
	}

	if (!ViewportLoadingOverlay.IsValid())
	{
		ViewportLoadingOverlay =
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
			.BorderBackgroundColor(FLinearColor::Black)
			.Padding(0.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(20.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("DIVIDED")))
						.Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 52))
						.ColorAndOpacity(FLinearColor(0.72f, 0.07f, 0.04f, 1.0f))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(10.0f)
					[
						SNew(SBox)
						.WidthOverride(420.0f)
						[
							SAssignNew(ViewportProgressBar, SProgressBar)
							.Percent(PreloadProgress)
							.FillColorAndOpacity(FLinearColor(0.72f, 0.07f, 0.04f, 1.0f))
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(10.0f)
					[
						SAssignNew(ViewportStatusText, STextBlock)
						.Text(Status)
						.Font(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 18))
						.ColorAndOpacity(FLinearColor(0.82f, 0.82f, 0.84f, 1.0f))
					]
				]
			];
		GEngine->GameViewport->AddViewportWidgetContent(ViewportLoadingOverlay.ToSharedRef(), MAX_int32);
	}

	if (ViewportStatusText.IsValid())
	{
		ViewportStatusText->SetText(Status);
	}
	if (ViewportProgressBar.IsValid())
	{
		ViewportProgressBar->SetPercent(
			bDeterminate ? TOptional<float>(PreloadProgress) : TOptional<float>());
	}
}

void UHronoLoadingSubsystem::HideViewportLoadingOverlay()
{
	if (ViewportLoadingOverlay.IsValid() && GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(ViewportLoadingOverlay.ToSharedRef());
	}
	ViewportProgressBar.Reset();
	ViewportStatusText.Reset();
	ViewportLoadingOverlay.Reset();
}

void UHronoLoadingSubsystem::ReleasePreloadedAssets()
{
	if (PreloadHandle.IsValid())
	{
		PreloadHandle->ReleaseHandle();
		PreloadHandle.Reset();
	}
}

bool UHronoLoadingSubsystem::TickLoading(float DeltaSeconds)
{
	if (!bTickerEnabled)
	{
		return false;
	}

	if (bPrepareMovieScreenWhenIdle && IsMoviePlayerEnabled())
	{
		IGameMoviePlayer* MoviePlayer = GetMoviePlayer();
		if (MoviePlayer && MoviePlayer->IsInitialized() && !MoviePlayer->IsMovieCurrentlyPlaying())
		{
			PrepareMovieLoadingScreen();
		}
	}

	if (!bWorldWarmupActive)
	{
		return true;
	}

	UWorld* World = WarmupWorld.Get();
	if (!World)
	{
		FinishWorldWarmup(true);
		return true;
	}
	if (!bPlayerInputBlocked)
	{
		SetPlayerInputBlocked(World, true);
	}

	IStreamingManager::Get().StreamAllResources(0.05f);
	const double Elapsed = FPlatformTime::Seconds() - WarmupStartTime;
	if (Elapsed >= MinimumWorldWarmupSeconds && IsWorldReady(World))
	{
		++StableReadyFrames;
		if (StableReadyFrames >= RequiredStableReadyFrames)
		{
			FinishWorldWarmup(false);
		}
	}
	else
	{
		StableReadyFrames = 0;
	}

	if (bWorldWarmupActive && Elapsed >= MaximumWorldWarmupSeconds)
	{
		FinishWorldWarmup(true);
	}
	return true;
}

bool UHronoLoadingSubsystem::IsWorldReady(UWorld* World) const
{
	if (!World || IsAsyncLoading() || World->IsVisibilityRequestPending())
	{
		return false;
	}
	if (const UWorldPartitionSubsystem* WorldPartition = World->GetSubsystem<UWorldPartitionSubsystem>())
	{
		if (!WorldPartition->IsStreamingCompleted())
		{
			return false;
		}
	}
	return IStreamingManager::Get().GetNumWantingResources() == 0
		&& UKismetRenderingLibrary::NumPrecompilingPSOsRemaining() == 0;
}

void UHronoLoadingSubsystem::SetPlayerInputBlocked(UWorld* World, bool bBlocked)
{
	if (!World || bPlayerInputBlocked == bBlocked)
	{
		return;
	}
	if (APlayerController* PlayerController = World->GetFirstPlayerController())
	{
		PlayerController->SetIgnoreMoveInput(bBlocked);
		PlayerController->SetIgnoreLookInput(bBlocked);
		if (!bBlocked)
		{
			FInputModeGameOnly GameOnlyInput;
			PlayerController->SetInputMode(GameOnlyInput);
			PlayerController->bShowMouseCursor = false;
		}
		bPlayerInputBlocked = bBlocked;
	}
}
