// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/SoftObjectPtr.h"
#include "HronoLoadingSubsystem.generated.h"

class SProgressBar;
class STextBlock;
class SWidget;
class UWorld;
struct FStreamableHandle;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHronoPreloadProgressSignature, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHronoPreloadCompletedSignature, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHronoWorldReadySignature);

/** Persistent preload, travel-screen, and post-travel warmup service. */
UCLASS(BlueprintType)
class HRONO_API UHronoLoadingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Loads the map and optional soft assets, retaining them through the next travel. */
	UFUNCTION(BlueprintCallable, Category = "Hrono|Loading")
	void PreloadGameplayContent(
		TSoftObjectPtr<UWorld> GameplayMap,
		const TArray<TSoftObjectPtr<UObject>>& AdditionalAssets);

	/** Call when Create/Join Session fails and no map travel will happen. */
	UFUNCTION(BlueprintCallable, Category = "Hrono|Loading")
	void CancelLoadingFlow();

	UFUNCTION(BlueprintPure, Category = "Hrono|Loading")
	bool IsPreloading() const;

	UFUNCTION(BlueprintPure, Category = "Hrono|Loading")
	float GetPreloadProgress() const { return PreloadProgress; }

	UPROPERTY(BlueprintAssignable, Category = "Hrono|Loading")
	FHronoPreloadProgressSignature OnPreloadProgress;

	UPROPERTY(BlueprintAssignable, Category = "Hrono|Loading")
	FHronoPreloadCompletedSignature OnPreloadCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Hrono|Loading")
	FHronoWorldReadySignature OnWorldReady;

private:
	void PrepareMovieLoadingScreen();
	void HandlePostLoadMap(UWorld* LoadedWorld);
	void HandlePreloadUpdate(TSharedRef<FStreamableHandle> Handle);
	void HandlePreloadComplete();
	void BeginWorldWarmup(UWorld* LoadedWorld);
	void FinishWorldWarmup(bool bTimedOut);
	void ShowViewportLoadingOverlay(const FText& Status, bool bDeterminate);
	void HideViewportLoadingOverlay();
	void ReleasePreloadedAssets();
	bool TickLoading(float DeltaSeconds);
	bool IsWorldReady(UWorld* World) const;
	void SetPlayerInputBlocked(UWorld* World, bool bBlocked);

	TSharedPtr<FStreamableHandle> PreloadHandle;
	TSharedPtr<SWidget> ViewportLoadingOverlay;
	TSharedPtr<SProgressBar> ViewportProgressBar;
	TSharedPtr<STextBlock> ViewportStatusText;
	TWeakObjectPtr<UWorld> WarmupWorld;
	FString TargetGameplayMapName;
	FDelegateHandle PostLoadMapHandle;
	float PreloadProgress = 0.0f;
	double WarmupStartTime = 0.0;
	int32 StableReadyFrames = 0;
	bool bArmNextMapWarmup = false;
	bool bWorldWarmupActive = false;
	bool bPrepareMovieScreenWhenIdle = false;
	bool bPlayerInputBlocked = false;
	bool bTickerEnabled = false;

	static constexpr float MinimumWorldWarmupSeconds = 1.0f;
	static constexpr float MaximumWorldWarmupSeconds = 30.0f;
	static constexpr int32 RequiredStableReadyFrames = 5;
};
