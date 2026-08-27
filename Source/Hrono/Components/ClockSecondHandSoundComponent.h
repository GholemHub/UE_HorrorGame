#pragma once

#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "ClockSecondHandSoundComponent.generated.h"

class USoundBase;

/** Plays one short sound whenever a clock's second hand advances. */
UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class HRONO_API UClockSecondHandSoundComponent : public UAudioComponent
{
	GENERATED_BODY()

public:
	UClockSecondHandSoundComponent();

	/** One-shot sound played for a single second-hand step. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock|Second Hand Sound")
	TObjectPtr<USoundBase> TickSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock|Second Hand Sound")
	bool bTickSoundEnabled = true;

	/** Restart the sound when another tick arrives before it has finished. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock|Second Hand Sound")
	bool bRestartTickWhenPlaying = true;

	/** Fast and slow clock modes also modify the sound playback rate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock|Second Hand Sound")
	bool bScalePitchWithClockRate = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock|Second Hand Sound",
		meta = (ClampMin = "0.0"))
	float TickVolumeMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock|Second Hand Sound",
		meta = (ClampMin = "0.01"))
	float BaseTickPitchMultiplier = 1.0f;

	UFUNCTION(BlueprintCallable, Category = "Clock|Second Hand Sound")
	void PlaySecondHandTick(float ClockRateMultiplier = 1.0f);
};
