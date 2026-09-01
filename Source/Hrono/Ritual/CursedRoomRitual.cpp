#include "Ritual/CursedRoomRitual.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Curves/CurveFloat.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Enviroment/Room.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "HronoCharacter.h"
#include "Items/Base_Item.h"
#include "Items/RitualCandle.h"
#include "Items/RitualGoatSkull.h"
#include "Net/UnrealNetwork.h"
#include "Ritual/RitualSymbolVisual.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogCursedRoomRitual, Log, All);

ACursedRoomRitual::ACursedRoomRitual()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(false);
	SetNetUpdateFrequency(10.0f);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

void ACursedRoomRitual::BeginPlay()
{
	Super::BeginPlay();

	int32 RitualCount = 0;
	for (TActorIterator<ACursedRoomRitual> It(GetWorld()); It; ++It)
	{
		++RitualCount;
	}
	if (RitualCount > 1)
	{
		UE_LOG(LogCursedRoomRitual, Warning,
			TEXT("[%s] %d cursed-room ritual coordinators exist. Place exactly one."),
			*GetName(), RitualCount);
	}

	NormalizeVisibleSlots();
	ApplyReplicatedState();
}

void ACursedRoomRitual::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorldTimerManager().ClearTimer(StateTimerHandle);
		GetWorldTimerManager().ClearTimer(VisualTimerHandle);
		GetWorldTimerManager().ClearTimer(LocalClueRetryTimerHandle);
	}
	ClearLocalSymbolVisuals();
	Super::EndPlay(EndPlayReason);
}

void ACursedRoomRitual::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACursedRoomRitual, ReplicatedState);
}

ACursedRoomRitual* ACursedRoomRitual::FindRitual(const UObject* WorldContextObject)
{
	if (!IsValid(WorldContextObject))
	{
		return nullptr;
	}

	const UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<ACursedRoomRitual> It(World); It; ++It)
	{
		return *It;
	}
	return nullptr;
}

bool ACursedRoomRitual::TryActivateRitual(
	AHronoCharacter* ActivatingCharacter,
	ARitualCandle* Candle)
{
	if (!HasAuthority()
		|| ReplicatedState.State != ECursedRoomRitualState::Idle
		|| !IsValid(ActivatingCharacter)
		|| !IsValid(Candle)
		|| ActivatingCharacter->GetTimeline() != EItemTimeline::Future
		|| FVector::DistSquared(
			ActivatingCharacter->GetActorLocation(),
			GetRitualItemWorldTransform(Candle).GetLocation())
			> FMath::Square(FMath::Max(1.0f, ActivationDistance))
		|| !IsDroppedRitualItemValid(Candle, EItemTimeline::Future))
	{
		return false;
	}

	ARoom* CandleRoom = FindContainingRoom(Candle);
	ARitualGoatSkull* Skull = FindAvailableSkullInRoom(CandleRoom);
	if (!IsValid(CandleRoom) || !IsValid(Skull))
	{
		if (bDebugRitual)
		{
			UE_LOG(LogCursedRoomRitual, Log,
				TEXT("[%s] Activation not ready. CandleRoom=%s SkullInSameRoom=%s"),
				*GetName(), *GetNameSafe(CandleRoom), *GetNameSafe(Skull));
		}
		return false;
	}

	if (CandleRoom->IsCursed())
	{
		StartCorrectRitual(CandleRoom, Skull, Candle);
	}
	else
	{
		StartWrongRoomRitual(CandleRoom, Skull, Candle);
	}
	return true;
}

TArray<FString> ACursedRoomRitual::GetRitualCode() const
{
	return HasAuthority() && ReplicatedState.State == ECursedRoomRitualState::Completed
		? FullCode
		: TArray<FString>();
}

bool ACursedRoomRitual::ValidateRitualCode(const TArray<FString>& InputSymbols) const
{
	if (!HasAuthority()
		|| ReplicatedState.State != ECursedRoomRitualState::Completed
		|| InputSymbols.Num() != FullCode.Num())
	{
		return false;
	}

	for (int32 Index = 0; Index < FullCode.Num(); ++Index)
	{
		if (!InputSymbols[Index].Equals(FullCode[Index], ESearchCase::CaseSensitive))
		{
			return false;
		}
	}
	return true;
}

void ACursedRoomRitual::DeliverCompleteClueToCharacter(
	AHronoCharacter* Character,
	int32 RequestedSequenceId)
{
	if (!HasAuthority()
		|| !IsValid(Character)
		|| ReplicatedState.State != ECursedRoomRitualState::Completed
		|| RequestedSequenceId != ReplicatedState.SequenceId
		|| FullCode.Num() != NumberOfSymbols)
	{
		return;
	}

	const EItemTimeline Timeline = Character->GetTimeline();
	const TArray<int32>* VisibleSlots = Timeline == EItemTimeline::Past
		? &PastVisibleSlots
		: (Timeline == EItemTimeline::Future ? &FutureVisibleSlots : nullptr);
	if (!VisibleSlots)
	{
		return;
	}

	for (const int32 SlotIndex : *VisibleSlots)
	{
		if (FullCode.IsValidIndex(SlotIndex))
		{
			Character->ClientReceiveCursedRoomRitualSymbol(
				this,
				ReplicatedState.SequenceId,
				SlotIndex,
				FullCode[SlotIndex],
				Timeline,
				MakeSymbolTransform(SlotIndex, Timeline));
		}
	}
}

