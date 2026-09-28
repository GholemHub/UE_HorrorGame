#include "Audio/HronoAudioLibrary.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "GameFramework/Actor.h"

void UHronoAudioLibrary::StopManagedSound(UAudioComponent* Component)
{
    if (!IsValid(Component)) return;
    Component->Stop();
    Component->DestroyComponent();
}

UAudioComponent* UHronoAudioLibrary::ReplaceAttachedSound(UAudioComponent* Previous,
    USoundBase* Sound, USceneComponent* AttachToComponent)
{
    // Stop first even when the replacement is missing or playback is disabled.
    StopManagedSound(Previous);
    if (!IsValid(Sound) || !IsValid(AttachToComponent)
        || !AttachToComponent->GetWorld()
        || AttachToComponent->GetWorld()->GetNetMode() == NM_DedicatedServer) return nullptr;
    return UGameplayStatics::SpawnSoundAttached(Sound, AttachToComponent, NAME_None,
        FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset,
        true, 1.0f, 1.0f, 0.0f, nullptr, nullptr, true);
}

UAudioComponent* UHronoAudioLibrary::ReplaceSoundOnActor(UAudioComponent* Previous,
    USoundBase* Sound, AActor* Actor)
{
    return ReplaceAttachedSound(Previous, Sound,
        IsValid(Actor) ? Actor->GetRootComponent() : nullptr);
}
