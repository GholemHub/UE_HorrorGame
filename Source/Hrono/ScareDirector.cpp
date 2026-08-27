#include "ScareDirector.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Components/SceneComponent.h"
#include "Components/Drag_Component.h"
#include "Enviroment/Light_Env.h"
#include "Enviroment/Room.h"
#include "Enviroment/Switcher_Env.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "HronoCharacter.h"
#include "Interface/GhostHuntAIInterface.h"
#include "Items/Base_Item.h"
#include "Items/Drag_Item.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"

#if !UE_BUILD_SHIPPING
#include "HAL/IConsoleManager.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogGhostHuntDirector, Log, All);

namespace HuntDirectorConstants
{
	constexpr float DisturbedThreshold = 30.0f;
	constexpr float ManifestingThreshold = 60.0f;
}

AScareDirector::AScareDirector()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(false);
	SetNetUpdateFrequency(2.0f);
	SetMinNetUpdateFrequency(1.0f);

	AvailableHuntOmens = {
		EGhostHuntOmen::LightsFlicker,
		EGhostHuntOmen::RadioInterference,
		EGhostHuntOmen::DosimeterSpike,
		EGhostHuntOmen::ClocksStop,
		EGhostHuntOmen::DoorsClose,
		EGhostHuntOmen::Footsteps,
		EGhostHuntOmen::StrangeSound,
		EGhostHuntOmen::MirrorAnomaly,
		EGhostHuntOmen::GhostManifestation
	};

}

void AScareDirector::BeginPlay()
{
	Super::BeginPlay();

#if !UE_BUILD_SHIPPING
	StartDebugScreenTimer();
#endif

	if (!HasAuthority())
	{
		return;
	}

	// Do not load Blueprint classes in the native constructor. Loading BP_Babaj
	// while Unreal creates this class's CDO re-enters the Blueprint dependency
	// graph, requests the same CDO, and deadlocks Blueprint compilation.
	// At BeginPlay the CDO and all Blueprint classes are fully linked, making these
	// fallback loads safe. Blueprint-assigned overrides remain untouched.
	if (!BabajClass)
	{
		BabajClass = LoadClass<AActor>(
			nullptr,
			TEXT("/Game/_Alex/AI/BP_Babaj.BP_Babaj_C"));
	}

	if (!BabajSpawnPointClass)
	{
		BabajSpawnPointClass = LoadClass<ABase_Item>(
			nullptr,
			TEXT("/Game/_Alex/Room/BP_ItemPointSpawn.BP_ItemPointSpawn_C"));
	}

	int32 DirectorCount = 0;
	for (TActorIterator<AScareDirector> It(GetWorld()); It; ++It)
	{
		++DirectorCount;
	}

	if (DirectorCount > 1)
	{
		UE_LOG(LogGhostHuntDirector, Warning,
			TEXT("[%s] %d Hunt Directors exist in this world. Only one should orchestrate hunts."),
			*GetName(), DirectorCount);
	}

	MaxThreat = FMath::Max(1.0f, MaxThreat);
	HuntThreshold = FMath::Clamp(HuntThreshold, HuntDirectorConstants::ManifestingThreshold, MaxThreat);
	Threat = FMath::Clamp(Threat, 0.0f, MaxThreat);
	RealWarningsSinceLastFalseAlarm = FMath::Max(0, MinimumRealWarningsBetweenFalseAlarms);
	UpdateThreatState(TEXT("Director initialized"));

	if (bChooseCursedRoomOnBeginPlay)
	{
		ChooseCursedRoom();
	}

	if (PassiveThreatPerInterval > 0.0f && PassiveThreatInterval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(
			PassiveThreatTimerHandle,
			this,
			&AScareDirector::HandlePassiveThreatTimer,
			FMath::Max(0.1f, PassiveThreatInterval),
			true);
	}

	UE_LOG(LogGhostHuntDirector, Log,
		TEXT("[%s] Server Hunt Director ready. Threat is hidden; threshold %.1f, passive +%.1f every %.1fs."),
		*GetName(), HuntThreshold, PassiveThreatPerInterval, PassiveThreatInterval);
}

void AScareDirector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		ClearHuntTimers();
		GetWorldTimerManager().ClearTimer(PassiveThreatTimerHandle);
		GetWorldTimerManager().ClearTimer(CooldownTimerHandle);
		GetWorldTimerManager().ClearTimer(DebugScreenTimerHandle);
		GetWorldTimerManager().ClearTimer(DebugTestScenarioTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void AScareDirector::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AScareDirector, CurrentThreatState);
	DOREPLIFETIME(AScareDirector, CurrentHuntState);
	DOREPLIFETIME(AScareDirector, CurrentHuntTimelineTarget);
	DOREPLIFETIME(AScareDirector, CurrentHuntType);
	DOREPLIFETIME(AScareDirector, CurrentCursedRoom);
	DOREPLIFETIME(AScareDirector, ClockAnomalyPatternSeed);
	DOREPLIFETIME(AScareDirector, HotDotPatternSeed);
	DOREPLIFETIME(AScareDirector, PaintingPatternSeed);
}

AScareDirector* AScareDirector::GetHuntDirector(const UObject* WorldContextObject)
{
	if (!WorldContextObject || !GEngine)
	{
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<AScareDirector> It(World); It; ++It)
	{
		return *It;
	}

	return nullptr;
}

bool AScareDirector::ChooseCursedRoom()
{
	if (!HasAuthority() || !GetWorld())
	{
		return false;
	}

	TArray<ARoom*> ValidRooms;
	ValidRooms.Reserve(CandidateRooms.Num());

	for (ARoom* Room : CandidateRooms)
	{
		if (IsValid(Room))
		{
			ValidRooms.AddUnique(Room);
		}
	}

	if (ValidRooms.IsEmpty())
	{
		for (TActorIterator<ARoom> It(GetWorld()); It; ++It)
		{
			if (IsValid(*It))
			{
				ValidRooms.Add(*It);
			}
		}
	}

	if (ValidRooms.IsEmpty())
	{
		UE_LOG(LogGhostHuntDirector, Warning,
			TEXT("[%s] Cannot choose a cursed room: no Room actors are configured or placed."),
			*GetName());
		return false;
	}

	for (ARoom* Room : ValidRooms)
	{
		Room->SetCursed(false);
	}

	CurrentCursedRoom = ValidRooms[FMath::RandHelper(ValidRooms.Num())];
	CurrentCursedRoom->SetCursed(true);
	if (bConfigureRoomClockAnomalies)
	{
		ConfigureRoomClockAnomalies(ValidRooms);
	}
	if (bConfigureRoomHotDots)
	{
		ConfigureRoomHotDots(ValidRooms);
	}
	if (bConfigureRoomPaintingEvidence)
	{
		ConfigureRoomPaintingEvidence(ValidRooms);
	}

	if (bUseCursedRoomAsHuntOrigin)
	{
		HuntOriginActor = CurrentCursedRoom;
	}

	OnCursedRoomSelected.Broadcast(CurrentCursedRoom);
	ReceiveCursedRoomSelected(CurrentCursedRoom);
	ForceNetUpdate();

	UE_LOG(LogGhostHuntDirector, Log,
		TEXT("[%s] Selected cursed room %s from %d candidate(s)."),
		*GetName(), *GetNameSafe(CurrentCursedRoom.Get()), ValidRooms.Num());
	return true;
}

void AScareDirector::ConfigureRoomClockAnomalies(const TArray<ARoom*>& Rooms)
{
	if (!HasAuthority())
	{
		return;
	}

	ClockAnomalyPatternSeed = FMath::RandHelper(MAX_int32 - 1) + 1;
	FRandomStream Random(ClockAnomalyPatternSeed);

	TArray<int32> OrdinaryPatternDeck;
	const int32 OrdinaryPatternCount = ARoom::GetOrdinaryClockPatternCount();
	OrdinaryPatternDeck.Reserve(OrdinaryPatternCount);
	for (int32 PatternIndex = 0; PatternIndex < OrdinaryPatternCount; ++PatternIndex)
	{
		OrdinaryPatternDeck.Add(PatternIndex);
	}
	for (int32 Index = OrdinaryPatternDeck.Num() - 1; Index > 0; --Index)
	{
		OrdinaryPatternDeck.Swap(Index, Random.RandRange(0, Index));
	}

	int32 OrdinaryRoomIndex = 0;
	for (ARoom* Room : Rooms)
	{
		if (!IsValid(Room))
		{
			continue;
		}

		int32 PatternIndex = 0;
		if (Room == CurrentCursedRoom)
		{
			PatternIndex = Random.RandRange(0, ARoom::GetCursedClockPatternCount() - 1);
		}
		else
		{
			const int32 DeckIndex = OrdinaryRoomIndex % OrdinaryPatternDeck.Num();
			const int32 DeckCycle = OrdinaryRoomIndex / OrdinaryPatternDeck.Num();
			PatternIndex = OrdinaryPatternDeck[DeckIndex] + DeckCycle * OrdinaryPatternDeck.Num();
			++OrdinaryRoomIndex;
		}

		uint32 RoomSeedHash = HashCombine(GetTypeHash(ClockAnomalyPatternSeed), GetTypeHash(Room->GetPathName()));
		RoomSeedHash = HashCombine(RoomSeedHash, GetTypeHash(PatternIndex));
		const int32 RoomSeed = RoomSeedHash == 0 ? 1 : static_cast<int32>(RoomSeedHash);
		Room->ConfigureClockAnomalies(PatternIndex, RoomSeed);
	}

	ForceNetUpdate();
	UE_LOG(LogGhostHuntDirector, Log,
		TEXT("[%s] Distributed clock anomaly patterns to %d room(s) with seed %d."),
		*GetName(), Rooms.Num(), ClockAnomalyPatternSeed);
}