void ACursedRoomRitual::ReceiveSymbolForLocalPlayer(
	int32 SequenceId,
	int32 SlotIndex,
	const FString& Symbol,
	EItemTimeline ClueTimeline,
	const FTransform& SymbolTransform)
{
	if (SequenceId != ReplicatedState.SequenceId || SlotIndex < 0)
	{
		return;
	}

	if (LocalVisibleSequenceId != SequenceId)
	{
		ClearLocalSymbolVisuals();
		LocalVisibleSequenceId = SequenceId;
		LocalReceivedSlots.Reset();
		LocalVisibleClue.Init(TEXT("?"), FMath::Max(NumberOfSymbols, SlotIndex + 1));
	}
	else if (LocalVisibleClue.Num() <= SlotIndex)
	{
		const int32 PreviousNum = LocalVisibleClue.Num();
		LocalVisibleClue.SetNum(SlotIndex + 1);
		for (int32 Index = PreviousNum; Index < LocalVisibleClue.Num(); ++Index)
		{
			LocalVisibleClue[Index] = TEXT("?");
		}
	}

	LocalVisibleClue[SlotIndex] = Symbol;
	if (LocalReceivedSlots.Contains(SlotIndex))
	{
		return;
	}
	LocalReceivedSlots.Add(SlotIndex);

	if (SymbolDecalClass && GetWorld())
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		if (ARitualSymbolVisual* SymbolActor = GetWorld()->SpawnActor<ARitualSymbolVisual>(
			SymbolDecalClass,
			SymbolTransform,
			SpawnParameters))
		{
			SymbolActor->InitializeSymbol(SlotIndex, Symbol, ClueTimeline);
			LocalSymbolVisuals.Add(SymbolActor);
		}
	}

	BP_OnSymbolRevealed(SlotIndex, Symbol, ClueTimeline, SymbolTransform);
}

FString ACursedRoomRitual::GetRitualDebugStatus() const
{
	const FString StateName = StaticEnum<ECursedRoomRitualState>()->GetNameStringByValue(
		static_cast<int64>(ReplicatedState.State));
	if (!HasAuthority())
	{
		return FString::Printf(
			TEXT("Ritual=%s Sequence=%d LocalClue=[%s] (answer and cursed room are server-only)"),
			*StateName,
			ReplicatedState.SequenceId,
			*FString::Join(LocalVisibleClue, TEXT(" ")));
	}

	return FString::Printf(
		TEXT("Ritual=%s Sequence=%d SkullRoom=%s CandleRoom=%s TestedRoom=%s Cursed=%s Code=[%s] Past=[%s] Future=[%s]"),
		*StateName,
		ReplicatedState.SequenceId,
		*GetNameSafe(FindContainingRoom(ReplicatedState.GoatSkull)),
		*GetNameSafe(FindContainingRoom(ReplicatedState.RitualCandle)),
		*GetNameSafe(ActiveTestedRoom),
		IsValid(ActiveTestedRoom) && ActiveTestedRoom->IsCursed() ? TEXT("true") : TEXT("false"),
		*FString::Join(FullCode, TEXT(" ")),
		*BuildMaskedCode(EItemTimeline::Past),
		*BuildMaskedCode(EItemTimeline::Future));
}

void ACursedRoomRitual::OnRep_RitualState()
{
	ApplyReplicatedState();
}

ARoom* ACursedRoomRitual::FindContainingRoom(const AActor* Actor) const
{
	if (!IsValid(Actor) || !GetWorld())
	{
		return nullptr;
	}

	for (TActorIterator<ARoom> It(GetWorld()); It; ++It)
	{
		ARoom* Room = *It;
		if (IsValid(Room) && IsValid(Room->RoomVolume) && Room->RoomVolume->IsOverlappingActor(Actor))
		{
			return Room;
		}
	}
	return nullptr;
}

ARitualGoatSkull* ACursedRoomRitual::FindAvailableSkullInRoom(const ARoom* Room) const
{
	if (!IsValid(Room) || !GetWorld())
	{
		return nullptr;
	}

	ARitualGoatSkull* ClosestSkull = nullptr;
	float ClosestDistanceSquared = TNumericLimits<float>::Max();
	for (TActorIterator<ARitualGoatSkull> It(GetWorld()); It; ++It)
	{
		ARitualGoatSkull* Skull = *It;
		if (!IsDroppedRitualItemValid(Skull, EItemTimeline::Past)
			|| FindContainingRoom(Skull) != Room)
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(
			GetRitualItemWorldTransform(Skull).GetLocation(),
			Room->GetActorLocation());
		if (DistanceSquared < ClosestDistanceSquared)
		{
			ClosestDistanceSquared = DistanceSquared;
			ClosestSkull = Skull;
		}
	}
	return ClosestSkull;
}

