// Shared collision channel definitions for the timeline system.
// These map to the custom channels defined in DefaultEngine.ini.

#pragma once

#include "Engine/EngineTypes.h"

// Custom Object Type channels:
// Characters set their capsule to one of these based on their timeline.
#define COLLISION_CHANNEL_PAWN_PAST    ECC_GameTraceChannel2
#define COLLISION_CHANNEL_PAWN_FUTURE  ECC_GameTraceChannel3

// Doors/interactables set their collision mesh to one of these based on their timeline.
#define COLLISION_CHANNEL_DOOR_PAST    ECC_GameTraceChannel4
#define COLLISION_CHANNEL_DOOR_FUTURE  ECC_GameTraceChannel5