void AScareDirector::ConfigureRoomHotDots(const TArray<ARoom*>& Rooms)
{
	if (!HasAuthority())
	{
		return;
	}

	HotDotPatternSeed = FMath::RandHelper(MAX_int32 - 1) + 1;
	FRandomStream Random(HotDotPatternSeed);
	const int32 FirstOrdinaryPattern = Random.RandRange(0, 1);
	int32 OrdinaryRoomIndex = 0;

	for (ARoom* Room : Rooms)
	{
		if (!IsValid(Room))
		{
			continue;
		}

		const bool bCursedRoom = Room == CurrentCursedRoom;
		const int32 PatternIndex = bCursedRoom
			? 0
			: FirstOrdinaryPattern + OrdinaryRoomIndex++;
		uint32 RoomSeedHash = HashCombine(GetTypeHash(HotDotPatternSeed), GetTypeHash(Room->GetPathName()));
		RoomSeedHash = HashCombine(RoomSeedHash, GetTypeHash(PatternIndex));
		Room->ConfigureHotDots(
			PatternIndex,
			RoomSeedHash == 0 ? 1 : static_cast<int32>(RoomSeedHash));
	}

	ForceNetUpdate();
	UE_LOG(LogGhostHuntDirector, Log,
		TEXT("[%s] Distributed HotDot activation patterns to %d room(s) with seed %d."),
		*GetName(), Rooms.Num(), HotDotPatternSeed);
}

void AScareDirector::ConfigureRoomPaintingEvidence(const TArray<ARoom*>& Rooms)
{
	if (!HasAuthority())
	{
		return;
	}

	PaintingPatternSeed = FMath::RandHelper(MAX_int32 - 1) + 1;
	FRandomStream Random(PaintingPatternSeed);
	const int32 FirstOrdinaryPattern = Random.RandRange(0, 2);
	int32 OrdinaryRoomIndex = 0;

	for (ARoom* Room : Rooms)
	{
		if (!IsValid(Room))
		{
			continue;
		}

		const bool bCursedRoom = Room == CurrentCursedRoom;
		const int32 PatternIndex = bCursedRoom
			? 0
			: FirstOrdinaryPattern + OrdinaryRoomIndex++;
		uint32 RoomSeedHash = HashCombine(GetTypeHash(PaintingPatternSeed), GetTypeHash(Room->GetPathName()));
		RoomSeedHash = HashCombine(RoomSeedHash, GetTypeHash(PatternIndex));
		Room->ConfigurePaintingEvidence(
			PatternIndex,
			RoomSeedHash == 0 ? 1 : static_cast<int32>(RoomSeedHash));
	}

	ForceNetUpdate();
	UE_LOG(LogGhostHuntDirector, Log,
		TEXT("[%s] Distributed painting evidence patterns to %d room(s) with seed %d."),
		*GetName(), Rooms.Num(), PaintingPatternSeed);
}

void AScareDirector::OnRep_CurrentCursedRoom()
{
	OnCursedRoomSelected.Broadcast(CurrentCursedRoom);
	ReceiveCursedRoomSelected(CurrentCursedRoom);
}

void AScareDirector::AddThreat(float Amount)
{
	AddThreatWithReason(Amount, TEXT("External gameplay event called AddThreat"));
}

void AScareDirector::RemoveThreat(float Amount)
{
	RemoveThreatWithReason(Amount, TEXT("External gameplay event called RemoveThreat"));
}

void AScareDirector::SetThreat(float Value)
{
	SetThreatWithReason(Value, TEXT("External gameplay event called SetThreat"));
}

void AScareDirector::AddThreatWithReason(float Amount, const FString& Reason)
{
	if (!HasAuthority() || Amount <= 0.0f)
	{
		return;
	}

	SetThreatInternal(Threat + Amount, Reason);
}

void AScareDirector::RemoveThreatWithReason(float Amount, const FString& Reason)
{
	if (!HasAuthority() || Amount <= 0.0f)
	{
		return;
	}

	SetThreatInternal(Threat - Amount, Reason);
}

void AScareDirector::SetThreatWithReason(float Value, const FString& Reason)
{
	SetThreatInternal(Value, Reason);
}

void AScareDirector::SetThreatInternal(float Value, const FString& Reason)
{
	if (!HasAuthority())
	{
		return;
	}

	const float NewThreat = FMath::Clamp(Value, 0.0f, FMath::Max(1.0f, MaxThreat));
	if (FMath::IsNearlyEqual(Threat, NewThreat))
	{
		return;
	}

	const float PreviousThreat = Threat;
	Threat = NewThreat;
	LastThreatStateChangeReason = Reason.IsEmpty() ? TEXT("Unspecified Threat change") : Reason;
	UE_LOG(LogGhostHuntDirector, Log, TEXT("[%s] Threat %.1f -> %.1f. Reason: %s"),
		*GetName(), PreviousThreat, Threat, *LastThreatStateChangeReason);

	UpdateThreatState(LastThreatStateChangeReason);

	if (CanStartHunt())
	{
		ScheduleNextHuntCheck();
	}
	else if (Threat < HuntThreshold)
	{
		CancelHuntCheck();
	}
}

float AScareDirector::GetThreat() const
{
	return HasAuthority() ? Threat : 0.0f;
}

bool AScareDirector::CanStartHunt() const
{
	return HasAuthority()
		&& Threat >= FMath::Clamp(HuntThreshold, HuntDirectorConstants::ManifestingThreshold, FMath::Max(1.0f, MaxThreat))
		&& !bHuntOnCooldown
		&& CurrentHuntState == EGhostHuntState::None;
}

bool AScareDirector::RequestTriggeredHunt(
	EItemTimeline TargetTimeline,
	bool bUseWarningPhase,
	bool bIgnoreCooldown,
	bool bAllowFalseAlarm)
{
	if (!HasAuthority() || IsHuntActive())
	{
		return false;
	}

	if (bHuntOnCooldown && !bIgnoreCooldown)
	{
		UE_LOG(LogGhostHuntDirector, Log, TEXT("[%s] Triggered hunt rejected during cooldown."), *GetName());
		return false;
	}

	if (bIgnoreCooldown)
	{
		GetWorldTimerManager().ClearTimer(CooldownTimerHandle);
		bHuntOnCooldown = false;
		if (CurrentHuntState == EGhostHuntState::Cooldown)
		{
			SetHuntState(EGhostHuntState::None, TEXT("Triggered hunt explicitly bypassed the active cooldown"));
		}
	}

	CancelHuntCheck();
	CurrentHuntType = EGhostHuntType::Triggered;
	CurrentHuntTimelineTarget = TargetTimeline;
	ForceNetUpdate();

	UE_LOG(LogGhostHuntDirector, Log,
		TEXT("[%s] Triggered hunt accepted. Timeline=%s warning=%s ignoreCooldown=%s falseAlarmAllowed=%s"),
		*GetName(),
		*StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(TargetTimeline)),
		bUseWarningPhase ? TEXT("true") : TEXT("false"),
		bIgnoreCooldown ? TEXT("true") : TEXT("false"),
		bAllowFalseAlarm ? TEXT("true") : TEXT("false"));

	if (bUseWarningPhase)
	{
		BeginWarningPhase(TargetTimeline, bAllowFalseAlarm);
	}
	else
	{
		bPendingFalseAlarm = false;
		SelectedHuntOmens.Reset();
		StartActualHunt();
	}

	return true;
}

void AScareDirector::EndHunt()
{
	if (!HasAuthority()
		|| CurrentHuntState == EGhostHuntState::None
		|| CurrentHuntState == EGhostHuntState::Ending
		|| CurrentHuntState == EGhostHuntState::Cooldown)
	{
		return;
	}

	ClearHuntTimers();
	TargetedPlayer.Reset();
	SetHuntState(EGhostHuntState::Ending, TEXT("EndHunt was requested or the configured hunt duration expired"));

	UE_LOG(LogGhostHuntDirector, Log, TEXT("[%s] Hunt ending. Beginning cooldown in %.1fs."),
		*GetName(), EndingStateDuration);

	if (EndingStateDuration <= 0.0f)
	{
		BeginCooldown();
	}
	else
	{
		GetWorldTimerManager().SetTimer(
			EndingTimerHandle,
			this,
			&AScareDirector::BeginCooldown,
			EndingStateDuration,
			false);
	}
}