bool ACursedRoomRitual::IsDroppedRitualItemValid(
	const ABase_Item* Item,
	EItemTimeline RequiredTimeline) const
{
	return IsValid(Item)
		&& !Item->bIsPickedUp
		&& !IsValid(Item->OwningCharacter)
		&& Item->ItemTimeline == RequiredTimeline;
}

void ACursedRoomRitual::StartCorrectRitual(
	ARoom* TestedRoom,
	ARitualGoatSkull* Skull,
	ARitualCandle* Candle)
{
	ActiveTestedRoom = TestedRoom;
	InitialSkullTransform = GetRitualItemWorldTransform(Skull);
	ReplicatedState.GoatSkull = Skull;
	ReplicatedState.RitualCandle = Candle;
	++ReplicatedState.SequenceId;
	RevealedSymbolCount = 0;
	GenerateCodeOnServer();

	FHitResult CeilingHit;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CursedRoomRitualCeiling), false);
	QueryParams.AddIgnoredActor(Skull);
	QueryParams.AddIgnoredActor(Candle);
	const FVector TraceStart = InitialSkullTransform.GetLocation();
	const FVector TraceEnd = TraceStart + FVector::UpVector * FMath::Max(FloatHeight, CeilingTraceDistance);
	const bool bHitCeiling = GetWorld()->LineTraceSingleByChannel(
		CeilingHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
	const bool bUseCeilingHit = bHitCeiling && CeilingHit.Distance >= FMath::Max(0.0f, MinimumCeilingRise);

	CeilingSurfaceNormal = bUseCeilingHit ? CeilingHit.ImpactNormal.GetSafeNormal() : FVector::DownVector;
	const FVector CeilingSurfaceCenter = bUseCeilingHit
		? CeilingHit.ImpactPoint
		: TraceStart + FVector::UpVector * FloatHeight;

	UE_LOG(LogCursedRoomRitual, Log,
		TEXT("[RitualCeiling][Sequence=%d] Start=%s End=%s Hit=%s Accepted=%s Actor=%s Component=%s Distance=%.1f TargetSurface=%s Normal=%s"),
		ReplicatedState.SequenceId,
		*TraceStart.ToCompactString(),
		*TraceEnd.ToCompactString(),
		bHitCeiling ? TEXT("true") : TEXT("false"),
		bUseCeilingHit ? TEXT("true") : TEXT("false"),
		*GetNameSafe(CeilingHit.GetActor()),
		*GetNameSafe(CeilingHit.GetComponent()),
		bHitCeiling ? CeilingHit.Distance : -1.0f,
		*CeilingSurfaceCenter.ToCompactString(),
		*CeilingSurfaceNormal.ToCompactString());
	if (bHitCeiling && !bUseCeilingHit)
	{
		UE_LOG(LogCursedRoomRitual, Warning,
			TEXT("[RitualCeiling][Sequence=%d] Rejected nearby hit %.1fcm (< MinimumCeilingRise %.1fcm); using FloatHeight %.1fcm."),
			ReplicatedState.SequenceId,
			CeilingHit.Distance,
			MinimumCeilingRise,
			FloatHeight);
	}

	ScratchWorldDirection = InitialSkullTransform.TransformVectorNoScale(ScratchDirectionLocal);
	ScratchWorldDirection = FVector::VectorPlaneProject(ScratchWorldDirection, CeilingSurfaceNormal).GetSafeNormal();
	if (ScratchWorldDirection.IsNearlyZero())
	{
		ScratchWorldDirection = FVector::VectorPlaneProject(FVector::RightVector, CeilingSurfaceNormal).GetSafeNormal();
	}
	ReplicatedState.ScratchDirection = ScratchWorldDirection;

	const float ScratchLength = FMath::Max(0, NumberOfSymbols - 1) * ScratchSymbolSpacing;
	CeilingSurfaceStart = CeilingSurfaceCenter - ScratchWorldDirection * (ScratchLength * 0.5f);
	const FVector SkullCeilingLocation = CeilingSurfaceCenter + CeilingSurfaceNormal * CeilingClearance;
	const FQuat CeilingRotation = (InitialSkullTransform.Rotator() + SkullCeilingRotationOffset).Quaternion();

	ScratchStartTransform = InitialSkullTransform;
	ScratchStartTransform.SetLocation(SkullCeilingLocation);
	ScratchStartTransform.SetRotation(CeilingRotation);
	ScratchEndTransform = ScratchStartTransform;

	Skull->SetRitualLocked(true);
	Candle->SetRitualLocked(true);
	const float SafeReactionMin = FMath::Max(0.0f, RitualStartReactionDuration);
	const float SafeReactionMax = FMath::Max(SafeReactionMin, RitualStartReactionDurationMax);
	const float SelectedReactionDuration = FMath::FRandRange(SafeReactionMin, SafeReactionMax);
	SetState(
		ECursedRoomRitualState::Reacting,
		SelectedReactionDuration,
		InitialSkullTransform,
		InitialSkullTransform);

	UE_LOG(LogCursedRoomRitual, Log,
		TEXT("[%s] Correct room ritual started in %s. Code remains server-only."),
		*GetName(), *GetNameSafe(TestedRoom));
}

