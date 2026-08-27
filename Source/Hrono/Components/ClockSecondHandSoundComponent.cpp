#include "Components/ClockSecondHandSoundComponent.h"

#include "Sound/SoundBase.h"

UClockSecondHandSoundComponent::UClockSecondHandSoundComponent()
{
	bAutoActivate = false;
	bStopWhenOwnerDestroyed = true;
}

void UClockSecondHandSoundComponent::PlaySecondHandTick(float ClockRateMultiplier)
{
	if (!bTickSoundEnabled || !TickSound)
	{
		return;
	}

	if (IsPlaying())
	{
		if (!bRestartTickWhenPlaying)
		{
			return;
		}
		Stop();
	}

	const float RatePitch = bScalePitchWithClockRate
		? FMath::Max(ClockRateMultiplier, 0.01f)
		: 1.0f;

	SetSound(TickSound);
	SetVolumeMultiplier(TickVolumeMultiplier);
	SetPitchMultiplier(BaseTickPitchMultiplier * RatePitch);
	Play(0.0f);
}