bool AScareDirector::IsHuntActive() const
{
	switch (CurrentHuntState)
	{
	case EGhostHuntState::Warning:
	case EGhostHuntState::Manifestation:
	case EGhostHuntState::Searching:
	case EGhostHuntState::Chasing:
	case EGhostHuntState::Ending:
		return true;
	default:
		return false;
	}
}

bool AScareDirector::DoesHuntAffectTimeline(EItemTimeline Timeline) const
{
	return CurrentHuntTimelineTarget == EItemTimeline::Both
		|| Timeline == EItemTimeline::Both
		|| CurrentHuntTimelineTarget == Timeline;
}

void AScareDirector::SetHuntDemon(AActor* NewHuntDemon)
{
	if (!HasAuthority())
	{
		return;
	}

	HuntDemon = NewHuntDemon;
	if (HuntDemon && !ResolveHuntDemonInterfaceTarget())
	{
		UE_LOG(LogGhostHuntDirector, Warning,
			TEXT("[%s] HuntDemon %s and its Controller do not implement GhostHuntAIInterface."),
			*GetName(), *HuntDemon->GetName());
	}
}

void AScareDirector::ReportPlayerDetected(AActor* Player, FVector DetectedPosition, EItemTimeline PlayerTimeline)
{
	if (!HasAuthority() || !IsValid(Player) || !DoesHuntAffectTimeline(PlayerTimeline)
		|| (CurrentHuntState != EGhostHuntState::Manifestation
			&& CurrentHuntState != EGhostHuntState::Searching
			&& CurrentHuntState != EGhostHuntState::Chasing))
	{
		return;
	}

	TargetedPlayer = Player;
	UpdateLastKnownPlayerPosition(DetectedPosition);
	DispatchStimulus(EGhostHuntStimulus::VisualDetection, Player, nullptr, DetectedPosition, PlayerTimeline);

	if (CurrentHuntState == EGhostHuntState::Searching)
	{
		SetHuntState(EGhostHuntState::Chasing, TEXT("Demon visually detected a player"));
	}
}

void AScareDirector::ReportPlayerNoise(AActor* NoiseSource, FVector NoiseLocation, EItemTimeline NoiseTimeline)
{
	if (!HasAuthority() || !DoesHuntAffectTimeline(NoiseTimeline)
		|| (CurrentHuntState != EGhostHuntState::Manifestation
			&& CurrentHuntState != EGhostHuntState::Searching
			&& CurrentHuntState != EGhostHuntState::Chasing))
	{
		return;
	}

	UpdateLastKnownPlayerPosition(NoiseLocation);
	DispatchStimulus(EGhostHuntStimulus::PlayerNoise, NoiseSource, nullptr, NoiseLocation, NoiseTimeline);
}

void AScareDirector::ReportPlayerLost(AActor* Player, FVector LastSeenPosition, EItemTimeline PlayerTimeline)
{
	if (!HasAuthority() || !DoesHuntAffectTimeline(PlayerTimeline)
		|| (CurrentHuntState != EGhostHuntState::Searching && CurrentHuntState != EGhostHuntState::Chasing))
	{
		return;
	}

	UpdateLastKnownPlayerPosition(LastSeenPosition);
	DispatchStimulus(EGhostHuntStimulus::LostSight, Player, nullptr, LastSeenPosition, PlayerTimeline);

	if (CurrentHuntState == EGhostHuntState::Chasing
		&& (!TargetedPlayer.IsValid() || TargetedPlayer.Get() == Player))
	{
		TargetedPlayer.Reset();
		SetHuntState(EGhostHuntState::Searching, TEXT("Demon lost visual contact; searching LastKnownPlayerPosition"));
	}
}

void AScareDirector::ReportPlayerEnteredHiding(
	AActor* Player,
	AActor* HidingPlace,
	bool bDemonObservedEntry,
	FVector LastVisiblePosition,
	EItemTimeline PlayerTimeline)
{
	if (!HasAuthority() || !DoesHuntAffectTimeline(PlayerTimeline)
		|| (CurrentHuntState != EGhostHuntState::Searching && CurrentHuntState != EGhostHuntState::Chasing))
	{
		return;
	}

	if (bDemonObservedEntry && IsValid(HidingPlace))
	{
		const FVector InvestigationLocation = HidingPlace->GetActorLocation();
		UpdateLastKnownPlayerPosition(InvestigationLocation);
		DispatchStimulus(EGhostHuntStimulus::HidingPlaceObserved, Player, HidingPlace, InvestigationLocation, PlayerTimeline);
	}
	else
	{
		UpdateLastKnownPlayerPosition(LastVisiblePosition);
		// Deliberately do not send HidingPlace: the Demon did not see which one was used.
		DispatchStimulus(EGhostHuntStimulus::LostSight, Player, nullptr, LastVisiblePosition, PlayerTimeline);
	}

	if (CurrentHuntState == EGhostHuntState::Chasing
		&& (!TargetedPlayer.IsValid() || TargetedPlayer.Get() == Player))
	{
		TargetedPlayer.Reset();
		SetHuntState(
			EGhostHuntState::Searching,
			bDemonObservedEntry
				? TEXT("Demon saw the player enter hiding and will investigate that hiding place")
				: TEXT("Player broke line of sight before hiding; searching only the last visible position"));
	}
}

void AScareDirector::HandlePassiveThreatTimer()
{
	AddThreatWithReason(PassiveThreatPerInterval, TEXT("Passive Threat increase over time"));
}

void AScareDirector::UpdateThreatState(const FString& Reason)
{
	const EGhostThreatState NewState = CalculateThreatState();
	if (NewState == CurrentThreatState)
	{
		return;
	}

	const EGhostThreatState OldState = CurrentThreatState;
	CurrentThreatState = NewState;
	LastThreatStateChangeReason = Reason.IsEmpty() ? TEXT("Threat crossed an aggression threshold") : Reason;
	ForceNetUpdate();
	DispatchThreatStateChanged(OldState, NewState);
}

EGhostThreatState AScareDirector::CalculateThreatState() const
{
	const float EffectiveHuntThreshold = FMath::Clamp(
		HuntThreshold,
		HuntDirectorConstants::ManifestingThreshold,
		FMath::Max(1.0f, MaxThreat));

	if (Threat >= EffectiveHuntThreshold)
	{
		return EGhostThreatState::HuntEligible;
	}
	if (Threat >= HuntDirectorConstants::ManifestingThreshold)
	{
		return EGhostThreatState::Manifesting;
	}
	if (Threat >= HuntDirectorConstants::DisturbedThreshold)
	{
		return EGhostThreatState::Disturbed;
	}
	return EGhostThreatState::Dormant;
}

void AScareDirector::SetHuntState(EGhostHuntState NewState, const FString& Reason)
{
	if (!HasAuthority() || NewState == CurrentHuntState)
	{
		return;
	}

	const EGhostHuntState OldState = CurrentHuntState;
	CurrentHuntState = NewState;
	LastHuntStateChangeReason = Reason.IsEmpty() ? TEXT("Director state transition") : Reason;
	ForceNetUpdate();
	DispatchHuntStateChanged(OldState, NewState);
}

void AScareDirector::OnRep_CurrentThreatState(EGhostThreatState PreviousState)
{
	LastThreatStateChangeReason = TEXT("State replicated by the authoritative server");
	DispatchThreatStateChanged(PreviousState, CurrentThreatState);
}

void AScareDirector::OnRep_CurrentHuntState(EGhostHuntState PreviousState)
{
	LastHuntStateChangeReason = TEXT("State replicated by the authoritative server");
	DispatchHuntStateChanged(PreviousState, CurrentHuntState);
}

void AScareDirector::DispatchThreatStateChanged(EGhostThreatState OldState, EGhostThreatState NewState)
{
	UE_LOG(LogGhostHuntDirector, Log, TEXT("[%s] Threat state %s -> %s%s"),
		*GetName(),
		*StaticEnum<EGhostThreatState>()->GetNameStringByValue(static_cast<int64>(OldState)),
		*StaticEnum<EGhostThreatState>()->GetNameStringByValue(static_cast<int64>(NewState)),
		HasAuthority() ? TEXT(" [server]") : TEXT(" [client]"));

	OnThreatStateChanged.Broadcast(OldState, NewState);
	ReceiveThreatStateChanged(OldState, NewState);

	PrintHuntDebugMessage(
		FString::Printf(
			TEXT("AGGRESSION CHANGED: %s -> %s\nThreat: %s\nWHY: %s"),
			*StaticEnum<EGhostThreatState>()->GetNameStringByValue(static_cast<int64>(OldState)),
			*StaticEnum<EGhostThreatState>()->GetNameStringByValue(static_cast<int64>(NewState)),
			HasAuthority() ? *FString::Printf(TEXT("%.1f / %.1f"), Threat, MaxThreat) : TEXT("hidden on client"),
			*LastThreatStateChangeReason),
		GetThreatDebugColor(),
		6.0f);

	if (bTurnOffAllLightsOnThreatStateChange)
	{
		TurnOffAllLightsForThreatState(NewState);
	}

	if (HasAuthority())
	{
		if (NewState == EGhostThreatState::HuntEligible)
		{
			SpawnBabajAtRandomPoint();
		}

		if (bAnimateDoorsOnThreatStateChanges && NewState == EGhostThreatState::Manifesting)
		{
			AnimateAllDoorsForThreatState(false, TEXT("Aggression entered Manifesting"));
		}
		else if (bAnimateDoorsOnThreatStateChanges && NewState == EGhostThreatState::HuntEligible)
		{
			AnimateAllDoorsForThreatState(true, TEXT("Aggression entered HuntEligible"));
		}
	}
}