void ACursedRoomRitual::StartWrongRoomRitual(
	ARoom* TestedRoom,
	ARitualGoatSkull* Skull,
	ARitualCandle* Candle)
{
	ActiveTestedRoom = TestedRoom;
	InitialSkullTransform = GetRitualItemWorldTransform(Skull);
	ReplicatedState.GoatSkull = Skull;
	ReplicatedState.RitualCandle = Candle;
	++ReplicatedState.SequenceId;
	FullCode.Reset();
	RevealedSymbolCount = 0;

	Skull->SetRitualLocked(true);
	Candle->SetRitualLocked(true);
	SetState(
		ECursedRoomRitualState::Failed,
		WrongRoomReactionDuration,
		InitialSkullTransform,
		InitialSkullTransform);

	OnWrongRoomRitual.Broadcast(TestedRoom, Skull, Candle);
	UE_LOG(LogCursedRoomRitual, Log,
		TEXT("[%s] Ritual produced an environmental failure reaction in room %s."),
		*GetName(), *GetNameSafe(TestedRoom));
}

void ACursedRoomRitual::GenerateCodeOnServer()
{
	if (!HasAuthority())
	{
		return;
	}

	NumberOfSymbols = FMath::Clamp(NumberOfSymbols, 1, 16);
	NormalizeVisibleSlots();
	if (AvailableSymbols.IsEmpty())
	{
		AvailableSymbols = { TEXT("0"), TEXT("1"), TEXT("2"), TEXT("3"), TEXT("4"),
			TEXT("5"), TEXT("6"), TEXT("7"), TEXT("8"), TEXT("9") };
	}

	FullCode.Reset(NumberOfSymbols);
	for (int32 SlotIndex = 0; SlotIndex < NumberOfSymbols; ++SlotIndex)
	{
		FullCode.Add(AvailableSymbols[FMath::RandHelper(AvailableSymbols.Num())]);
	}

	if (bDebugRitual)
	{
		UE_LOG(LogCursedRoomRitual, Log, TEXT("[%s] %s"), *GetName(), *GetRitualDebugStatus());
	}
}

void ACursedRoomRitual::NormalizeVisibleSlots()
{
	NumberOfSymbols = FMath::Clamp(NumberOfSymbols, 1, 16);
	TSet<int32> AssignedSlots;
	TArray<int32> CleanPast;
	TArray<int32> CleanFuture;

	for (const int32 Slot : PastVisibleSlots)
	{
		if (Slot >= 0 && Slot < NumberOfSymbols && !AssignedSlots.Contains(Slot))
		{
			CleanPast.Add(Slot);
			AssignedSlots.Add(Slot);
		}
	}
	for (const int32 Slot : FutureVisibleSlots)
	{
		if (Slot >= 0 && Slot < NumberOfSymbols && !AssignedSlots.Contains(Slot))
		{
			CleanFuture.Add(Slot);
			AssignedSlots.Add(Slot);
		}
	}
	for (int32 Slot = 0; Slot < NumberOfSymbols; ++Slot)
	{
		if (!AssignedSlots.Contains(Slot))
		{
			(Slot % 2 == 0 ? CleanPast : CleanFuture).Add(Slot);
		}
	}

	PastVisibleSlots = MoveTemp(CleanPast);
	FutureVisibleSlots = MoveTemp(CleanFuture);
}

void ACursedRoomRitual::SetState(
	ECursedRoomRitualState NewState,
	float Duration,
	const FTransform& Start,
	const FTransform& Target)
{
	if (!HasAuthority())
	{
		return;
	}

	const ECursedRoomRitualState PreviousState = ReplicatedState.State;
	ReplicatedState.State = NewState;
	ReplicatedState.StartServerTime = GetSynchronizedServerTime();
	ReplicatedState.Duration = FMath::Max(0.0f, Duration);
	ReplicatedState.SkullStartTransform = Start;
	ReplicatedState.SkullTargetTransform = Target;

	GetWorldTimerManager().ClearTimer(StateTimerHandle);
	if (ReplicatedState.Duration > 0.0f)
	{
		GetWorldTimerManager().SetTimer(
			StateTimerHandle,
			this,
			&ACursedRoomRitual::HandleStateFinished,
			ReplicatedState.Duration,
			false);
	}

	ApplyReplicatedState();
	ForceNetUpdate();
	LogStageEntered(PreviousState);

	UE_LOG(LogCursedRoomRitual, Log,
		TEXT("[%s] State %s -> %s (sequence %d, %.2fs)"),
		*GetName(),
		*StaticEnum<ECursedRoomRitualState>()->GetNameStringByValue(static_cast<int64>(PreviousState)),
		*StaticEnum<ECursedRoomRitualState>()->GetNameStringByValue(static_cast<int64>(NewState)),
		ReplicatedState.SequenceId,
		ReplicatedState.Duration);
}

