#include "Ritual/CursedRoomRitual.h"

#include "Components/BoxComponent.h"
#include "Components/LightComponentBase.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Enviroment/Room.h"
#include "Enviroment/Light_Env.h"
#include "Enviroment/Switcher_Env.h"
#include "GameFramework/GameStateBase.h"
#include "HronoCharacter.h"
#include "HronoCollisionChannels.h"
#include "Items/Base_Item.h"
#include "Items/Drag_Item.h"
#include "Items/RitualGoatSkull.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "ScareDirector.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogCursedRoomRitual, Log, All);

namespace
{
	FTransform GetSkullTransform(const ARitualGoatSkull* Skull)
	{
		return IsValid(Skull) && IsValid(Skull->GetItemMesh())
			? Skull->GetItemMesh()->GetComponentTransform()
			: FTransform::Identity;
	}
}

ACursedRoomRitual::ACursedRoomRitual()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(false);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
}

void ACursedRoomRitual::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		int32 CoordinatorCount = 0;
		for (TActorIterator<ACursedRoomRitual> It(GetWorld()); It; ++It)
		{
			++CoordinatorCount;
		}
		if (CoordinatorCount != 1)
		{
			UE_LOG(LogCursedRoomRitual, Warning,
				TEXT("[Ritual] Expected one coordinator, found %d"), CoordinatorCount);
		}

		UClass* NewKeyClass = LoadClass<ABase_Item>(
			nullptr, TEXT("/Game/_Alex/Pickable/BP_Key_Item_A.BP_Key_Item_A_C"));
		UClass* LegacyKeyClass = LoadClass<ABase_Item>(
			nullptr, TEXT("/Game/_Alex/Pickable/BP_Key_Item.BP_Key_Item_C"));

		// Existing maps can serialize the old key into the Blueprint class or placed
		// coordinator instance. Migrate only that legacy value; preserve any other
		// deliberate editor override.
		if (!PastKeyClass || PastKeyClass.Get() == LegacyKeyClass)
		{
			PastKeyClass = NewKeyClass ? NewKeyClass : LegacyKeyClass;
		}
		if (!FutureKeyClass || FutureKeyClass.Get() == LegacyKeyClass)
		{
			FutureKeyClass = NewKeyClass ? NewKeyClass : LegacyKeyClass;
		}

		UE_LOG(LogCursedRoomRitual, Log,
			TEXT("[RitualKey] Configured key classes. Past=%s Future=%s"),
			*GetNameSafe(PastKeyClass.Get()),
			*GetNameSafe(FutureKeyClass.Get()));
	}
}

void ACursedRoomRitual::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopStageUpdates();
	if (GetWorld())
	{
		GetWorldTimerManager().ClearTimer(StateTimerHandle);
		GetWorldTimerManager().ClearTimer(HouseFlickerTimerHandle);
		GetWorldTimerManager().ClearTimer(FloatingKeysTimerHandle);
	}
	EndLocalHouseFlicker(true);
	if (HasAuthority())
	{
		UnlockTestedRoomDoors();
	}
	Super::EndPlay(EndPlayReason);
}

void ACursedRoomRitual::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACursedRoomRitual, ReplicatedState);
	DOREPLIFETIME(ACursedRoomRitual, SpawnedPastKey);
	DOREPLIFETIME(ACursedRoomRitual, SpawnedFutureKey);
	DOREPLIFETIME(ACursedRoomRitual, HouseLightMode);
}

ACursedRoomRitual* ACursedRoomRitual::FindRitual(const UObject* WorldContextObject)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
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

void ACursedRoomRitual::NotifySkullDropped(ARitualGoatSkull* DroppedSkull)
{
	if (!HasAuthority() || !IsValid(DroppedSkull)
		|| (ReplicatedState.State != ECursedRoomRitualState::Idle
			&& ReplicatedState.State != ECursedRoomRitualState::Failed))
	{
		return;
	}

	ARoom* TestedRoom = nullptr;
	ARitualGoatSkull* PastSkull = nullptr;
	ARitualGoatSkull* FutureSkull = nullptr;
	if (!FindSkullPairInRoom(TestedRoom, PastSkull, FutureSkull))
	{
		UE_LOG(LogCursedRoomRitual, Log,
			TEXT("[Ritual] Waiting for matching skull. Dropped=%s Timeline=%s Room=%s"),
			*GetNameSafe(DroppedSkull),
			*StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(DroppedSkull->ItemTimeline)),
			*GetNameSafe(FindContainingRoom(DroppedSkull)));
		return;
	}
	// Moving the pair out of a failed room is required before trying again.
	// This also lets players set a skull down there while rearranging the pair.
	if (ReplicatedState.State == ECursedRoomRitualState::Failed
		&& TestedRoom == ActiveTestedRoom)
	{
		return;
	}

	StartRitual(TestedRoom, PastSkull, FutureSkull);
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

bool ACursedRoomRitual::FindSkullPairInRoom(
	ARoom*& OutRoom,
	ARitualGoatSkull*& OutPast,
	ARitualGoatSkull*& OutFuture) const
{
	OutRoom = nullptr;
	OutPast = nullptr;
	OutFuture = nullptr;
	if (!GetWorld())
	{
		return false;
	}

	for (TActorIterator<ARitualGoatSkull> PastIt(GetWorld()); PastIt; ++PastIt)
	{
		ARitualGoatSkull* Past = *PastIt;
		if (!IsDroppedSkullValid(Past, EItemTimeline::Past))
		{
			continue;
		}

		ARoom* Room = FindContainingRoom(Past);
		if (!IsValid(Room))
		{
			continue;
		}

		for (TActorIterator<ARitualGoatSkull> FutureIt(GetWorld()); FutureIt; ++FutureIt)
		{
			ARitualGoatSkull* Future = *FutureIt;
			if (IsDroppedSkullValid(Future, EItemTimeline::Future)
				&& FindContainingRoom(Future) == Room)
			{
				OutRoom = Room;
				OutPast = Past;
				OutFuture = Future;
				return true;
			}
		}
	}
	return false;
}

bool ACursedRoomRitual::IsDroppedSkullValid(
	const ARitualGoatSkull* Skull,
	EItemTimeline RequiredTimeline) const
{
	return IsValid(Skull)
		&& Skull->ItemTimeline == RequiredTimeline
		&& !Skull->bIsPickedUp
		&& Skull->OwningCharacter == nullptr
		&& !Skull->IsRitualLocked()
		&& !Skull->WasDestroyedByRitual();
}