bool AScareDirector::SpawnBabajAtRandomPoint()
{
	if (!HasAuthority() || !GetWorld())
	{
		return false;
	}

	TArray<ABase_Item*> ValidSpawnPoints;
	if (!BabajSpawnPoints.IsEmpty())
	{
		for (ABase_Item* SpawnPoint : BabajSpawnPoints)
		{
			if (IsValid(SpawnPoint))
			{
				ValidSpawnPoints.AddUnique(SpawnPoint);
			}
		}
	}
	else if (BabajSpawnPointClass)
	{
		for (TActorIterator<ABase_Item> It(GetWorld(), BabajSpawnPointClass); It; ++It)
		{
			if (IsValid(*It))
			{
				ValidSpawnPoints.Add(*It);
			}
		}
	}

	if (ValidSpawnPoints.IsEmpty())
	{
		UE_LOG(LogGhostHuntDirector, Error,
			TEXT("[%s] Cannot spawn BP_Babaj: no valid BP_ItemPointSpawn actors were configured or found."),
			*GetName());
		return false;
	}

	ABase_Item* SpawnPoint = ValidSpawnPoints[FMath::RandRange(0, ValidSpawnPoints.Num() - 1)];
	USceneComponent* SpawnComponent = ResolveBabajSpawnComponent(SpawnPoint);
	const FTransform SpawnTransform = SpawnComponent
		? SpawnComponent->GetComponentTransform()
		: SpawnPoint->GetActorTransform();

	const float Lifetime = FMath::Clamp(BabajLifetime, 0.1f, 40.0f);
	ReceiveSpawnBabaj(SpawnPoint, SpawnTransform, BabajClass, Lifetime);

	UE_LOG(LogGhostHuntDirector, Log,
		TEXT("[%s] Requested Blueprint Babaj spawn at random point %s."),
		*GetName(), *SpawnPoint->GetName());

	return true;
}

USceneComponent* AScareDirector::ResolveBabajSpawnComponent(const ABase_Item* SpawnPoint) const
{
	if (!IsValid(SpawnPoint))
	{
		return nullptr;
	}

	TArray<USceneComponent*> SceneComponents;
	SpawnPoint->GetComponents(SceneComponents);
	for (USceneComponent* SceneComponent : SceneComponents)
	{
		if (!IsValid(SceneComponent))
		{
			continue;
		}

		const FName ComponentName = SceneComponent->GetFName();
		if (ComponentName == TEXT("PointSetComponent") || ComponentName == TEXT("PointSet"))
		{
			return SceneComponent;
		}
	}

	return SpawnPoint->GetRootComponent();
}

void AScareDirector::TurnOffAllLightsForThreatState(EGhostThreatState NewState)
{
	if (!GetWorld())
	{
		return;
	}

	int32 EnvironmentLightCount = 0;
	for (TActorIterator<ALight_Env> It(GetWorld()); It; ++It)
	{
		if (ALight_Env* EnvironmentLight = *It)
		{
			EnvironmentLight->SetLightEnabled(false);
			++EnvironmentLightCount;
		}
	}

	int32 LinkedLightCount = 0;
	int32 ChangedSwitchCount = 0;
	for (TActorIterator<ASwitcher_Env> It(GetWorld()); It; ++It)
	{
		if (ASwitcher_Env* Switcher = *It)
		{
			// Apply on every machine at every aggression transition. This supports
			// linked Blueprint lamps which are plain AActor + LightComponent and
			// also re-enforces OFF if their switch bool was already false.
			LinkedLightCount += Switcher->ApplyLightStateToLinkedActors(false);
			if (HasAuthority())
			{
				ChangedSwitchCount += Switcher->SetLightState(false) ? 1 : 0;
			}
		}
	}

	if (HasAuthority())
	{
		const FString StateName = StaticEnum<EGhostThreatState>()->GetNameStringByValue(
			static_cast<int64>(NewState));
		UE_LOG(LogGhostHuntDirector, Log,
			TEXT("[%s] Aggression entered %s: forced OFF %d Light_Env actors and %d linked light components; changed %d switches."),
			*GetName(), *StateName, EnvironmentLightCount, LinkedLightCount, ChangedSwitchCount);

		PrintHuntDebugMessage(
			FString::Printf(
				TEXT("LIGHT BLACKOUT: %d Light_Env + %d linked lights OFF\n%d switches changed\nWHY: AGGRESSION entered %s"),
				EnvironmentLightCount,
				LinkedLightCount,
				ChangedSwitchCount,
				*StateName),
			FLinearColor(0.35f, 0.35f, 0.5f),
			6.0f);
	}
}

void AScareDirector::AnimateAllDoorsForThreatState(bool bOpen, const FString& Reason)
{
	if (!HasAuthority() || !GetWorld())
	{
		return;
	}

	int32 AnimatedDoorCount = 0;
	int32 DisabledDoorCount = 0;
	for (TActorIterator<ADrag_Item> It(GetWorld()); It; ++It)
	{
		ADrag_Item* Door = *It;
		if (!IsValid(Door)
			|| !IsValid(Door->DragComponent)
			|| Door->DragComponent->bIsShelf
			|| Door->DragComponent->bIsCupBoard)
		{
			continue;
		}

		if (!Door->bAllowAnimateDoorOpenClose)
		{
			++DisabledDoorCount;
			continue;
		}

		Door->AnimateDoor(bOpen);
		++AnimatedDoorCount;
	}

	const FString DoorAction = bOpen ? TEXT("OPENED") : TEXT("CLOSED");
	UE_LOG(LogGhostHuntDirector, Log,
		TEXT("[%s] %s %d doors through AnimateDoor; %d doors opted out. Reason: %s"),
		*GetName(), *DoorAction, AnimatedDoorCount, DisabledDoorCount, *Reason);

	PrintHuntDebugMessage(
		FString::Printf(
			TEXT("DOOR EVENT: %s %d doors\n%d doors disabled animation\nWHY: %s"),
			*DoorAction,
			AnimatedDoorCount,
			DisabledDoorCount,
			*Reason),
		bOpen ? FLinearColor(0.2f, 1.0f, 0.25f) : FLinearColor(1.0f, 0.1f, 0.05f),
		6.0f);
}

void AScareDirector::DispatchHuntStateChanged(EGhostHuntState OldState, EGhostHuntState NewState)
{
	UE_LOG(LogGhostHuntDirector, Log, TEXT("[%s] Hunt state %s -> %s%s"),
		*GetName(),
		*StaticEnum<EGhostHuntState>()->GetNameStringByValue(static_cast<int64>(OldState)),
		*StaticEnum<EGhostHuntState>()->GetNameStringByValue(static_cast<int64>(NewState)),
		HasAuthority() ? TEXT(" [server]") : TEXT(" [client]"));

	OnHuntStateChanged.Broadcast(OldState, NewState);
	ReceiveHuntStateChanged(OldState, NewState);

	PrintHuntDebugMessage(
		FString::Printf(
			TEXT("HUNT STATE CHANGED: %s -> %s\nWHY: %s"),
			*StaticEnum<EGhostHuntState>()->GetNameStringByValue(static_cast<int64>(OldState)),
			*StaticEnum<EGhostHuntState>()->GetNameStringByValue(static_cast<int64>(NewState)),
			*LastHuntStateChangeReason),
		FLinearColor(1.0f, 0.35f, 0.08f),
		6.0f);

	if (NewState == EGhostHuntState::Warning)
	{
		OnHuntWarningStarted.Broadcast(CurrentHuntTimelineTarget);
		ReceiveHuntWarningStarted(CurrentHuntTimelineTarget);
	}
	else if (NewState == EGhostHuntState::Manifestation)
	{
		OnHuntStarted.Broadcast(CurrentHuntTimelineTarget);
		ReceiveHuntStarted(CurrentHuntTimelineTarget);
	}
	else if (NewState == EGhostHuntState::Ending)
	{
		OnHuntEnded.Broadcast(CurrentHuntTimelineTarget);
		ReceiveHuntEnded(CurrentHuntTimelineTarget);
	}

	if (HasAuthority())
	{
		NotifyDemonOfState(NewState);
	}
}