void ACursedRoomRitual::HandleStateFinished()
{
	if (!HasAuthority() || !IsValid(ReplicatedState.GoatSkull))
	{
		return;
	}

	UpdateRitualVisuals();
	const FTransform CurrentTransform = GetRitualItemWorldTransform(ReplicatedState.GoatSkull);
	LogStageFinished(ReplicatedState.State);
	switch (ReplicatedState.State)
	{
	case ECursedRoomRitualState::Reacting:
		SetState(ECursedRoomRitualState::Floating, FloatDuration, CurrentTransform, ScratchStartTransform);
		break;
	case ECursedRoomRitualState::Floating:
		SetState(ECursedRoomRitualState::Scratching, ScratchDuration, CurrentTransform, ScratchEndTransform);
		break;
	case ECursedRoomRitualState::Rotating:
		// Recovery path for an older replicated PIE state. New rituals skip this pause.
		SetState(ECursedRoomRitualState::Scratching, ScratchDuration, CurrentTransform, ScratchEndTransform);
		break;
	case ECursedRoomRitualState::Scratching:
		RevealSymbolsUpTo(NumberOfSymbols);
		SetState(ECursedRoomRitualState::Completed, 0.0f, CurrentTransform, CurrentTransform);
		break;
	case ECursedRoomRitualState::Failed:
		SetRitualItemWorldTransform(ReplicatedState.GoatSkull, InitialSkullTransform);
		SetState(ECursedRoomRitualState::Idle, 0.0f, InitialSkullTransform, InitialSkullTransform);
		break;
	default:
		break;
	}
}

void ACursedRoomRitual::ApplyReplicatedState()
{
	const ECursedRoomRitualState PreviousState = LastDispatchedState;
	DispatchStateEvents(PreviousState);

	if (ReplicatedState.State == ECursedRoomRitualState::Reacting
		|| ReplicatedState.State == ECursedRoomRitualState::Floating
		|| ReplicatedState.State == ECursedRoomRitualState::Rotating
		|| ReplicatedState.State == ECursedRoomRitualState::Scratching
		|| ReplicatedState.State == ECursedRoomRitualState::Failed)
	{
		StartVisualUpdates();
		UpdateRitualVisuals();
	}
	else
	{
		StopVisualUpdates();
	}

	if (ReplicatedState.State == ECursedRoomRitualState::Completed && !HasAuthority())
	{
		RequestCompleteClueForLocalPlayer();
	}
}

void ACursedRoomRitual::DispatchStateEvents(ECursedRoomRitualState PreviousState)
{
	if (LastDispatchedSequenceId == ReplicatedState.SequenceId
		&& LastDispatchedState == ReplicatedState.State)
	{
		return;
	}

	LastDispatchedSequenceId = ReplicatedState.SequenceId;
	LastDispatchedState = ReplicatedState.State;
	ARitualGoatSkull* Skull = ReplicatedState.GoatSkull;
	ARitualCandle* Candle = ReplicatedState.RitualCandle;

	switch (ReplicatedState.State)
	{
	case ECursedRoomRitualState::Idle:
		if (IsValid(Skull))
		{
			Skull->SetRitualLocked(false);
		}
		if (IsValid(Candle))
		{
			Candle->SetRitualLocked(false);
		}
		break;
	case ECursedRoomRitualState::Reacting:
		ClearLocalSymbolVisuals();
		if (IsValid(Skull))
		{
			Skull->SetRitualLocked(true);
		}
		if (IsValid(Candle))
		{
			Candle->SetRitualLocked(true);
			Candle->BP_OnCandleIgnited();
			Candle->BP_OnCandleFlameIncreased();
		}
		BP_OnRitualStarted(Skull, Candle);
		break;
	case ECursedRoomRitualState::Floating:
		if (IsValid(Skull))
		{
			Skull->BP_OnSkullStartFloating();
		}
		BP_OnSkullStartFloating(Skull);
		break;
	case ECursedRoomRitualState::Rotating:
		if (IsValid(Skull))
		{
			Skull->BP_OnSkullStartRotating();
		}
		break;
	case ECursedRoomRitualState::Scratching:
		if (IsValid(Skull))
		{
			Skull->BP_OnSkullStartScratching();
		}
		BP_OnSkullStartScratching(Skull);
		break;
	case ECursedRoomRitualState::Completed:
		if (IsValid(Skull))
		{
			Skull->SetRitualLocked(false);
			Skull->BP_OnSkullRitualCompleted();
		}
		if (IsValid(Candle))
		{
			Candle->SetRitualLocked(false);
			Candle->BP_OnCandleRitualCompleted();
		}
		OnRitualCompleted.Broadcast();
		BP_OnRitualCompleted();
		break;
	case ECursedRoomRitualState::Failed:
		ClearLocalSymbolVisuals();
		if (IsValid(Skull))
		{
			Skull->SetRitualLocked(true);
			Skull->BP_OnWrongRoomReaction();
		}
		if (IsValid(Candle))
		{
			Candle->SetRitualLocked(true);
			Candle->BP_OnWrongRoomReaction();
			Candle->BP_OnCandleExtinguished();
		}
		BP_OnWrongRoom(HasAuthority() ? ActiveTestedRoom.Get() : nullptr, Skull, Candle);
		break;
	}

	OnRitualStateChanged.Broadcast(PreviousState, ReplicatedState.State);
	if (bDebugRitual)
	{
		UE_LOG(LogCursedRoomRitual, Log, TEXT("[%s] %s"), *GetName(), *GetRitualDebugStatus());
	}
}

