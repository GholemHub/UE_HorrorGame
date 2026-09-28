#include "Audio/HronoAudioSettingsSubsystem.h"

#include "AudioDevice.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "UI/HronoMenuSettingsSaveGame.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	float UnitVolume(float Value)
	{
		return FMath::IsFinite(Value) ? FMath::Clamp(Value, 0.0f, 1.0f) : 1.0f;
	}
}

UHronoAudioSettingsSubsystem::UHronoAudioSettingsSubsystem()
{
	static ConstructorHelpers::FObjectFinder<USoundClass> Master(TEXT("/Game/_Alex/Audio/SC_HronoMaster.SC_HronoMaster"));
	static ConstructorHelpers::FObjectFinder<USoundClass> Music(TEXT("/Game/_Alex/Audio/SC_HronoMusic.SC_HronoMusic"));
	static ConstructorHelpers::FObjectFinder<USoundClass> Sfx(TEXT("/Game/_Alex/Audio/SC_HronoSFX.SC_HronoSFX"));
	MasterSoundClass = Master.Object;
	MusicSoundClass = Music.Object;
	SfxSoundClass = Sfx.Object;
}

void UHronoAudioSettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	if (const UHronoMenuSettingsSaveGame* Save = Cast<UHronoMenuSettingsSaveGame>(
		UGameplayStatics::LoadGameFromSlot(TEXT("HronoMenuSettings"), 0)))
	{
		MasterVolume = UnitVolume(Save->MasterVolume);
		MusicVolume = UnitVolume(Save->MusicVolume);
		SfxVolume = UnitVolume(Save->SfxVolume);
	}
	UserVolumeMix = NewObject<USoundMix>(this, TEXT("UserVolumeMix"));
	UserVolumeMix->Duration = -1.0f;
	UserVolumeMix->FadeInTime = 0.0f;
	UserVolumeMix->FadeOutTime = 0.0f;
	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::HandlePostLoadMap);
	ApplyToWorld(GetWorld());
}

void UHronoAudioSettingsSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
	ReleaseMix();
	Super::Deinitialize();
}

void UHronoAudioSettingsSubsystem::ReleaseMix()
{
	if (AppliedDevice.IsValid() && UserVolumeMix)
	{
		AppliedDevice->PopSoundMixModifier(UserVolumeMix);
	}
	AppliedDevice.Reset();
}

void UHronoAudioSettingsSubsystem::HandlePostLoadMap(UWorld* World)
{
	ApplyToWorld(World);
}

void UHronoAudioSettingsSubsystem::SetVolumes(float Master, float Music, float Sfx, float FadeTime)
{
	MasterVolume = UnitVolume(Master);
	MusicVolume = UnitVolume(Music);
	SfxVolume = UnitVolume(Sfx);
	ApplyToWorld(GetWorld(), FadeTime);
}

void UHronoAudioSettingsSubsystem::ApplyToWorld(UWorld* World, float FadeTime)
{
	if (!World || World->GetGameInstance() != GetGameInstance() || World->GetNetMode() == NM_DedicatedServer
		|| !UserVolumeMix || !MasterSoundClass || !MusicSoundClass || !SfxSoundClass) return;
	const FAudioDeviceHandle Device = World->GetAudioDevice();
	if (!Device.IsValid()) return;
	if (!AppliedDevice.IsValid() || AppliedDevice.GetDeviceID() != Device.GetDeviceID())
	{
		ReleaseMix();
		AppliedDevice = Device;
		AppliedDevice->PushSoundMixModifier(UserVolumeMix);
	}
	const float Fade = FMath::IsFinite(FadeTime) ? FMath::Max(0.0f, FadeTime) : 0.0f;
	AppliedDevice->SetSoundMixClassOverride(UserVolumeMix, MasterSoundClass, MasterVolume, 1.0f, Fade, true);
	AppliedDevice->SetSoundMixClassOverride(UserVolumeMix, MusicSoundClass, MusicVolume, 1.0f, Fade, true);
	AppliedDevice->SetSoundMixClassOverride(UserVolumeMix, SfxSoundClass, SfxVolume, 1.0f, Fade, true);
}