void AScareDirector::ScheduleNextHuntCheck()
{
	if (!CanStartHunt() || GetWorldTimerManager().IsTimerActive(HuntCheckTimerHandle))
	{
		return;
	}

	const float MinInterval = FMath::Max(0.1f, FMath::Min(HuntCheckIntervalMin, HuntCheckIntervalMax));
	const float MaxInterval = FMath::Max(MinInterval, FMath::Max(HuntCheckIntervalMin, HuntCheckIntervalMax));
	const float Delay = FMath::FRandRange(MinInterval, MaxInterval);
	GetWorldTimerManager().SetTimer(HuntCheckTimerHandle, this, &AScareDirector::EvaluateOrganicHunt, Delay, false);

	UE_LOG(LogGhostHuntDirector, Verbose, TEXT("[%s] Hunt eligible; evaluation scheduled in %.1fs."), *GetName(), Delay);
}

void AScareDirector::EvaluateOrganicHunt()
{
	if (!CanStartHunt())
	{
		return;
	}

	const float Roll = FMath::FRand();
	if (Roll <= FMath::Clamp(HuntChance, 0.0f, 1.0f))
	{
		UE_LOG(LogGhostHuntDirector, Log,
			TEXT("[%s] Organic hunt check succeeded (roll %.3f, chance %.3f)."),
			*GetName(), Roll, HuntChance);
		CurrentHuntType = EGhostHuntType::Organic;
		BeginWarningPhase(OrganicHuntTimelineTarget, true);
	}
	else
	{
		UE_LOG(LogGhostHuntDirector, Verbose,
			TEXT("[%s] Organic hunt check failed (roll %.3f, chance %.3f)."),
			*GetName(), Roll, HuntChance);
		ScheduleNextHuntCheck();
	}
}

void AScareDirector::CancelHuntCheck()
{
	if (GetWorld())
	{
		GetWorldTimerManager().ClearTimer(HuntCheckTimerHandle);
	}
}

void AScareDirector::BeginWarningPhase(EItemTimeline TargetTimeline, bool bAllowFalseAlarm)
{
	if (!HasAuthority() || CurrentHuntState != EGhostHuntState::None)
	{
		return;
	}

	ClearHuntTimers();
	CurrentHuntTimelineTarget = TargetTimeline;
	ActiveSearchOrigin = ResolveSearchOrigin();
	bHasLastKnownPlayerPosition = false;
	LastKnownPlayerPosition = FVector::ZeroVector;
	TargetedPlayer.Reset();

	const bool bFalseAlarmGateOpen = RealWarningsSinceLastFalseAlarm >= FMath::Max(0, MinimumRealWarningsBetweenFalseAlarms);
	bPendingFalseAlarm = bAllowFalseAlarm
		&& bFalseAlarmGateOpen
		&& FMath::FRand() <= FMath::Clamp(FalseAlarmChance, 0.0f, 1.0f);

	SelectHuntOmens();
	ForceNetUpdate();
	SetHuntState(
		EGhostHuntState::Warning,
		CurrentHuntType == EGhostHuntType::Organic
			? TEXT("Organic Hunt eligibility check succeeded")
			: TEXT("A gameplay system requested a Triggered Hunt"));

	UE_LOG(LogGhostHuntDirector, Log,
		TEXT("[%s] Warning started. Timeline=%s omens=[%s] pendingFalseAlarm=%s"),
		*GetName(),
		*StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(CurrentHuntTimelineTarget)),
		*GetSelectedOmensDebugString(),
		bPendingFalseAlarm ? TEXT("true") : TEXT("false"));

	TriggerNextSelectedOmen();
}

void AScareDirector::SelectHuntOmens()
{
	SelectedHuntOmens.Reset();
	NextOmenIndex = 0;

	TArray<EGhostHuntOmen> Candidates;
	for (EGhostHuntOmen Omen : AvailableHuntOmens)
	{
		Candidates.AddUnique(Omen);
	}

	for (int32 Index = Candidates.Num() - 1; Index > 0; --Index)
	{
		Candidates.Swap(Index, FMath::RandRange(0, Index));
	}

	const int32 MinCount = FMath::Clamp(FMath::Min(MinimumOmens, MaximumOmens), 0, Candidates.Num());
	const int32 MaxCount = FMath::Clamp(FMath::Max(MinimumOmens, MaximumOmens), MinCount, Candidates.Num());
	const int32 OmenCount = MaxCount > 0 ? FMath::RandRange(MinCount, MaxCount) : 0;

	for (int32 Index = 0; Index < OmenCount; ++Index)
	{
		SelectedHuntOmens.Add(Candidates[Index]);
	}
}

void AScareDirector::TriggerNextSelectedOmen()
{
	if (!HasAuthority() || CurrentHuntState != EGhostHuntState::Warning)
	{
		return;
	}

	if (SelectedHuntOmens.IsValidIndex(NextOmenIndex))
	{
		MulticastTriggerHuntOmen(SelectedHuntOmens[NextOmenIndex], CurrentHuntTimelineTarget);
		++NextOmenIndex;
	}

	if (NextOmenIndex < SelectedHuntOmens.Num())
	{
		const float MinInterval = FMath::Max(0.0f, FMath::Min(OmenIntervalMin, OmenIntervalMax));
		const float MaxInterval = FMath::Max(MinInterval, FMath::Max(OmenIntervalMin, OmenIntervalMax));
		GetWorldTimerManager().SetTimer(
			OmenTimerHandle,
			this,
			&AScareDirector::TriggerNextSelectedOmen,
			FMath::Max(0.01f, FMath::FRandRange(MinInterval, MaxInterval)),
			false);
		return;
	}

	const float MinDelay = FMath::Max(0.0f, FMath::Min(PostOmenDelayMin, PostOmenDelayMax));
	const float MaxDelay = FMath::Max(MinDelay, FMath::Max(PostOmenDelayMin, PostOmenDelayMax));
	const float Delay = FMath::FRandRange(MinDelay, MaxDelay);
	if (Delay <= 0.0f)
	{
		ResolveWarningPhase();
	}
	else
	{
		GetWorldTimerManager().SetTimer(PostOmenTimerHandle, this, &AScareDirector::ResolveWarningPhase, Delay, false);
	}
}

void AScareDirector::ResolveWarningPhase()
{
	if (!HasAuthority() || CurrentHuntState != EGhostHuntState::Warning)
	{
		return;
	}

	if (bPendingFalseAlarm)
	{
		ResolveFalseAlarm();
	}
	else
	{
		StartActualHunt();
	}
}

void AScareDirector::ResolveFalseAlarm()
{
	bPendingFalseAlarm = false;
	RealWarningsSinceLastFalseAlarm = 0;
	SetHuntState(EGhostHuntState::None, TEXT("Warning phase resolved as a false alarm; no Hunt will start"));
	RemoveThreatWithReason(FalseAlarmThreatReduction, TEXT("False alarm released some accumulated aggression"));
	MulticastFalseAlarmResolved(CurrentHuntTimelineTarget);

	UE_LOG(LogGhostHuntDirector, Log, TEXT("[%s] False alarm resolved; Threat reduced by %.1f."),
		*GetName(), FalseAlarmThreatReduction);
}

void AScareDirector::StartActualHunt()
{
	if (!HasAuthority()
		|| (CurrentHuntState != EGhostHuntState::Warning && CurrentHuntState != EGhostHuntState::None))
	{
		return;
	}

	const bool bCompletedWarningPhase = CurrentHuntState == EGhostHuntState::Warning;
	bPendingFalseAlarm = false;
	++RealWarningsSinceLastFalseAlarm;
	ActiveSearchOrigin = ResolveSearchOrigin();
	SetHuntState(
		EGhostHuntState::Manifestation,
		bCompletedWarningPhase
			? TEXT("Omen sequence completed and the warning resolved as a real Hunt")
			: TEXT("Triggered Hunt was configured to skip the warning phase"));

	const float MinDuration = FMath::Max(1.0f, FMath::Min(HuntDurationMin, HuntDurationMax));
	const float MaxDuration = FMath::Max(MinDuration, FMath::Max(HuntDurationMin, HuntDurationMax));
	const float HuntDuration = FMath::FRandRange(MinDuration, MaxDuration);
	GetWorldTimerManager().SetTimer(HuntDurationTimerHandle, this, &AScareDirector::EndHunt, HuntDuration, false);

	if (ManifestationDuration <= 0.0f)
	{
		EnterSearchingState();
	}
	else
	{
		GetWorldTimerManager().SetTimer(
			ManifestationTimerHandle,
			this,
			&AScareDirector::EnterSearchingState,
			ManifestationDuration,
			false);
	}

	UE_LOG(LogGhostHuntDirector, Log,
		TEXT("[%s] Hunt started for %.1fs. Search origin=%s timeline=%s."),
		*GetName(), HuntDuration, *ActiveSearchOrigin.ToCompactString(),
		*StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(CurrentHuntTimelineTarget)));
}

void AScareDirector::EnterSearchingState()
{
	if (HasAuthority() && CurrentHuntState == EGhostHuntState::Manifestation)
	{
		SetHuntState(EGhostHuntState::Searching, TEXT("Manifestation phase completed; Demon begins searching from the Hunt origin"));

#if !UE_BUILD_SHIPPING
		if (bDebugTestScenarioRunning)
		{
			GetWorldTimerManager().SetTimer(
				DebugTestPerceptionTimerHandle,
				this,
				&AScareDirector::HandleDebugTestPlayerDetected,
				2.0f,
				false);
		}
#endif
	}
}