void ACursedRoomRitual::StartRitual(
	ARoom* TestedRoom,
	ARitualGoatSkull* PastSkull,
	ARitualGoatSkull* FutureSkull)
{
	if (!HasAuthority() || !IsValid(TestedRoom) || !IsValid(PastSkull) || !IsValid(FutureSkull))
	{
		return;
	}

	ActiveTestedRoom = TestedRoom;
	ReplicatedState.PastSkull = PastSkull;
	ReplicatedState.FutureSkull = FutureSkull;
	ReplicatedState.RoomCenter = IsValid(TestedRoom->RoomVolume)
		? TestedRoom->RoomVolume->GetComponentLocation()
		: TestedRoom->GetActorLocation();
	++ReplicatedState.SequenceId;
	PastSkull->SetRitualLocked(true);
	FutureSkull->SetRitualLocked(true);

	UE_LOG(LogCursedRoomRitual, Log,
		TEXT("[Ritual] Pair accepted. Room=%s Past=%s Future=%s. Preparing for %.2fs before validation."),
		*GetNameSafe(TestedRoom), *GetNameSafe(PastSkull), *GetNameSafe(FutureSkull),
		PreparationDuration);

	SetState(ECursedRoomRitualState::Preparing, PreparationDuration);
}

void ACursedRoomRitual::SetState(ECursedRoomRitualState NewState, float Duration)
{
	if (!HasAuthority())
	{
		return;
	}

	const ECursedRoomRitualState PreviousState = ReplicatedState.State;
	ReplicatedState.State = NewState;
	ReplicatedState.StartServerTime = GetSynchronizedServerTime();
	ReplicatedState.Duration = FMath::Max(0.0f, Duration);
	ReplicatedState.RandomSeed = FMath::Rand();
	ReplicatedState.PastStartTransform = GetSkullTransform(ReplicatedState.PastSkull);
	ReplicatedState.FutureStartTransform = GetSkullTransform(ReplicatedState.FutureSkull);
	LastAppliedImpulseIndex = INDEX_NONE;

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

	ForceNetUpdate();
	ApplyReplicatedState();
	UE_LOG(LogCursedRoomRitual, Log, TEXT("[RitualStage] %s -> %s (%.2fs)"),
		*StaticEnum<ECursedRoomRitualState>()->GetNameStringByValue(static_cast<int64>(PreviousState)),
		*StaticEnum<ECursedRoomRitualState>()->GetNameStringByValue(static_cast<int64>(NewState)),
		ReplicatedState.Duration);
}

void ACursedRoomRitual::HandleStateFinished()
{
	if (!HasAuthority())
	{
		return;
	}

	LogStageFinished(ReplicatedState.State);
	switch (ReplicatedState.State)
	{
	case ECursedRoomRitualState::Preparing:
		if (!IsValid(ActiveTestedRoom)
			|| !IsValid(ReplicatedState.PastSkull)
			|| ReplicatedState.PastSkull->ItemTimeline != EItemTimeline::Past
			|| ReplicatedState.PastSkull->bIsPickedUp
			|| ReplicatedState.PastSkull->OwningCharacter != nullptr
			|| ReplicatedState.PastSkull->WasDestroyedByRitual()
			|| !IsValid(ReplicatedState.FutureSkull)
			|| ReplicatedState.FutureSkull->ItemTimeline != EItemTimeline::Future
			|| ReplicatedState.FutureSkull->bIsPickedUp
			|| ReplicatedState.FutureSkull->OwningCharacter != nullptr
			|| ReplicatedState.FutureSkull->WasDestroyedByRitual())
		{
			UE_LOG(LogCursedRoomRitual, Warning,
				TEXT("[Ritual] Preparation cancelled: committed skull pair became invalid in room %s"),
				*GetNameSafe(ActiveTestedRoom));
			UnlockActiveSkulls();
			SetState(ECursedRoomRitualState::Idle, 0.0f);
			break;
		}
		if (ActiveTestedRoom->IsCursed())
		{
			for (TActorIterator<AHronoCharacter> It(GetWorld()); It; ++It)
			{
				if (AHronoCharacter* Character = *It;
					IsValid(Character) && IsValid(Character->GetController()))
				{
					Character->CompleteTutorialStep(EHronoTutorialStep::StartCorrectRoomRitual);
				}
			}
		}
		SetState(ECursedRoomRitualState::Rising, RiseDuration);
		break;
	case ECursedRoomRitualState::Rising:
		// Snap to the exact authored height before the five-second hold begins.
		ApplyRiseTransform(ReplicatedState.PastSkull, ReplicatedState.PastStartTransform, 1.0f);
		ApplyRiseTransform(ReplicatedState.FutureSkull, ReplicatedState.FutureStartTransform, 1.0f);
		CloseAndLockTestedRoomDoors();
		SetHouseLightMode(ERitualHouseLightMode::Flickering);
		SetState(ECursedRoomRitualState::Hovering, HoverDuration);
		break;
	case ECursedRoomRitualState::Hovering:
		UE_LOG(LogCursedRoomRitual, Log,
			TEXT("[Ritual] Shared hover complete. Room=%s Cursed=%s"),
			*GetNameSafe(ActiveTestedRoom),
			IsValid(ActiveTestedRoom) && ActiveTestedRoom->IsCursed() ? TEXT("true") : TEXT("false"));
		if (IsValid(ActiveTestedRoom) && ActiveTestedRoom->IsCursed())
		{
			SetState(ECursedRoomRitualState::Scratching, ScratchDuration);
		}
		else
		{
			SetState(ECursedRoomRitualState::FallingSilent,
				FMath::Max(0.1f, WrongRoomFallingSilenceDuration));
		}
		break;
	case ECursedRoomRitualState::Scratching:
		SetState(
			ECursedRoomRitualState::ReturningToRoomCenter,
			FMath::Max(0.1f, ReturnToCenterDuration));
		break;
	case ECursedRoomRitualState::ReturningToRoomCenter:
		ApplyReturnToCenterTransform(
			ReplicatedState.PastSkull, ReplicatedState.PastStartTransform, 1.0f);
		ApplyReturnToCenterTransform(
			ReplicatedState.FutureSkull, ReplicatedState.FutureStartTransform, 1.0f);
		BeginSuccessfulSkullFall();
		break;
	case ECursedRoomRitualState::FallingToBreak:
		// A floor hit normally completes each skull independently. This timeout
		// prevents the ritual from stalling on unusual collision geometry.
		CompleteSuccessfulRitual();
		break;
	case ECursedRoomRitualState::FallingSilent:
		TriggerWrongRoomConsequences();
		break;
	default:
		break;
	}
}