void ACursedRoomRitual::StartVisualUpdates()
{
	if (!GetWorldTimerManager().IsTimerActive(VisualTimerHandle))
	{
		GetWorldTimerManager().SetTimer(
			VisualTimerHandle,
			this,
			&ACursedRoomRitual::UpdateRitualVisuals,
			FMath::Max(0.01f, VisualUpdateInterval),
			true);
	}
}

void ACursedRoomRitual::UpdateRitualVisuals()
{
	ARitualGoatSkull* Skull = ReplicatedState.GoatSkull;
	if (!IsValid(Skull) || ReplicatedState.Duration <= 0.0f)
	{
		return;
	}

	const float LinearAlpha = FMath::Clamp(
		static_cast<float>((GetSynchronizedServerTime() - ReplicatedState.StartServerTime)
			/ ReplicatedState.Duration),
		0.0f,
		1.0f);

	if (ReplicatedState.State == ECursedRoomRitualState::Reacting)
	{
		const float ShakePhase = LinearAlpha * FMath::Max(1.0f, CorrectRoomShakeCycles) * 2.0f * PI;
		const float ShakeEnvelope = FMath::Sin(LinearAlpha * PI);
		const float LateralShake = FMath::Sin(ShakePhase) * CorrectRoomShakeDistance * ShakeEnvelope;
		const float ForwardShake = FMath::Sin(ShakePhase * 1.7f)
			* CorrectRoomShakeDistance * 0.35f * ShakeEnvelope;
		FTransform ShakenTransform = ReplicatedState.SkullStartTransform;
		const FVector Right = ReplicatedState.SkullStartTransform.TransformVectorNoScale(
			FVector::RightVector).GetSafeNormal();
		const FVector Forward = ReplicatedState.SkullStartTransform.TransformVectorNoScale(
			FVector::ForwardVector).GetSafeNormal();
		ShakenTransform.AddToTranslation(Right * LateralShake + Forward * ForwardShake);
		SetRitualItemWorldTransform(Skull, ShakenTransform);
	}
	else if (ReplicatedState.State == ECursedRoomRitualState::Failed)
	{
		const float Shake = FMath::Sin(LinearAlpha * WrongRoomShakeCycles * 2.0f * PI)
			* WrongRoomShakeDistance * (1.0f - LinearAlpha);
		FTransform ShakenTransform = ReplicatedState.SkullStartTransform;
		const FVector ShakeDirection = ReplicatedState.SkullStartTransform.TransformVectorNoScale(
			FVector::RightVector).GetSafeNormal();
		ShakenTransform.AddToTranslation(ShakeDirection * Shake);
		SetRitualItemWorldTransform(Skull, ShakenTransform);
	}
	else if (ReplicatedState.State == ECursedRoomRitualState::Scratching)
	{
		const float ScratchPhase = LinearAlpha * FMath::Max(0.5f, ScratchSideToSideCycles) * 2.0f * PI;
		FTransform ScratchTransform = ReplicatedState.SkullStartTransform;
		ScratchTransform.SetLocation(
			ReplicatedState.SkullStartTransform.GetLocation()
			+ ReplicatedState.ScratchDirection.GetSafeNormal()
				* (FMath::Sin(ScratchPhase) * ScratchSideToSideDistance));
		SetRitualItemWorldTransform(Skull, ScratchTransform);
	}
	else
	{
		const float MoveAlpha = EvaluateMoveAlpha(LinearAlpha);
		FTransform NewTransform;
		NewTransform.SetLocation(FMath::Lerp(
			ReplicatedState.SkullStartTransform.GetLocation(),
			ReplicatedState.SkullTargetTransform.GetLocation(),
			MoveAlpha));
		NewTransform.SetRotation(FQuat::Slerp(
			ReplicatedState.SkullStartTransform.GetRotation(),
			ReplicatedState.SkullTargetTransform.GetRotation(),
			MoveAlpha));
		NewTransform.SetScale3D(ReplicatedState.SkullStartTransform.GetScale3D());
		SetRitualItemWorldTransform(Skull, NewTransform);
	}

	if (HasAuthority() && ReplicatedState.State == ECursedRoomRitualState::Scratching)
	{
		const int32 DesiredCount = FMath::Clamp(
			FMath::FloorToInt(LinearAlpha * NumberOfSymbols) + 1,
			0,
			NumberOfSymbols);
		RevealSymbolsUpTo(DesiredCount);
	}

	if (LinearAlpha >= 1.0f && !HasAuthority())
	{
		StopVisualUpdates();
	}
}

void ACursedRoomRitual::StopVisualUpdates()
{
	if (GetWorld())
	{
		GetWorldTimerManager().ClearTimer(VisualTimerHandle);
	}
}

