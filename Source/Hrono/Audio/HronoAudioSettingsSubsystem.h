#pragma once

#include "CoreMinimal.h"
#include "AudioDeviceHandle.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HronoAudioSettingsSubsystem.generated.h"

class USoundClass;
class USoundMix;

/** One local owner of user volume settings and one persistent mix per audio device. */
UCLASS(BlueprintType)
class HRONO_API UHronoAudioSettingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UHronoAudioSettingsSubsystem();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Hrono|Audio")
	void SetVolumes(float Master, float Music, float Sfx, float FadeTime = 0.1f);
	void ApplyToWorld(UWorld* World, float FadeTime = 0.0f);

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Hrono|Audio")
	float MasterVolume = 1.0f;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Hrono|Audio")
	float MusicVolume = 1.0f;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Hrono|Audio")
	float SfxVolume = 1.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Hrono|Audio")
	TObjectPtr<USoundClass> MasterSoundClass;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Hrono|Audio")
	TObjectPtr<USoundClass> MusicSoundClass;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Hrono|Audio")
	TObjectPtr<USoundClass> SfxSoundClass;

private:
	void HandlePostLoadMap(UWorld* World);
	void ReleaseMix();
	UPROPERTY(Transient)
	TObjectPtr<USoundMix> UserVolumeMix;
	FAudioDeviceHandle AppliedDevice;
	FDelegateHandle PostLoadMapHandle;
};