void ACursedRoomRitual::OnRep_RitualState()
{
	ApplyReplicatedState();
}

void ACursedRoomRitual::OnRep_HouseLightMode()
{
	ApplyLocalHouseLightMode();
}

void ACursedRoomRitual::ApplyReplicatedState()
{
	const ECursedRoomRitualState PreviousState = LastDispatchedState;
	StopStageUpdates();
	LastAppliedImpulseIndex = INDEX_NONE;

	const bool bRitualSoundActive =
		ReplicatedState.State == ECursedRoomRitualState::Rising
		|| ReplicatedState.State == ECursedRoomRitualState::Hovering
		|| ReplicatedState.State == ECursedRoomRitualState::Scratching
		|| ReplicatedState.State == ECursedRoomRitualState::ReturningToRoomCenter
		|| ReplicatedState.State == ECursedRoomRitualState::FallingToBreak
		|| ReplicatedState.State == ECursedRoomRitualState::FallingSilent;
	if (HasAuthority())
	{
		const ERitualSkullAudioState SkullAudioState = bRitualSoundActive
			? ERitualSkullAudioState::Playing
			: ((ReplicatedState.State == ECursedRoomRitualState::Completed
				|| ReplicatedState.State == ECursedRoomRitualState::Failed)
				? ERitualSkullAudioState::Finished
				: ERitualSkullAudioState::Silent);
		for (ARitualGoatSkull* Skull : {
			ReplicatedState.PastSkull.Get(),
			ReplicatedState.FutureSkull.Get() })
		{
			if (IsValid(Skull))
			{
				Skull->SetRitualAudioState(SkullAudioState);
			}
		}
	}

	switch (ReplicatedState.State)
	{
	case ECursedRoomRitualState::Preparing:
		if (IsValid(ReplicatedState.PastSkull))
		{
			ReplicatedState.PastSkull->SetRitualPhysics(false);
		}
		if (IsValid(ReplicatedState.FutureSkull))
		{
			ReplicatedState.FutureSkull->SetRitualPhysics(false);
		}
		break;
	case ECursedRoomRitualState::Rising:
		if (IsValid(ReplicatedState.PastSkull))
		{
			ReplicatedState.PastSkull->SetRitualKinematic();
		}
		if (IsValid(ReplicatedState.FutureSkull))
		{
			ReplicatedState.FutureSkull->SetRitualKinematic();
		}
		StartStageUpdates();
		break;
	case ECursedRoomRitualState::Hovering:
		if (IsValid(ReplicatedState.PastSkull))
		{
			ReplicatedState.PastSkull->SetRitualKinematic();
		}
		if (IsValid(ReplicatedState.FutureSkull))
		{
			ReplicatedState.FutureSkull->SetRitualKinematic();
		}
		break;
	case ECursedRoomRitualState::Scratching:
		if (IsValid(ReplicatedState.PastSkull))
		{
			ReplicatedState.PastSkull->SetRitualPhysics(true);
		}
		if (IsValid(ReplicatedState.FutureSkull))
		{
			ReplicatedState.FutureSkull->SetRitualPhysics(true);
		}
		StartStageUpdates();
		break;
	case ECursedRoomRitualState::ReturningToRoomCenter:
		// Freeze physics first, then drive a synchronized smooth transform back to
		// the center authored by the room volume.
		if (IsValid(ReplicatedState.PastSkull))
		{
			ReplicatedState.PastSkull->SetRitualKinematic();
		}
		if (IsValid(ReplicatedState.FutureSkull))
		{
			ReplicatedState.FutureSkull->SetRitualKinematic();
		}
		StartStageUpdates();
		break;
	case ECursedRoomRitualState::FallingToBreak:
		for (ARitualGoatSkull* Skull : {
			ReplicatedState.PastSkull.Get(),
			ReplicatedState.FutureSkull.Get() })
		{
			if (IsValid(Skull) && !Skull->WasDestroyedByRitual()
				&& !Skull->bIsPickedUp && Skull->OwningCharacter == nullptr)
			{
				Skull->SetRitualPhysics(false);
				if (UStaticMeshComponent* SkullMesh = Skull->GetItemMesh())
				{
					// Past and Future skulls share the same center but must not
					// physically collide with one another while falling.
					SkullMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_ITEM, ECR_Ignore);
				}
			}
		}
		break;
	case ECursedRoomRitualState::FallingSilent:
		for (ARitualGoatSkull* Skull : {
			ReplicatedState.PastSkull.Get(),
			ReplicatedState.FutureSkull.Get() })
		{
			if (IsValid(Skull) && !Skull->WasDestroyedByRitual()
				&& !Skull->bIsPickedUp && Skull->OwningCharacter == nullptr)
			{
				if (HasAuthority() && IsValid(Skull->GetItemMesh()))
				{
					Skull->GetItemMesh()->SetNotifyRigidBodyCollision(true);
					Skull->GetItemMesh()->OnComponentHit.AddUniqueDynamic(
						this, &ACursedRoomRitual::HandleFallingSkullHit);
				}
				Skull->SetRitualPhysics(false);
			}
		}
		break;
	case ECursedRoomRitualState::Idle:
		UnlockActiveSkulls();
		break;
	case ECursedRoomRitualState::Completed:
		break;
	case ECursedRoomRitualState::Failed:
		UnlockActiveSkulls();
		break;
	}

	DispatchStateEvents(PreviousState);
	LogStageEntered(PreviousState);
	LastDispatchedState = ReplicatedState.State;
	LastDispatchedSequenceId = ReplicatedState.SequenceId;
}

void ACursedRoomRitual::StartStageUpdates()
{
	if (!GetWorld())
	{
		return;
	}
	UpdateActiveStage();
	GetWorldTimerManager().SetTimer(
		StageUpdateTimerHandle,
		this,
		&ACursedRoomRitual::UpdateActiveStage,
		FMath::Max(0.01f, PhysicsUpdateInterval),
		true);
}