void AScareDirector::BeginCooldown()
{
	if (!HasAuthority())
	{
		return;
	}

	ClearHuntTimers();
	bHuntOnCooldown = true;
	SetThreatWithReason(ThreatAfterHunt, TEXT("Hunt ended and aggression was reset to ThreatAfterHunt"));
	SetHuntState(EGhostHuntState::Cooldown, TEXT("Ending phase completed; MinimumHuntCooldown begins"));

	if (MinimumHuntCooldown <= 0.0f)
	{
		FinishCooldown();
	}
	else
	{
		GetWorldTimerManager().SetTimer(
			CooldownTimerHandle,
			this,
			&AScareDirector::FinishCooldown,
			MinimumHuntCooldown,
			false);
	}

	UE_LOG(LogGhostHuntDirector, Log, TEXT("[%s] Cooldown started for %.1fs; Threat reset to %.1f."),
		*GetName(), MinimumHuntCooldown, Threat);
}

void AScareDirector::FinishCooldown()
{
	if (!HasAuthority())
	{
		return;
	}

	bHuntOnCooldown = false;
	GetWorldTimerManager().ClearTimer(CooldownTimerHandle);
	SetHuntState(EGhostHuntState::None, TEXT("MinimumHuntCooldown expired; future Hunts are allowed but not automatically started"));

	UE_LOG(LogGhostHuntDirector, Log,
		TEXT("[%s] Cooldown expired. This permits future eligibility checks; it does not start a hunt."),
		*GetName());

#if !UE_BUILD_SHIPPING
	if (bDebugTestScenarioRunning)
	{
		bDebugTestScenarioRunning = false;
		DebugTestPlayer.Reset();
		PrintHuntDebugMessage(
			TEXT("AUTOMATED HUNT TEST COMPLETE\nCooldown expired, Director returned to None, and original tuning values were restored."),
			FLinearColor(0.1f, 1.0f, 0.2f),
			10.0f);
		RestoreDebugTestTuning();
	}
#endif

	if (CanStartHunt())
	{
		ScheduleNextHuntCheck();
	}
}

void AScareDirector::ClearHuntTimers()
{
	if (!GetWorld())
	{
		return;
	}

	FTimerManager& Timers = GetWorldTimerManager();
	Timers.ClearTimer(HuntCheckTimerHandle);
	Timers.ClearTimer(OmenTimerHandle);
	Timers.ClearTimer(PostOmenTimerHandle);
	Timers.ClearTimer(ManifestationTimerHandle);
	Timers.ClearTimer(HuntDurationTimerHandle);
	Timers.ClearTimer(EndingTimerHandle);
	Timers.ClearTimer(DebugTestPerceptionTimerHandle);
}

void AScareDirector::MulticastTriggerHuntOmen_Implementation(EGhostHuntOmen Omen, EItemTimeline TargetTimeline)
{
	UE_LOG(LogGhostHuntDirector, Log, TEXT("[%s] Omen: %s (timeline %s)%s"),
		*GetName(),
		*StaticEnum<EGhostHuntOmen>()->GetNameStringByValue(static_cast<int64>(Omen)),
		*StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(TargetTimeline)),
		HasAuthority() ? TEXT(" [server]") : TEXT(" [client]"));

	OnHuntOmenTriggered.Broadcast(Omen, TargetTimeline);
	ReceiveHuntOmenTriggered(Omen, TargetTimeline);
}

void AScareDirector::MulticastFalseAlarmResolved_Implementation(EItemTimeline TargetTimeline)
{
	OnFalseAlarm.Broadcast(TargetTimeline);
	ReceiveFalseAlarm(TargetTimeline);
}

void AScareDirector::UpdateLastKnownPlayerPosition(const FVector& Position)
{
	bHasLastKnownPlayerPosition = true;
	LastKnownPlayerPosition = Position;
}

void AScareDirector::DispatchStimulus(
	EGhostHuntStimulus Stimulus,
	AActor* SubjectActor,
	AActor* InterestActor,
	const FVector& Location,
	EItemTimeline StimulusTimeline)
{
	if (!HasAuthority())
	{
		return;
	}

	OnHuntStimulus.Broadcast(Stimulus, SubjectActor, InterestActor, Location, StimulusTimeline);

	if (AActor* InterfaceTarget = ResolveHuntDemonInterfaceTarget())
	{
		IGhostHuntAIInterface::Execute_HandleHuntStimulus(
			InterfaceTarget,
			Stimulus,
			SubjectActor,
			InterestActor,
			Location,
			StimulusTimeline);
	}
}

void AScareDirector::NotifyDemonOfState(EGhostHuntState NewState)
{
	if (AActor* InterfaceTarget = ResolveHuntDemonInterfaceTarget())
	{
		IGhostHuntAIInterface::Execute_HandleHuntStateChanged(
			InterfaceTarget,
			NewState,
			CurrentHuntTimelineTarget,
			ActiveSearchOrigin,
			bHasLastKnownPlayerPosition,
			LastKnownPlayerPosition);
	}
}

AActor* AScareDirector::ResolveHuntDemonInterfaceTarget() const
{
	if (!IsValid(HuntDemon))
	{
		return nullptr;
	}

	if (HuntDemon->GetClass()->ImplementsInterface(UGhostHuntAIInterface::StaticClass()))
	{
		return HuntDemon;
	}

	if (const APawn* DemonPawn = Cast<APawn>(HuntDemon))
	{
		AController* DemonController = DemonPawn->GetController();
		if (IsValid(DemonController)
			&& DemonController->GetClass()->ImplementsInterface(UGhostHuntAIInterface::StaticClass()))
		{
			return DemonController;
		}
	}

	return nullptr;
}

FVector AScareDirector::ResolveSearchOrigin() const
{
	if (IsValid(HuntOriginActor))
	{
		return HuntOriginActor->GetActorLocation();
	}
	if (IsValid(HuntDemon))
	{
		return HuntDemon->GetActorLocation();
	}
	return GetActorLocation();
}

FString AScareDirector::GetSelectedOmensDebugString() const
{
	TArray<FString> Names;
	Names.Reserve(SelectedHuntOmens.Num());
	for (EGhostHuntOmen Omen : SelectedHuntOmens)
	{
		Names.Add(StaticEnum<EGhostHuntOmen>()->GetNameStringByValue(static_cast<int64>(Omen)));
	}
	return FString::Join(Names, TEXT(", "));
}

void AScareDirector::StartDebugScreenTimer()
{
#if !UE_BUILD_SHIPPING
	if (!GetWorld() || !HasAuthority() || !bShowHuntDebugOnScreen)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(DebugScreenTimerHandle);
	HandleDebugScreenTimer();
	GetWorldTimerManager().SetTimer(
		DebugScreenTimerHandle,
		this,
		&AScareDirector::HandleDebugScreenTimer,
		FMath::Max(0.1f, DebugScreenRefreshInterval),
		true);
#endif
}

void AScareDirector::HandleDebugScreenTimer()
{
#if !UE_BUILD_SHIPPING
	if (!bShowHuntDebugOnScreen || !HasAuthority())
	{
		return;
	}

	UKismetSystemLibrary::PrintString(
		this,
		BuildOnScreenDebugStatus(),
		true,
		false,
		GetThreatDebugColor(),
		FMath::Max(0.2f, DebugScreenRefreshInterval + 0.15f),
		TEXT("Mirrorbound_HuntDirector_Status"));
#endif
}

void AScareDirector::PrintHuntDebugMessage(
	const FString& Message,
	const FLinearColor& Color,
	float Duration,
	bool bAlsoLog) const
{
#if !UE_BUILD_SHIPPING
	if (!bShowHuntDebugOnScreen)
	{
		return;
	}

	UKismetSystemLibrary::PrintString(this, Message, true, bAlsoLog, Color, Duration);
#endif
}

FString AScareDirector::BuildOnScreenDebugStatus() const
{
#if !UE_BUILD_SHIPPING
	const FString ThreatValue = HasAuthority()
		? FString::Printf(TEXT("%.1f / %.1f"), Threat, MaxThreat)
		: TEXT("HIDDEN (server-only)");
	const FString CooldownText = bHuntOnCooldown
		? FString::Printf(TEXT("YES (%.1fs left)"), GetCooldownRemaining())
		: TEXT("NO");

	return FString::Printf(
		TEXT("MIRRORBOUND HUNT DEBUG [SERVER]\n")
		TEXT("AGGRESSION: %s | Threat: %s\n")
		TEXT("HUNT: %s | Type: %s | Timeline: %s\n")
		TEXT("COOLDOWN: %s\n")
		TEXT("Aggression reason: %s\n")
		TEXT("Hunt reason: %s\n")
		TEXT("Console: Hunt.TestScenario Both | Hunt.Status"),
		*StaticEnum<EGhostThreatState>()->GetNameStringByValue(static_cast<int64>(CurrentThreatState)),
		*ThreatValue,
		*StaticEnum<EGhostHuntState>()->GetNameStringByValue(static_cast<int64>(CurrentHuntState)),
		*StaticEnum<EGhostHuntType>()->GetNameStringByValue(static_cast<int64>(CurrentHuntType)),
		*StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(CurrentHuntTimelineTarget)),
		*CooldownText,
		*LastThreatStateChangeReason,
		*LastHuntStateChangeReason);
