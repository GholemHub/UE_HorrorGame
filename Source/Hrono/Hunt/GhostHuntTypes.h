#pragma once

#include "CoreMinimal.h"
#include "HronoSharedTools.h"
#include "GhostHuntTypes.generated.h"

/** Coarse, replicated danger band. The exact numeric Threat value remains server-only. */
UENUM(BlueprintType)
enum class EGhostThreatState : uint8
{
	Dormant UMETA(DisplayName = "Dormant"),
	Disturbed UMETA(DisplayName = "Disturbed"),
	Manifesting UMETA(DisplayName = "Manifesting"),
	HuntEligible UMETA(DisplayName = "Hunt Eligible")
};

/** High-level hunt orchestration state. Demon locomotion/combat remains in the AI. */
UENUM(BlueprintType)
enum class EGhostHuntState : uint8
{
	None UMETA(DisplayName = "None"),
	Warning UMETA(DisplayName = "Warning"),
	Manifestation UMETA(DisplayName = "Manifestation"),
	Searching UMETA(DisplayName = "Searching"),
	Chasing UMETA(DisplayName = "Chasing"),
	Ending UMETA(DisplayName = "Ending"),
	Cooldown UMETA(DisplayName = "Cooldown")
};

UENUM(BlueprintType)
enum class EGhostHuntType : uint8
{
	Organic UMETA(DisplayName = "Organic Hunt"),
	Triggered UMETA(DisplayName = "Triggered Hunt")
};

/** Environmental warning selected by the server and implemented by Blueprint listeners. */
UENUM(BlueprintType)
enum class EGhostHuntOmen : uint8
{
	LightsFlicker UMETA(DisplayName = "Lights Flicker"),
	RadioInterference UMETA(DisplayName = "Radio Interference"),
	DosimeterSpike UMETA(DisplayName = "Dosimeter Spike"),
	ClocksStop UMETA(DisplayName = "Clocks Stop"),
	DoorsClose UMETA(DisplayName = "Doors Slowly Close"),
	Footsteps UMETA(DisplayName = "Footsteps"),
	StrangeSound UMETA(DisplayName = "Strange Sound"),
	MirrorAnomaly UMETA(DisplayName = "Mirror Anomaly"),
	GhostManifestation UMETA(DisplayName = "Temporary Ghost Manifestation")
};

/** Server-side sensory information passed from perception, noise, and hiding systems to the Demon AI. */
UENUM(BlueprintType)
enum class EGhostHuntStimulus : uint8
{
	VisualDetection UMETA(DisplayName = "Visual Detection"),
	PlayerNoise UMETA(DisplayName = "Player Noise"),
	LostSight UMETA(DisplayName = "Lost Sight"),
	HidingPlaceObserved UMETA(DisplayName = "Hiding Place Observed")
};