void ACursedRoomRitual::StopStageUpdates()
{
	if (GetWorld())
	{
		GetWorldTimerManager().ClearTimer(StageUpdateTimerHandle);
	}
}

void ACursedRoomRitual::UpdateActiveStage()
{
	const float Duration = FMath::Max(KINDA_SMALL_NUMBER, ReplicatedState.Duration);
	const float Alpha = FMath::Clamp(
		static_cast<float>((GetSynchronizedServerTime() - ReplicatedState.StartServerTime) / Duration),
		0.0f, 1.0f);

	if (ReplicatedState.State == ECursedRoomRitualState::Rising)
	{
		ApplyRiseTransform(ReplicatedState.PastSkull, ReplicatedState.PastStartTransform, Alpha);
		ApplyRiseTransform(ReplicatedState.FutureSkull, ReplicatedState.FutureStartTransform, Alpha);
		return;
	}

	if (ReplicatedState.State == ECursedRoomRitualState::ReturningToRoomCenter)
	{
		ApplyReturnToCenterTransform(
			ReplicatedState.PastSkull, ReplicatedState.PastStartTransform, Alpha);
		ApplyReturnToCenterTransform(
			ReplicatedState.FutureSkull, ReplicatedState.FutureStartTransform, Alpha);
		return;
	}

	// Only the server drives rigid bodies. Root-body movement replication carries
	// the result to clients; clients never choose forces or ritual outcomes.
	if (!HasAuthority())
	{
		return;
	}

	if (ReplicatedState.State == ECursedRoomRitualState::Scratching)
	{
		ApplyUpwardAcceleration(ReplicatedState.PastSkull, CeilingHoldAcceleration);
		ApplyUpwardAcceleration(ReplicatedState.FutureSkull, CeilingHoldAcceleration);

		const int32 ImpulseIndex = FMath::FloorToInt(
			static_cast<float>(GetSynchronizedServerTime() - ReplicatedState.StartServerTime)
			/ FMath::Max(0.05f, RandomImpulseInterval));
		if (ImpulseIndex > LastAppliedImpulseIndex)
		{
			LastAppliedImpulseIndex = ImpulseIndex;
			ApplyRandomImpulse(ReplicatedState.PastSkull, ImpulseIndex, 17);
			ApplyRandomImpulse(ReplicatedState.FutureSkull, ImpulseIndex, 53);
			UE_LOG(LogCursedRoomRitual, Verbose,
				TEXT("[RitualStage] Scratching impulse %d"), ImpulseIndex);
		}
	}
}

void ACursedRoomRitual::ApplyRiseTransform(
	ARitualGoatSkull* Skull,
	const FTransform& StartTransform,
	float Alpha) const
{
	if (!IsValid(Skull) || !IsValid(Skull->GetItemMesh()))
	{
		return;
	}

	const FVector TargetLocation = StartTransform.GetLocation() + FVector::UpVector * HoverHeight;
	Skull->GetItemMesh()->SetWorldLocationAndRotation(
		FMath::Lerp(StartTransform.GetLocation(), TargetLocation, FMath::SmoothStep(0.0f, 1.0f, Alpha)),
		StartTransform.GetRotation(),
		false,
		nullptr,
		ETeleportType::TeleportPhysics);
}

void ACursedRoomRitual::ApplyReturnToCenterTransform(
	ARitualGoatSkull* Skull,
	const FTransform& StartTransform,
	float Alpha) const
{
	if (!IsValid(Skull) || !IsValid(Skull->GetItemMesh()))
	{
		return;
	}

	const float SmoothAlpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);
	Skull->GetItemMesh()->SetWorldLocationAndRotation(
		FMath::Lerp(StartTransform.GetLocation(), ReplicatedState.RoomCenter, SmoothAlpha),
		StartTransform.GetRotation(),
		false,
		nullptr,
		ETeleportType::TeleportPhysics);
}

void ACursedRoomRitual::ApplyUpwardAcceleration(ARitualGoatSkull* Skull, float Acceleration) const
{
	if (IsValid(Skull) && IsValid(Skull->GetItemMesh()))
	{
		Skull->GetItemMesh()->AddForce(FVector::UpVector * Acceleration, NAME_None, true);
	}
}

void ACursedRoomRitual::ApplyRandomImpulse(
	ARitualGoatSkull* Skull,
	int32 ImpulseIndex,
	int32 TimelineSalt) const
{
	if (!IsValid(Skull) || !IsValid(Skull->GetItemMesh()))
	{
		return;
	}

	FRandomStream RandomStream(HashCombine(ReplicatedState.RandomSeed, HashCombine(ImpulseIndex, TimelineSalt)));
	FVector Direction(
		RandomStream.FRandRange(-1.0f, 1.0f),
		RandomStream.FRandRange(-1.0f, 1.0f),
		RandomStream.FRandRange(-0.25f, 0.55f));
	Direction = Direction.GetSafeNormal(KINDA_SMALL_NUMBER, FVector::RightVector);
	Skull->GetItemMesh()->AddImpulse(Direction * RandomImpulseStrength, NAME_None, true);
	Skull->GetItemMesh()->AddAngularImpulseInDegrees(
		RandomStream.VRand() * RandomImpulseStrength * 0.8f, NAME_None, true);
}