void ACursedRoomRitual::LogStageEntered(ECursedRoomRitualState PreviousState) const
{
	const UStaticMeshComponent* SkullMesh = IsValid(ReplicatedState.GoatSkull)
		? ReplicatedState.GoatSkull->GetItemMesh()
		: nullptr;
	const FString StateName = StaticEnum<ECursedRoomRitualState>()->GetNameStringByValue(
		static_cast<int64>(ReplicatedState.State));
	const FString PreviousStateName = StaticEnum<ECursedRoomRitualState>()->GetNameStringByValue(
		static_cast<int64>(PreviousState));

	UE_LOG(LogCursedRoomRitual, Log,
		TEXT("[RitualStage][Sequence=%d] ENTER %s from %s | Duration=%.2fs Mesh=%s Location=%s Target=%s Physics=%s Parent=%s"),
		ReplicatedState.SequenceId,
		*StateName,
		*PreviousStateName,
		ReplicatedState.Duration,
		*GetNameSafe(SkullMesh),
		*GetRitualItemWorldTransform(ReplicatedState.GoatSkull).GetLocation().ToCompactString(),
		*ReplicatedState.SkullTargetTransform.GetLocation().ToCompactString(),
		SkullMesh && SkullMesh->IsSimulatingPhysics() ? TEXT("ON") : TEXT("OFF"),
		*GetNameSafe(SkullMesh ? SkullMesh->GetAttachParent() : nullptr));

	if (ReplicatedState.State == ECursedRoomRitualState::Reacting)
	{
		UE_LOG(LogCursedRoomRitual, Log,
			TEXT("[RitualStage][Sequence=%d] SHAKE Distance=+/-%.1fcm Cycles=%.1f"),
			ReplicatedState.SequenceId,
			CorrectRoomShakeDistance,
			CorrectRoomShakeCycles);
	}
	else if (ReplicatedState.State == ECursedRoomRitualState::Floating)
	{
		UE_LOG(LogCursedRoomRitual, Log,
			TEXT("[RitualStage][Sequence=%d] FLOAT RiseDistance=%.1fcm Duration=%.2fs CeilingTarget=%s"),
			ReplicatedState.SequenceId,
			FVector::Distance(
				ReplicatedState.SkullStartTransform.GetLocation(),
				ReplicatedState.SkullTargetTransform.GetLocation()),
			ReplicatedState.Duration,
			*ReplicatedState.SkullTargetTransform.GetLocation().ToCompactString());
	}
	else if (ReplicatedState.State == ECursedRoomRitualState::Scratching)
	{
		UE_LOG(LogCursedRoomRitual, Log,
			TEXT("[RitualStage][Sequence=%d] CEILING SIDE-TO-SIDE Distance=+/-%.1fcm Cycles=%.1f Duration=%.2fs Direction=%s"),
			ReplicatedState.SequenceId,
			ScratchSideToSideDistance,
			ScratchSideToSideCycles,
			ReplicatedState.Duration,
			*ReplicatedState.ScratchDirection.ToCompactString());
	}
}

void ACursedRoomRitual::LogStageFinished(ECursedRoomRitualState FinishedState) const
{
	UE_LOG(LogCursedRoomRitual, Log,
		TEXT("[RitualStage][Sequence=%d] EXIT %s | MeshLocation=%s Elapsed=%.2fs"),
		ReplicatedState.SequenceId,
		*StaticEnum<ECursedRoomRitualState>()->GetNameStringByValue(static_cast<int64>(FinishedState)),
		*GetRitualItemWorldTransform(ReplicatedState.GoatSkull).GetLocation().ToCompactString(),
		FMath::Max(0.0, GetSynchronizedServerTime() - ReplicatedState.StartServerTime));
}

void ACursedRoomRitual::RevealSymbolsUpTo(int32 DesiredCount)
{
	if (!HasAuthority())
	{
		return;
	}

	DesiredCount = FMath::Clamp(DesiredCount, 0, NumberOfSymbols);
	while (RevealedSymbolCount < DesiredCount)
	{
		RevealSymbol(RevealedSymbolCount++);
	}
}

void ACursedRoomRitual::RevealSymbol(int32 SlotIndex)
{
	if (!FullCode.IsValidIndex(SlotIndex))
	{
		return;
	}

	if (PastVisibleSlots.Contains(SlotIndex))
	{
		SendSymbolToTimeline(SlotIndex, EItemTimeline::Past);
	}
	else if (FutureVisibleSlots.Contains(SlotIndex))
	{
		SendSymbolToTimeline(SlotIndex, EItemTimeline::Future);
	}
}

void ACursedRoomRitual::SendSymbolToTimeline(int32 SlotIndex, EItemTimeline Timeline)
{
	if (!HasAuthority() || !FullCode.IsValidIndex(SlotIndex))
	{
		return;
	}

	const FTransform SymbolTransform = MakeSymbolTransform(SlotIndex, Timeline);
	for (TActorIterator<AHronoCharacter> It(GetWorld()); It; ++It)
	{
		AHronoCharacter* Character = *It;
		if (IsValid(Character) && Character->GetTimeline() == Timeline)
		{
			Character->ClientReceiveCursedRoomRitualSymbol(
				this,
				ReplicatedState.SequenceId,
				SlotIndex,
				FullCode[SlotIndex],
				Timeline,
				SymbolTransform);
		}
	}
}

