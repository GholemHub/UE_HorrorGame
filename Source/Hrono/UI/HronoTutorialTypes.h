#pragma once

#include "CoreMinimal.h"
#include "HronoTutorialTypes.generated.h"

/** Equipment pages supported by the in-game tutorial journal. */
UENUM(BlueprintType)
enum class EHronoTutorialItem : uint8
{
	None UMETA(Hidden),
	Monocle UMETA(DisplayName = "Monocle"),
	Dosimeter UMETA(DisplayName = "Dosimeter"),
	Skull UMETA(DisplayName = "Skull"),
	Axe UMETA(DisplayName = "Axe"),
	// Appended to preserve serialized numeric values of the existing item entries.
	Clock UMETA(DisplayName = "Clock"),
	TableRitual UMETA(DisplayName = "Table Ritual"),
	Mirror UMETA(DisplayName = "Mirror")
};

/** Ordered, server-authoritative objectives shown by the bottom-screen tutorial todo. */
UENUM(BlueprintType)
enum class EHronoTutorialStep : uint8
{
	PickUpMonocle UMETA(DisplayName = "Pick Up Monocle"),
	FindPaintingAnomaly UMETA(DisplayName = "Find Painting Anomaly"),
	InteractWithClock UMETA(DisplayName = "Interact With Clock"),
	DetectDosimeterAnomaly UMETA(DisplayName = "Detect Dosimeter Anomaly"),
	StartCorrectRoomRitual UMETA(DisplayName = "Start Correct Room Ritual"),
	PickUpKey UMETA(DisplayName = "Pick Up Key"),
	UnlockOfficeDoor UMETA(DisplayName = "Unlock Office Door"),
	SeatAllPlayers UMETA(DisplayName = "Seat All Players"),
	EnterDemonName UMETA(DisplayName = "Enter Demon Name"),
	UnitePlayerTimelines UMETA(DisplayName = "Unite Player Timelines"),
	Completed UMETA(DisplayName = "Completed")
};