ABase_Item* ACursedRoomRitual::SpawnTimelineKey(
	TSubclassOf<ABase_Item> KeyClass,
	EItemTimeline Timeline,
	ARitualGoatSkull* SourceSkull)
{
	if (!HasAuthority() || !GetWorld() || !KeyClass || !IsValid(SourceSkull))
	{
		UE_LOG(LogCursedRoomRitual, Error,
			TEXT("[RitualKey] Cannot spawn %s key. Class=%s Skull=%s"),
			*StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(Timeline)),
			*GetNameSafe(KeyClass.Get()), *GetNameSafe(SourceSkull));
		return nullptr;
	}

	const FVector SkullLocation = GetSkullTransform(SourceSkull).GetLocation();
	const FVector HorizontalOffset(KeySpawnOffset.X, KeySpawnOffset.Y, 0.0f);
	const FVector TraceStart = SkullLocation + HorizontalOffset + FVector::UpVector * 100.0f;
	const FVector TraceEnd = TraceStart - FVector::UpVector * 700.0f;
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(RitualKeyFloor), false);
	TraceParams.AddIgnoredActor(SourceSkull);
	TraceParams.AddIgnoredActor(ReplicatedState.PastSkull.Get());
	TraceParams.AddIgnoredActor(ReplicatedState.FutureSkull.Get());
	TraceParams.AddIgnoredActor(SpawnedPastKey.Get());
	TraceParams.AddIgnoredActor(SpawnedFutureKey.Get());
	FHitResult FloorHit;
	const bool bFoundFloor = GetWorld()->LineTraceSingleByChannel(
		FloorHit, TraceStart, TraceEnd, ECC_Visibility, TraceParams)
		&& FloorHit.ImpactNormal.Z > 0.45f;
	const UStaticMeshComponent* SkullMesh = SourceSkull->GetItemMesh();
	const float FloorZ = bFoundFloor ? FloorHit.ImpactPoint.Z
		: (IsValid(SkullMesh)
			? SkullMesh->Bounds.Origin.Z - SkullMesh->Bounds.BoxExtent.Z
			: SkullLocation.Z);
	if (!bFoundFloor)
	{
		UE_LOG(LogCursedRoomRitual, Warning,
			TEXT("[RitualKey] No floor trace below %s; using skull bounds as height fallback"),
			*GetNameSafe(SourceSkull));
	}
	const FVector HoverOrigin(
		SkullLocation.X + HorizontalOffset.X,
		SkullLocation.Y + HorizontalOffset.Y,
		FloorZ + FMath::Max(0.0f, KeyHoverHeight));
	const FTransform SpawnTransform(FRotator::ZeroRotator, HoverOrigin);
	ABase_Item* Key = GetWorld()->SpawnActorDeferred<ABase_Item>(
		KeyClass,
		SpawnTransform,
		this,
		SourceSkull->GetInstigator(),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!IsValid(Key))
	{
		UE_LOG(LogCursedRoomRitual, Error,
			TEXT("[RitualKey] SpawnActorDeferred failed for %s"), *GetNameSafe(KeyClass.Get()));
		return nullptr;
	}

	Key->ItemType = EItemType::Key;
	Key->SetItemTimeline(Timeline);
	Key->FinishSpawning(SpawnTransform);
	if (!IsValid(Key))
	{
		UE_LOG(LogCursedRoomRitual, Error,
			TEXT("[RitualKey] FinishSpawning failed for %s"), *GetNameSafe(KeyClass.Get()));
		return nullptr;
	}
	Key->EnableFloatingPickup();
	Key->SetInteractionHighlightForced(true);
	if (IsValid(Key->GetItemMesh()))
	{
		FloatingKeyOrigins.Add(Key, HoverOrigin);
		if (!GetWorldTimerManager().IsTimerActive(FloatingKeysTimerHandle))
		{
			GetWorldTimerManager().SetTimer(
				FloatingKeysTimerHandle, this,
				&ACursedRoomRitual::UpdateFloatingKeys, 0.033f, true);
		}
	}

	UE_LOG(LogCursedRoomRitual, Log,
		TEXT("[RitualKey] Spawned floating %s for %s at %s (floor Z=%.1f)"),
		*GetNameSafe(Key),
		*StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(Timeline)),
		*HoverOrigin.ToCompactString(), FloorZ);
	return Key;
}

void ACursedRoomRitual::UpdateFloatingKeys()
{
	if (!HasAuthority() || !GetWorld())
	{
		return;
	}

	const float Phase = GetWorld()->GetTimeSeconds()
		* (2.0f * PI / FMath::Max(0.1f, KeyHoverPeriod));
	const float HeightOffset = FMath::Max(0.0f, KeyHoverAmplitude) * FMath::Sin(Phase);
	for (auto It = FloatingKeyOrigins.CreateIterator(); It; ++It)
	{
		ABase_Item* Key = It.Key().Get();
		if (!IsValid(Key) || Key->bIsPickedUp || Key->OwningCharacter != nullptr)
		{
			It.RemoveCurrent();
			continue;
		}
		Key->SetActorLocation(It.Value() + FVector::UpVector * HeightOffset,
			false, nullptr, ETeleportType::TeleportPhysics);
	}
	if (FloatingKeyOrigins.IsEmpty())
	{
		GetWorldTimerManager().ClearTimer(FloatingKeysTimerHandle);
	}
}

void ACursedRoomRitual::BeginSuccessfulSkullFall()
{
	if (!HasAuthority())
	{
		return;
	}

	StopStageUpdates();
	BrokenSkulls.Reset();
	SpawnedPastKey = nullptr;
	SpawnedFutureKey = nullptr;
	bSuccessfulConsequencesApplied = false;

	for (ARitualGoatSkull* Skull : {
		ReplicatedState.PastSkull.Get(),
		ReplicatedState.FutureSkull.Get() })
	{
		if (IsValid(Skull) && IsValid(Skull->GetItemMesh()))
		{
			Skull->GetItemMesh()->SetNotifyRigidBodyCollision(true);
			Skull->GetItemMesh()->OnComponentHit.AddUniqueDynamic(
				this, &ACursedRoomRitual::HandleFallingSkullHit);
		}
	}

	SetState(
		ECursedRoomRitualState::FallingToBreak,
		FMath::Max(0.1f, SkullFallTimeout));
}

void ACursedRoomRitual::HandleFallingSkullHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (!HasAuthority()
		|| (ReplicatedState.State != ECursedRoomRitualState::FallingToBreak
			&& ReplicatedState.State != ECursedRoomRitualState::FallingSilent)
		|| Hit.ImpactNormal.Z < 0.45f
		|| !IsValid(HitComponent))
	{
		return;
	}

	ARitualGoatSkull* Skull = Cast<ARitualGoatSkull>(HitComponent->GetOwner());
	if (Skull != ReplicatedState.PastSkull && Skull != ReplicatedState.FutureSkull)
	{
		return;
	}
	if (ReplicatedState.State == ECursedRoomRitualState::FallingSilent)
	{
		HitComponent->OnComponentHit.RemoveDynamic(this, &ACursedRoomRitual::HandleFallingSkullHit);
		Skull->ReleaseFromFailedRitual();
		UE_LOG(LogCursedRoomRitual, Log,
			TEXT("[Ritual] Wrong-room skull %s landed and can be picked up again"),
			*GetNameSafe(Skull));
		return;
	}

	BreakFallingSkull(Skull);
	const int32 ExpectedSkulls =
		(IsValid(ReplicatedState.PastSkull) ? 1 : 0)
		+ (IsValid(ReplicatedState.FutureSkull) ? 1 : 0);
	if (ExpectedSkulls > 0 && BrokenSkulls.Num() >= ExpectedSkulls)
	{
		CompleteSuccessfulRitual();
	}
}