#else
	return FString();
#endif
}

FLinearColor AScareDirector::GetThreatDebugColor() const
{
	switch (CurrentThreatState)
	{
	case EGhostThreatState::Dormant:
		return FLinearColor(0.1f, 0.75f, 1.0f);
	case EGhostThreatState::Disturbed:
		return FLinearColor(1.0f, 0.85f, 0.1f);
	case EGhostThreatState::Manifesting:
		return FLinearColor(1.0f, 0.35f, 0.05f);
	case EGhostThreatState::HuntEligible:
		return FLinearColor(1.0f, 0.02f, 0.02f);
	default:
		return FLinearColor::White;
	}
}

void AScareDirector::BackupAndApplyDebugTestTuning()
{
#if !UE_BUILD_SHIPPING
	if (!bDebugTuningBackedUp)
	{
		DebugTuningBackup.PassiveThreatPerInterval = PassiveThreatPerInterval;
		DebugTuningBackup.OmenIntervalMin = OmenIntervalMin;
		DebugTuningBackup.OmenIntervalMax = OmenIntervalMax;
		DebugTuningBackup.PostOmenDelayMin = PostOmenDelayMin;
		DebugTuningBackup.PostOmenDelayMax = PostOmenDelayMax;
		DebugTuningBackup.ManifestationDuration = ManifestationDuration;
		DebugTuningBackup.HuntDurationMin = HuntDurationMin;
		DebugTuningBackup.HuntDurationMax = HuntDurationMax;
		DebugTuningBackup.EndingStateDuration = EndingStateDuration;
		DebugTuningBackup.MinimumHuntCooldown = MinimumHuntCooldown;
		bDebugTuningBackedUp = true;
	}

	PassiveThreatPerInterval = 0.0f;
	OmenIntervalMin = 0.4f;
	OmenIntervalMax = 0.8f;
	PostOmenDelayMin = 2.0f;
	PostOmenDelayMax = 3.0f;
	ManifestationDuration = 1.5f;
	HuntDurationMin = 12.0f;
	HuntDurationMax = 12.0f;
	EndingStateDuration = 1.0f;
	MinimumHuntCooldown = 6.0f;
#endif
}

void AScareDirector::RestoreDebugTestTuning()
{
#if !UE_BUILD_SHIPPING
	if (!bDebugTuningBackedUp)
	{
		return;
	}

	PassiveThreatPerInterval = DebugTuningBackup.PassiveThreatPerInterval;
	OmenIntervalMin = DebugTuningBackup.OmenIntervalMin;
	OmenIntervalMax = DebugTuningBackup.OmenIntervalMax;
	PostOmenDelayMin = DebugTuningBackup.PostOmenDelayMin;
	PostOmenDelayMax = DebugTuningBackup.PostOmenDelayMax;
	ManifestationDuration = DebugTuningBackup.ManifestationDuration;
	HuntDurationMin = DebugTuningBackup.HuntDurationMin;
	HuntDurationMax = DebugTuningBackup.HuntDurationMax;
	EndingStateDuration = DebugTuningBackup.EndingStateDuration;
	MinimumHuntCooldown = DebugTuningBackup.MinimumHuntCooldown;
	bDebugTuningBackedUp = false;
#endif
}

void AScareDirector::DebugRunTestScenario(EItemTimeline TargetTimeline)
{
#if !UE_BUILD_SHIPPING
	if (!HasAuthority() || !GetWorld())
	{
		PrintHuntDebugMessage(TEXT("Hunt.TestScenario must be run on the server/listen host."), FLinearColor::Red, 6.0f);
		return;
	}

	BackupAndApplyDebugTestTuning();
	ClearHuntTimers();
	GetWorldTimerManager().ClearTimer(CooldownTimerHandle);
	GetWorldTimerManager().ClearTimer(DebugTestScenarioTimerHandle);
	bHuntOnCooldown = false;
	bPendingFalseAlarm = false;
	bDebugTestScenarioRunning = true;
	DebugTestScenarioStep = 0;
	DebugTestTimeline = TargetTimeline;
	TargetedPlayer.Reset();
	DebugTestPlayer.Reset();
	SelectedHuntOmens.Reset();
	CurrentHuntType = EGhostHuntType::Organic;
	CurrentHuntTimelineTarget = TargetTimeline;

	if (CurrentHuntState != EGhostHuntState::None)
	{
		SetHuntState(EGhostHuntState::None, TEXT("Automated test scenario reset the previous Hunt state"));
	}

	SetThreatWithReason(0.0f, TEXT("TEST STEP 0: reset aggression to Dormant"));
	PrintHuntDebugMessage(
		TEXT("AUTOMATED HUNT TEST STARTED\nThe test will visit every aggression band, then run warnings, a real Hunt, Ending and Cooldown."),
		FLinearColor(0.1f, 1.0f, 0.2f),
		8.0f);

	GetWorldTimerManager().SetTimer(
		DebugTestScenarioTimerHandle,
		this,
		&AScareDirector::HandleDebugTestScenarioStep,
		FMath::Max(0.5f, DebugTestStepInterval),
		true);
#endif
}

void AScareDirector::HandleDebugTestScenarioStep()
{
#if !UE_BUILD_SHIPPING
	switch (DebugTestScenarioStep)
	{
	case 0:
		SetThreatWithReason(35.0f, TEXT("TEST STEP 1: player discovered evidence (+35 Threat)"));
		break;
	case 1:
		SetThreatWithReason(65.0f, TEXT("TEST STEP 2: dangerous activity near the cursed room (+30 Threat)"));
		break;
	case 2:
		SetThreatWithReason(HuntThreshold, TEXT("TEST STEP 3: failed ritual pushed aggression to HuntEligible"));
		CancelHuntCheck();
		break;
	case 3:
		GetWorldTimerManager().ClearTimer(DebugTestScenarioTimerHandle);
		if (!RequestTriggeredHunt(DebugTestTimeline, true, true, false))
		{
			PrintHuntDebugMessage(TEXT("TEST FAILED: Director rejected the deterministic Hunt request."), FLinearColor::Red, 8.0f);
			bDebugTestScenarioRunning = false;
			RestoreDebugTestTuning();
		}
		else
		{
			PrintHuntDebugMessage(
				TEXT("TEST STEP 4: accelerated warning/omen sequence started. A real 12-second Hunt will follow."),
				FLinearColor(1.0f, 0.2f, 0.05f),
				8.0f);
		}
		return;
	default:
		return;
	}

	++DebugTestScenarioStep;
#endif
}

void AScareDirector::HandleDebugTestPlayerDetected()
{
#if !UE_BUILD_SHIPPING
	if (!bDebugTestScenarioRunning || CurrentHuntState != EGhostHuntState::Searching || !GetWorld())
	{
		return;
	}

	AHronoCharacter* TestCharacter = nullptr;
	for (TActorIterator<AHronoCharacter> It(GetWorld()); It; ++It)
	{
		if (DoesHuntAffectTimeline(It->GetTimeline()))
		{
			TestCharacter = *It;
			break;
		}
	}

	if (!TestCharacter)
	{
		PrintHuntDebugMessage(
			TEXT("TEST NOTE: no player in the target timeline was found, so the automated Chase step was skipped."),
			FLinearColor::Yellow,
			7.0f);
		return;
	}

	DebugTestPlayer = TestCharacter;
	ReportPlayerDetected(TestCharacter, TestCharacter->GetActorLocation(), TestCharacter->GetTimeline());
	PrintHuntDebugMessage(
		TEXT("TEST AI STEP: simulated visual detection. Expected transition: Searching -> Chasing."),
		FLinearColor::Red,
		6.0f);

	GetWorldTimerManager().SetTimer(
		DebugTestPerceptionTimerHandle,
		this,
		&AScareDirector::HandleDebugTestPlayerLost,
		3.0f,
		false);
#endif
}

void AScareDirector::HandleDebugTestPlayerLost()
{
#if !UE_BUILD_SHIPPING
	if (!bDebugTestScenarioRunning || !DebugTestPlayer.IsValid())
	{
		return;
	}

	AHronoCharacter* TestCharacter = Cast<AHronoCharacter>(DebugTestPlayer.Get());
	if (!TestCharacter)
	{
		return;
	}

	ReportPlayerLost(TestCharacter, TestCharacter->GetActorLocation(), TestCharacter->GetTimeline());
	PrintHuntDebugMessage(
		TEXT("TEST AI STEP: simulated lost sight. Expected transition: Chasing -> Searching around LastKnownPlayerPosition."),
		FLinearColor(1.0f, 0.55f, 0.05f),
		7.0f);
	DebugTestPlayer.Reset();
#endif
}

