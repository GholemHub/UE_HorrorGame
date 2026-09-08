#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MirrorOptimizationSubsystem.generated.h"

class AActor;
class APlayerCameraManager;
class APlayerController;

/** Suspends BP_Mirror's Blueprint Tick while it cannot affect the local player's view. */
UCLASS()
class HRONO_API UMirrorOptimizationSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	struct FMirrorActorState
	{
		bool bOriginalTickEnabled = true;
		float LastVisibleTime = -BIG_NUMBER;
	};

	void RefreshMirrorActors();
	bool ShouldTickMirror(
		AActor* MirrorActor,
		FMirrorActorState& MirrorState,
		APlayerController* PlayerController,
		APlayerCameraManager* CameraManager) const;
	void RestoreOriginalTickStates();

	TMap<TWeakObjectPtr<AActor>, FMirrorActorState> MirrorActors;
	float ScanCountdown = 0.0f;
	float VisibilityCheckCountdown = 0.0f;
};