void ACursedRoomRitual::BreakFallingSkull(ARitualGoatSkull* Skull)
{
	if (!HasAuthority() || !IsValid(Skull) || BrokenSkulls.Contains(Skull))
	{
		return;
	}

	BrokenSkulls.Add(Skull);
	if (UStaticMeshComponent* SkullMesh = Skull->GetItemMesh())
	{
		SkullMesh->OnComponentHit.RemoveDynamic(this, &ACursedRoomRitual::HandleFallingSkullHit);
	}

	if (Skull == ReplicatedState.PastSkull)
	{
		SpawnedPastKey = SpawnTimelineKey(PastKeyClass, EItemTimeline::Past, Skull);
	}
	else if (Skull == ReplicatedState.FutureSkull)
	{
		SpawnedFutureKey = SpawnTimelineKey(FutureKeyClass, EItemTimeline::Future, Skull);
	}

	Skull->ExplodeFromRitual();
	ForceNetUpdate();
	UE_LOG(LogCursedRoomRitual, Log,
		TEXT("[Ritual] %s hit the floor, broke, and spawned key %s"),
		*GetNameSafe(Skull),
		*GetNameSafe(Skull == ReplicatedState.PastSkull ? SpawnedPastKey.Get() : SpawnedFutureKey.Get()));
}

void ACursedRoomRitual::CompleteSuccessfulRitual()
{
	if (!HasAuthority())
	{
		return;
	}

	StopStageUpdates();
	// The normal path reaches here after both floor hits. On timeout, break any
	// skull that is still falling so the ritual cannot remain locked forever.
	BreakFallingSkull(ReplicatedState.PastSkull);
	BreakFallingSkull(ReplicatedState.FutureSkull);

	SetState(ECursedRoomRitualState::Completed, 0.0f);
	ApplySuccessfulRitualConsequences();
	UE_LOG(LogCursedRoomRitual, Log,
		TEXT("[Ritual] COMPLETED. Floating keys: PastKey=%s FutureKey=%s"),
		*GetNameSafe(SpawnedPastKey), *GetNameSafe(SpawnedFutureKey));
}

void ACursedRoomRitual::ApplySuccessfulRitualConsequences()
{
	if (!HasAuthority() || bSuccessfulConsequencesApplied)
	{
		return;
	}

	bSuccessfulConsequencesApplied = true;
	SetHouseLightMode(ERitualHouseLightMode::Normal);
	UnlockTestedRoomDoors();

	if (AScareDirector* Director = AScareDirector::GetHuntDirector(this))
	{
		Director->AddThreatWithReason(
			FMath::Max(0.0f, CorrectRoomThreatIncrease),
			TEXT("Correct cursed-room ritual completed and its keys spawned"));
	}
	else
	{
		UE_LOG(LogCursedRoomRitual, Warning,
			TEXT("[Ritual] Correct ritual could not add Threat: no ScareDirector found"));
	}

	UE_LOG(LogCursedRoomRitual, Log,
		TEXT("[Ritual] Keys spawned. Flicker stopped, room doors unlocked, Threat +%.1f."),
		CorrectRoomThreatIncrease);
}

void ACursedRoomRitual::TriggerWrongRoomConsequences()
{
	if (!HasAuthority())
	{
		return;
	}

	StopStageUpdates();
	UnlockTestedRoomDoors();
	SetHouseLightMode(ERitualHouseLightMode::Blackout);

	if (AScareDirector* Director = AScareDirector::GetHuntDirector(this))
	{
		Director->ForceCloseAllDoorsForEvent(TEXT("Wrong cursed-room ritual"));
		Director->ForceAllLightsOffForEvent(TEXT("Wrong cursed-room ritual"));
		const float TargetThreat = FMath::Max(1.0f, Director->MaxThreat)
			* FMath::Clamp(WrongRoomThreatFraction, 0.0f, 1.0f);
		Director->SetThreatWithReason(TargetThreat,
			TEXT("Wrong cursed-room ritual set aggression to configured percentage"));
		const bool bHuntStarted = Director->RequestTriggeredHunt(
			EItemTimeline::Both,
			false,
			true,
			false);
		UE_LOG(LogCursedRoomRitual, Log,
			TEXT("[Ritual] WRONG ROOM terminal consequence: Threat=%.1f/%.1f, triggered hunt=%s"),
			TargetThreat, Director->MaxThreat, bHuntStarted ? TEXT("started") : TEXT("not started"));
	}
	else
	{
		UE_LOG(LogCursedRoomRitual, Error,
			TEXT("[Ritual] Wrong-room hunt could not start: no ScareDirector found"));
	}

	// Preserve the failed state and its presentation. Once moved to another room,
	// the unlocked skulls can be dropped to start a new attempt.
	SetState(ECursedRoomRitualState::Failed, 0.0f);
}

void ACursedRoomRitual::CloseAndLockTestedRoomDoors()
{
	if (!HasAuthority() || !IsValid(ActiveTestedRoom))
	{
		return;
	}

	LockedRoomDoors.Reset();
	for (ADrag_Item* Door : ActiveTestedRoom->RoomDoors)
	{
		if (!IsValid(Door))
		{
			continue;
		}
		Door->AnimateDoor(false);
		Door->RegisterDoorTriggerLock(true);
		LockedRoomDoors.Add(Door);
	}

	UE_LOG(LogCursedRoomRitual, Log,
		TEXT("[RitualDoors] Closed and locked %d door(s) for room %s"),
		LockedRoomDoors.Num(), *GetNameSafe(ActiveTestedRoom));
}

void ACursedRoomRitual::UnlockTestedRoomDoors()
{
	if (!HasAuthority())
	{
		return;
	}

	for (const TWeakObjectPtr<ADrag_Item>& DoorPtr : LockedRoomDoors)
	{
		if (ADrag_Item* Door = DoorPtr.Get())
		{
			Door->RegisterDoorTriggerLock(false);
		}
	}
	LockedRoomDoors.Reset();
}