void AScareDirector::SetHuntDebugOnScreenEnabled(bool bEnabled)
{
#if !UE_BUILD_SHIPPING
	bShowHuntDebugOnScreen = bEnabled;
	if (GetWorld())
	{
		GetWorldTimerManager().ClearTimer(DebugScreenTimerHandle);
	}
	if (bShowHuntDebugOnScreen)
	{
		StartDebugScreenTimer();
	}
#endif
}

void AScareDirector::DebugAddThreat(float Amount)
{
#if !UE_BUILD_SHIPPING
	AddThreatWithReason(Amount, FString::Printf(TEXT("Debug command added %.1f Threat"), Amount));
#endif
}

void AScareDirector::DebugForceHuntEligible()
{
#if !UE_BUILD_SHIPPING
	SetThreatWithReason(HuntThreshold, TEXT("Debug command forced HuntEligible"));
#endif
}

void AScareDirector::DebugForceHunt(EItemTimeline TargetTimeline, bool bSkipWarning)
{
#if !UE_BUILD_SHIPPING
	RequestTriggeredHunt(TargetTimeline, !bSkipWarning, true, false);
#endif
}

void AScareDirector::DebugEndHunt()
{
#if !UE_BUILD_SHIPPING
	EndHunt();
#endif
}

void AScareDirector::DebugTriggerOmen(EGhostHuntOmen Omen)
{
#if !UE_BUILD_SHIPPING
	if (HasAuthority())
	{
		MulticastTriggerHuntOmen(Omen, CurrentHuntTimelineTarget);
	}
#endif
}

FString AScareDirector::GetDebugStatus() const
{
#if !UE_BUILD_SHIPPING
	return FString::Printf(
		TEXT("Threat=%.1f/%.1f ThreatState=%s HuntState=%s HuntType=%s Cooldown=%s(%.1fs) Timeline=%s Omens=[%s] SearchOrigin=%s LastKnown=%s%s ThreatReason='%s' HuntReason='%s' TestRunning=%s"),
		HasAuthority() ? Threat : -1.0f,
		MaxThreat,
		*StaticEnum<EGhostThreatState>()->GetNameStringByValue(static_cast<int64>(CurrentThreatState)),
		*StaticEnum<EGhostHuntState>()->GetNameStringByValue(static_cast<int64>(CurrentHuntState)),
		*StaticEnum<EGhostHuntType>()->GetNameStringByValue(static_cast<int64>(CurrentHuntType)),
		bHuntOnCooldown ? TEXT("true") : TEXT("false"),
		GetCooldownRemaining(),
		*StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(CurrentHuntTimelineTarget)),
		*GetSelectedOmensDebugString(),
		*ActiveSearchOrigin.ToCompactString(),
		bHasLastKnownPlayerPosition ? TEXT("") : TEXT("unset/"),
		*LastKnownPlayerPosition.ToCompactString(),
		*LastThreatStateChangeReason,
		*LastHuntStateChangeReason,
		bDebugTestScenarioRunning ? TEXT("true") : TEXT("false"));
#else
	return TEXT("Hunt debugging is disabled in Shipping builds.");
#endif
}

float AScareDirector::GetCooldownRemaining() const
{
#if !UE_BUILD_SHIPPING
	if (GetWorld() && bHuntOnCooldown)
	{
		return FMath::Max(0.0f, GetWorldTimerManager().GetTimerRemaining(CooldownTimerHandle));
	}
#endif
	return 0.0f;
}

#if !UE_BUILD_SHIPPING
namespace HuntDirectorDebugCommands
{
	AScareDirector* FindDirector(UWorld* World)
	{
		return AScareDirector::GetHuntDirector(World);
	}

	EItemTimeline ParseTimeline(const TArray<FString>& Args)
	{
		if (Args.Num() > 0)
		{
			if (Args[0].Equals(TEXT("Past"), ESearchCase::IgnoreCase))
			{
				return EItemTimeline::Past;
			}
			if (Args[0].Equals(TEXT("Future"), ESearchCase::IgnoreCase))
			{
				return EItemTimeline::Future;
			}
		}
		return EItemTimeline::Both;
	}

	void AddThreat(const TArray<FString>& Args, UWorld* World)
	{
		if (AScareDirector* Director = FindDirector(World))
		{
			Director->DebugAddThreat(Args.Num() > 0 ? FCString::Atof(*Args[0]) : 10.0f);
		}
	}

	void ForceEligible(const TArray<FString>& Args, UWorld* World)
	{
		if (AScareDirector* Director = FindDirector(World))
		{
			Director->DebugForceHuntEligible();
		}
	}

	void ForceHunt(const TArray<FString>& Args, UWorld* World)
	{
		if (AScareDirector* Director = FindDirector(World))
		{
			const bool bSkipWarning = Args.ContainsByPredicate([](const FString& Arg)
			{
				return Arg.Equals(TEXT("SkipWarning"), ESearchCase::IgnoreCase);
			});
			Director->DebugForceHunt(ParseTimeline(Args), bSkipWarning);
		}
	}

	void EndHunt(const TArray<FString>& Args, UWorld* World)
	{
		if (AScareDirector* Director = FindDirector(World))
		{
			Director->DebugEndHunt();
		}
	}

	void TestScenario(const TArray<FString>& Args, UWorld* World)
	{
		if (AScareDirector* Director = FindDirector(World))
		{
			Director->DebugRunTestScenario(ParseTimeline(Args));
		}
	}

	void ScreenDebug(const TArray<FString>& Args, UWorld* World)
	{
		if (AScareDirector* Director = FindDirector(World))
		{
			const bool bEnable = Args.IsEmpty()
				|| (!Args[0].Equals(TEXT("0")) && !Args[0].Equals(TEXT("false"), ESearchCase::IgnoreCase));
			Director->SetHuntDebugOnScreenEnabled(bEnable);
		}
	}

	void TriggerOmen(const TArray<FString>& Args, UWorld* World)
	{
		if (Args.IsEmpty())
		{
			UE_LOG(LogGhostHuntDirector, Warning, TEXT("Usage: Hunt.TriggerOmen <OmenName>"));
			return;
		}

		const UEnum* OmenEnum = StaticEnum<EGhostHuntOmen>();
		for (int32 Index = 0; Index < OmenEnum->NumEnums(); ++Index)
		{
			if (OmenEnum->GetNameStringByIndex(Index).Equals(Args[0], ESearchCase::IgnoreCase))
			{
				if (AScareDirector* Director = FindDirector(World))
				{
					Director->DebugTriggerOmen(static_cast<EGhostHuntOmen>(OmenEnum->GetValueByIndex(Index)));
				}
				return;
			}
		}

		UE_LOG(LogGhostHuntDirector, Warning, TEXT("Unknown Hunt Omen '%s'."), *Args[0]);
	}

	void Status(const TArray<FString>& Args, UWorld* World)
	{
		if (AScareDirector* Director = FindDirector(World))
		{
			UE_LOG(LogGhostHuntDirector, Display, TEXT("%s"), *Director->GetDebugStatus());
		}
	}

	FAutoConsoleCommandWithWorldAndArgs AddThreatCommand(
		TEXT("Hunt.AddThreat"),
		TEXT("Hunt.AddThreat [Amount] - adds hidden server Threat."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&AddThreat));

	FAutoConsoleCommandWithWorldAndArgs ForceEligibleCommand(
		TEXT("Hunt.ForceEligible"),
		TEXT("Sets hidden server Threat to HuntThreshold."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&ForceEligible));

	FAutoConsoleCommandWithWorldAndArgs ForceHuntCommand(
		TEXT("Hunt.Force"),
		TEXT("Hunt.Force [Past|Future|Both] [SkipWarning] - forces a triggered hunt."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&ForceHunt));

	FAutoConsoleCommandWithWorldAndArgs EndHuntCommand(
		TEXT("Hunt.End"),
		TEXT("Ends the active hunt and starts cooldown."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&EndHunt));

	FAutoConsoleCommandWithWorldAndArgs TriggerOmenCommand(
		TEXT("Hunt.TriggerOmen"),
		TEXT("Hunt.TriggerOmen <OmenName> - multicasts one omen."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&TriggerOmen));

	FAutoConsoleCommandWithWorldAndArgs TestScenarioCommand(
		TEXT("Hunt.TestScenario"),
		TEXT("Hunt.TestScenario [Past|Future|Both] - runs an accelerated complete Hunt demonstration."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&TestScenario));

	FAutoConsoleCommandWithWorldAndArgs ScreenDebugCommand(
		TEXT("Hunt.DebugScreen"),
		TEXT("Hunt.DebugScreen [1|0] - enables or disables the on-screen Hunt debug panel."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&ScreenDebug));

	FAutoConsoleCommandWithWorldAndArgs StatusCommand(
		TEXT("Hunt.Status"),
		TEXT("Logs Threat, states, cooldown, omens, timeline, and last-known position."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Status));
}
#endif
