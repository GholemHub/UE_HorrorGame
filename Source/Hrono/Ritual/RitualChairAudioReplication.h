#pragma once

#include "CoreMinimal.h"

class UObject;

/**
 * Plays the ritual-chair creak on a remote client when a replicated table ritual
 * actually starts. Standalone and listen-server audio remains owned by Blueprint.
 */
void PlayRitualChairStartAudioForRemoteClient(
	const UObject* WorldContextObject,
	const FVector& RitualOrigin);