void ACursedRoomRitual::SetHouseLightMode(ERitualHouseLightMode NewMode)
{
	if (!HasAuthority() || HouseLightMode == NewMode)
	{
		return;
	}

	HouseLightMode = NewMode;
	ApplyLocalHouseLightMode();
	ForceNetUpdate();
	UE_LOG(LogCursedRoomRitual, Log, TEXT("[RitualLights] House mode -> %s"),
		*StaticEnum<ERitualHouseLightMode>()->GetNameStringByValue(static_cast<int64>(HouseLightMode)));
}

void ACursedRoomRitual::ApplyLocalHouseLightMode()
{
	switch (HouseLightMode)
	{
	case ERitualHouseLightMode::Flickering:
		BeginLocalHouseFlicker();
		break;
	case ERitualHouseLightMode::Blackout:
		EndLocalHouseFlicker(false);
		ApplyLocalHouseBlackout();
		break;
	case ERitualHouseLightMode::Normal:
	default:
		EndLocalHouseFlicker(true);
		break;
	}
}

void ACursedRoomRitual::BeginLocalHouseFlicker()
{
	if (!GetWorld() || !PreviousEnvironmentLightFlicker.IsEmpty()
		|| !PreviousGenericLightVisibility.IsEmpty()
		|| !FlickeringEmissiveSwitchers.IsEmpty())
	{
		return;
	}

	// All controlled lights begin this mode visibly enabled. Tracking the pulse
	// edge prevents repeated OFF timer samples from replaying the sound.
	bLastHouseFlickerPulseOn = true;

	for (TActorIterator<ALight_Env> It(GetWorld()); It; ++It)
	{
		ALight_Env* EnvironmentLight = *It;
		if (IsValid(EnvironmentLight) && EnvironmentLight->IsLightEnabled())
		{
			PreviousEnvironmentLightFlicker.Add(EnvironmentLight, EnvironmentLight->IsFlickering());
			EnvironmentLight->SetFlickering(true);
		}
	}

	for (TActorIterator<ASwitcher_Env> It(GetWorld()); It; ++It)
	{
		ASwitcher_Env* Switcher = *It;
		if (!IsValid(Switcher) || !Switcher->bIsLightOn)
		{
			continue;
		}

		FlickeringEmissiveSwitchers.Add(Switcher);

		for (AActor* LightActor : Switcher->LightActors)
		{
			if (!IsValid(LightActor) || LightActor->IsA<ALight_Env>())
			{
				continue;
			}

			TInlineComponentArray<ULightComponentBase*> LightComponents;
			LightActor->GetComponents(LightComponents, true);
			for (ULightComponentBase* LightComponent : LightComponents)
			{
				if (IsValid(LightComponent) && LightComponent->IsVisible())
				{
					PreviousGenericLightVisibility.FindOrAdd(LightComponent) = true;
				}
			}
		}
	}

	UpdateLocalHouseFlicker();
	UE_LOG(LogCursedRoomRitual, Log,
		TEXT("[RitualLights] Local flicker started for %d Light_Env, %d linked generic light(s), and %d emissive switch group(s)"),
		PreviousEnvironmentLightFlicker.Num(), PreviousGenericLightVisibility.Num(),
		FlickeringEmissiveSwitchers.Num());
}

void ACursedRoomRitual::EndLocalHouseFlicker(bool bRestorePreviousState)
{
	if (GetWorld())
	{
		GetWorldTimerManager().ClearTimer(HouseFlickerTimerHandle);
	}

	for (const TPair<TWeakObjectPtr<ALight_Env>, bool>& Pair : PreviousEnvironmentLightFlicker)
	{
		if (ALight_Env* EnvironmentLight = Pair.Key.Get())
		{
			if (bRestorePreviousState)
			{
				EnvironmentLight->SetFlickering(Pair.Value);
			}
			else
			{
				EnvironmentLight->SetFlickering(false);
				EnvironmentLight->SetLightEnabled(false);
			}
		}
	}

	for (const TPair<TWeakObjectPtr<ULightComponentBase>, bool>& Pair : PreviousGenericLightVisibility)
	{
		if (ULightComponentBase* LightComponent = Pair.Key.Get())
		{
			LightComponent->SetVisibility(bRestorePreviousState ? Pair.Value : false, true);
		}
	}

	for (const TWeakObjectPtr<ASwitcher_Env>& SwitcherPtr : FlickeringEmissiveSwitchers)
	{
		if (ASwitcher_Env* Switcher = SwitcherPtr.Get())
		{
			Switcher->ApplyEmissiveStateToLinkedActors(
				bRestorePreviousState ? Switcher->bIsLightOn : false);
		}
	}

	PreviousEnvironmentLightFlicker.Reset();
	PreviousGenericLightVisibility.Reset();
	FlickeringEmissiveSwitchers.Reset();
	bLastHouseFlickerPulseOn = true;
}

void ACursedRoomRitual::UpdateLocalHouseFlicker()
{
	if (!GetWorld() || HouseLightMode != ERitualHouseLightMode::Flickering)
	{
		return;
	}

	const bool bPulseOn = FMath::FRand() > 0.30f;
	const bool bTurnedOffThisPulse = bLastHouseFlickerPulseOn && !bPulseOn;
	bLastHouseFlickerPulseOn = bPulseOn;
	for (const TPair<TWeakObjectPtr<ULightComponentBase>, bool>& Pair : PreviousGenericLightVisibility)
	{
		if (ULightComponentBase* LightComponent = Pair.Key.Get())
		{
			LightComponent->SetVisibility(bPulseOn, true);
		}
	}
	for (const TWeakObjectPtr<ASwitcher_Env>& SwitcherPtr : FlickeringEmissiveSwitchers)
	{
		if (ASwitcher_Env* Switcher = SwitcherPtr.Get())
		{
			Switcher->ApplyEmissiveStateToLinkedActors(bPulseOn);
		}
	}

	if (bTurnedOffThisPulse && FlickerOffSound)
	{
		UGameplayStatics::PlaySound2D(this, FlickerOffSound);
	}

	const float MinInterval = FMath::Max(0.04f,
		FMath::Min(HouseFlickerIntervalMin, HouseFlickerIntervalMax));
	const float MaxInterval = FMath::Max(MinInterval,
		FMath::Max(HouseFlickerIntervalMin, HouseFlickerIntervalMax));
	GetWorldTimerManager().SetTimer(
		HouseFlickerTimerHandle,
		this,
		&ACursedRoomRitual::UpdateLocalHouseFlicker,
		FMath::FRandRange(MinInterval, MaxInterval),
		false);
}