FTransform ACursedRoomRitual::MakeSymbolTransform(int32 SlotIndex, EItemTimeline Timeline) const
{
	if (Timeline == EItemTimeline::Past)
	{
		const FVector Location = CeilingSurfaceStart + ScratchWorldDirection * (ScratchSymbolSpacing * SlotIndex);
		const FQuat Rotation = FRotationMatrix::MakeFromZ(-CeilingSurfaceNormal).ToQuat();
		return FTransform(Rotation, Location);
	}

	const ARitualCandle* Candle = ReplicatedState.RitualCandle;
	if (!IsValid(Candle))
	{
		return FTransform::Identity;
	}

	FTransform Result = GetRitualItemWorldTransform(Candle);
	const FVector CenteredOffset = FutureSymbolOffset
		+ FVector(0.0f, (SlotIndex - (NumberOfSymbols - 1) * 0.5f) * FutureSymbolSpacing, 0.0f);
	Result.SetLocation(Result.TransformPosition(CenteredOffset));
	return Result;
}

FTransform ACursedRoomRitual::GetRitualItemWorldTransform(const ABase_Item* Item) const
{
	if (!IsValid(Item))
	{
		return FTransform::Identity;
	}

	if (const UStaticMeshComponent* ItemMesh = Item->GetItemMesh())
	{
		return ItemMesh->GetComponentTransform();
	}
	return Item->GetActorTransform();
}

void ACursedRoomRitual::SetRitualItemWorldTransform(
	ABase_Item* Item,
	const FTransform& WorldTransform) const
{
	if (!IsValid(Item))
	{
		return;
	}

	if (UStaticMeshComponent* ItemMesh = Item->GetItemMesh())
	{
		ItemMesh->SetWorldTransform(
			WorldTransform,
			false,
			nullptr,
			ETeleportType::TeleportPhysics);
		return;
	}

	Item->SetActorTransform(WorldTransform, false, nullptr, ETeleportType::TeleportPhysics);
}

void ACursedRoomRitual::RequestCompleteClueForLocalPlayer()
{
	if (!GetWorld())
	{
		return;
	}

	APlayerController* LocalController = GetWorld()->GetFirstPlayerController();
	AHronoCharacter* LocalCharacter = LocalController
		? Cast<AHronoCharacter>(LocalController->GetPawn())
		: nullptr;
	if (IsValid(LocalCharacter))
	{
		GetWorldTimerManager().ClearTimer(LocalClueRetryTimerHandle);
		LocalCharacter->ServerRequestCursedRoomRitualClue(this, ReplicatedState.SequenceId);
		return;
	}

	if (!GetWorldTimerManager().IsTimerActive(LocalClueRetryTimerHandle))
	{
		GetWorldTimerManager().SetTimer(
			LocalClueRetryTimerHandle,
			this,
			&ACursedRoomRitual::RequestCompleteClueForLocalPlayer,
			0.5f,
			true);
	}
}

double ACursedRoomRitual::GetSynchronizedServerTime() const
{
	if (const UWorld* World = GetWorld())
	{
		if (const AGameStateBase* GameState = World->GetGameState())
		{
			return GameState->GetServerWorldTimeSeconds();
		}
		return World->GetTimeSeconds();
	}
	return 0.0;
}

float ACursedRoomRitual::EvaluateMoveAlpha(float LinearAlpha) const
{
	const float ClampedAlpha = FMath::Clamp(LinearAlpha, 0.0f, 1.0f);
	return SkullMoveCurve
		? FMath::Clamp(SkullMoveCurve->GetFloatValue(ClampedAlpha), 0.0f, 1.0f)
		: FMath::InterpEaseInOut(0.0f, 1.0f, ClampedAlpha, 2.0f);
}

FString ACursedRoomRitual::BuildMaskedCode(EItemTimeline Timeline) const
{
	TArray<FString> Masked;
	Masked.Init(TEXT("?"), FullCode.Num());
	const TArray<int32>& VisibleSlots = Timeline == EItemTimeline::Past
		? PastVisibleSlots
		: FutureVisibleSlots;
	for (const int32 Slot : VisibleSlots)
	{
		if (FullCode.IsValidIndex(Slot))
		{
			Masked[Slot] = FullCode[Slot];
		}
	}
	return FString::Join(Masked, TEXT(" "));
}

void ACursedRoomRitual::ClearLocalSymbolVisuals()
{
	for (const TWeakObjectPtr<ARitualSymbolVisual>& SymbolVisual : LocalSymbolVisuals)
	{
		if (SymbolVisual.IsValid())
		{
			SymbolVisual->Destroy();
		}
	}
	LocalSymbolVisuals.Reset();
	LocalReceivedSlots.Reset();
	LocalVisibleClue.Reset();
	LocalVisibleSequenceId = INDEX_NONE;
}
