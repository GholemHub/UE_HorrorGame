#include "Audio/HronoAudioPolicy.h"
#include "HronoCharacter.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

bool HronoAudioPolicy::CanHear(const UObject* Context, EItemTimeline EventTimeline)
{
    const UWorld* World = IsValid(Context) ? Context->GetWorld() : nullptr;
    if (!World || World->GetNetMode() == NM_DedicatedServer) return false;
    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        const APlayerController* Controller = It->Get();
        if (!IsValid(Controller) || !Controller->IsLocalController()) continue;
        const AHronoCharacter* Listener = Cast<AHronoCharacter>(Controller->GetPawn());
        if (IsValid(Listener) && (EventTimeline == EItemTimeline::Both
            || Listener->GetTimeline() == EItemTimeline::Both
            || Listener->GetTimeline() == EventTimeline)) return true;
    }
    return false;
}