void ACursedRoomRitual::ApplyLocalHouseBlackout()
{
	if (!GetWorld())
	{
		return;
	}

	for (TActorIterator<ALight_Env> It(GetWorld()); It; ++It)
	{
		if (ALight_Env* EnvironmentLight = *It)
		{
			EnvironmentLight->SetFlickering(false);
			EnvironmentLight->SetLightEnabled(false);
		}
	}

	for (TActorIterator<ASwitcher_Env> It(GetWorld()); It; ++It)
	{
		if (ASwitcher_Env* Switcher = *It)
		{
			Switcher->ApplyLightStateToLinkedActors(false);
			if (HasAuthority())
			{
				Switcher->SetLightState(false);
			}
		}
	}
}

void ACursedRoomRitual::UnlockActiveSkulls()
{
	for (ARitualGoatSkull* Skull : { ReplicatedState.PastSkull.Get(), ReplicatedState.FutureSkull.Get() })
	{
		if (IsValid(Skull) && !Skull->WasDestroyedByRitual())
		{
			if (HasAuthority() && IsValid(Skull->GetItemMesh()))
			{
				Skull->GetItemMesh()->OnComponentHit.RemoveDynamic(
					this, &ACursedRoomRitual::HandleFallingSkullHit);
			}
			Skull->ReleaseFromFailedRitual();
		}
	}
}

double ACursedRoomRitual::GetSynchronizedServerTime() const
{
	const UWorld* World = GetWorld();
	const AGameStateBase* GameState = World ? World->GetGameState() : nullptr;
	return GameState ? GameState->GetServerWorldTimeSeconds()
		: (World ? World->GetTimeSeconds() : 0.0);
}

void ACursedRoomRitual::DispatchStateEvents(ECursedRoomRitualState PreviousState)
{
	if (PreviousState == ReplicatedState.State
		&& LastDispatchedSequenceId == ReplicatedState.SequenceId)
	{
		return;
	}

	OnRitualStateChanged.Broadcast(PreviousState, ReplicatedState.State);
	switch (ReplicatedState.State)
	{
	case ECursedRoomRitualState::Rising:
		BP_OnRitualStarted(ReplicatedState.PastSkull, ReplicatedState.FutureSkull);
		BP_OnSkullsStartFloating(ReplicatedState.PastSkull, ReplicatedState.FutureSkull);
		if (IsValid(ReplicatedState.PastSkull)) ReplicatedState.PastSkull->BP_OnSkullStartFloating();
		if (IsValid(ReplicatedState.FutureSkull)) ReplicatedState.FutureSkull->BP_OnSkullStartFloating();
		break;
	case ECursedRoomRitualState::Hovering:
		BP_OnSkullsStartHovering(ReplicatedState.PastSkull, ReplicatedState.FutureSkull);
		break;
	case ECursedRoomRitualState::Scratching:
		BP_OnSkullsStartScratching(ReplicatedState.PastSkull, ReplicatedState.FutureSkull);
		if (IsValid(ReplicatedState.PastSkull)) ReplicatedState.PastSkull->BP_OnSkullStartScratching();
		if (IsValid(ReplicatedState.FutureSkull)) ReplicatedState.FutureSkull->BP_OnSkullStartScratching();
		break;
	case ECursedRoomRitualState::Failed:
		BP_OnWrongRoom(ActiveTestedRoom, ReplicatedState.PastSkull, ReplicatedState.FutureSkull);
		if (IsValid(ReplicatedState.PastSkull)) ReplicatedState.PastSkull->BP_OnWrongRoomReaction();
		if (IsValid(ReplicatedState.FutureSkull)) ReplicatedState.FutureSkull->BP_OnWrongRoomReaction();
		if (HasAuthority())
		{
			OnWrongRoomRitual.Broadcast(ActiveTestedRoom, ReplicatedState.PastSkull, ReplicatedState.FutureSkull);
		}
		break;
	case ECursedRoomRitualState::Completed:
		BP_OnRitualCompleted(SpawnedPastKey, SpawnedFutureKey);
		OnRitualCompleted.Broadcast(SpawnedPastKey, SpawnedFutureKey);
		break;
	default:
		break;
	}
}

void ACursedRoomRitual::LogStageEntered(ECursedRoomRitualState PreviousState) const
{
	UE_LOG(LogCursedRoomRitual, Log,
		TEXT("[RitualStage] ENTER %s (from %s, Sequence=%d, Duration=%.2f, Past=%s, Future=%s)"),
		*StaticEnum<ECursedRoomRitualState>()->GetNameStringByValue(static_cast<int64>(ReplicatedState.State)),
		*StaticEnum<ECursedRoomRitualState>()->GetNameStringByValue(static_cast<int64>(PreviousState)),
		ReplicatedState.SequenceId, ReplicatedState.Duration,
		*GetNameSafe(ReplicatedState.PastSkull), *GetNameSafe(ReplicatedState.FutureSkull));
}

void ACursedRoomRitual::LogStageFinished(ECursedRoomRitualState FinishedState) const
{
	UE_LOG(LogCursedRoomRitual, Log, TEXT("[RitualStage] EXIT %s (Sequence=%d)"),
		*StaticEnum<ECursedRoomRitualState>()->GetNameStringByValue(static_cast<int64>(FinishedState)),
		ReplicatedState.SequenceId);
}

FString ACursedRoomRitual::GetRitualDebugStatus() const
{
	return FString::Printf(
		TEXT("State=%s Past=%s PastRoom=%s Future=%s FutureRoom=%s TestedRoom=%s Cursed(ServerOnly)=%s PastKey=%s FutureKey=%s"),
		*StaticEnum<ECursedRoomRitualState>()->GetNameStringByValue(static_cast<int64>(ReplicatedState.State)),
		*GetNameSafe(ReplicatedState.PastSkull), *GetNameSafe(FindContainingRoom(ReplicatedState.PastSkull)),
		*GetNameSafe(ReplicatedState.FutureSkull), *GetNameSafe(FindContainingRoom(ReplicatedState.FutureSkull)),
		*GetNameSafe(ActiveTestedRoom),
		HasAuthority() && IsValid(ActiveTestedRoom) && ActiveTestedRoom->IsCursed() ? TEXT("true") : TEXT("hidden/false"),
		*GetNameSafe(SpawnedPastKey), *GetNameSafe(SpawnedFutureKey));
}
