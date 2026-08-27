#include "Items/Clock.h"

#include "Components/ClockSecondHandSoundComponent.h"
#include "Components/SceneComponent.h"
#include "Enviroment/Room.h"
#include "GameFramework/GameStateBase.h"
#include "HronoCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundBase.h"

DEFINE_LOG_CATEGORY_STATIC(LogClock, Log, All);

float FClockTimeOfDay::ToSeconds() const
{
	const int32 NormalizedHour = ((Hour % 24) + 24) % 24;
	return static_cast<float>(
		NormalizedHour * 3600 + FMath::Clamp(Minute, 0, 59) * 60 + FMath::Clamp(Second, 0, 59));
}

FClockTimeOfDay FClockTimeOfDay::FromSeconds(float Seconds)
{
	const int32 WholeSeconds = FMath::FloorToInt(FMath::Fmod(
		FMath::Fmod(Seconds, 86400.0f) + 86400.0f,
		86400.0f));

	FClockTimeOfDay Result;
	Result.Hour = WholeSeconds / 3600;
	Result.Minute = (WholeSeconds / 60) % 60;
	Result.Second = WholeSeconds % 60;
	return Result;
}

void FClockTimeOfDay::Normalize()
{
	*this = FromSeconds(ToSeconds());
}

AClock::AClock()
{
	ItemType = EItemType::Clock;
	InteractionResetTime.Hour = 12;
	InteractionResetTime.Minute = 0;
	InteractionResetTime.Second = 0;

	static ConstructorHelpers::FObjectFinder<USoundBase> TimeResetSoundAsset(
		TEXT("/Game/_Alex/Sound/Clock/ClockReset.ClockReset"));
	if (TimeResetSoundAsset.Succeeded())
	{
		TimeResetSound = TimeResetSoundAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> SecondHandTickSoundAsset(
		TEXT("/Game/_Alex/Sound/Clock/ClockTick.ClockTick"));
	SecondHandSoundComponent = CreateDefaultSubobject<UClockSecondHandSoundComponent>(
		TEXT("SecondHandSound"));
	SecondHandSoundComponent->SetupAttachment(DefaultSceneRoot);
	if (SecondHandTickSoundAsset.Succeeded())
	{
		SecondHandSoundComponent->TickSound = SecondHandTickSoundAsset.Object;
	}

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bAllowTickOnDedicatedServer = false;
}

void AClock::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ResolveClockHands();

	if (!HasActorBegunPlay())
	{
		ApplyHandRotations(StartingTime.ToSeconds());
	}
}

void AClock::BeginPlay()
{
	Super::BeginPlay();
	ResolveClockHands();

	if (!bClockAnchorInitialized)
	{
		StartingTime.Normalize();
		ClockTimeAtAnchor = StartingTime.ToSeconds();
		ServerTimeAtAnchor = GetSynchronizedWorldTime();
		bClockAnchorInitialized = true;
		if (HasAuthority())
		{
			ForceNetUpdate();
		}
	}

	if (!HourHandComponent || !MinuteHandComponent || !SecondHandComponent)
	{
		UE_LOG(LogClock, Warning,
			TEXT("[%s] missing clock hand(s): Hour=%s Minute=%s Second=%s."),
			*GetName(), *GetNameSafe(HourHandComponent),
			*GetNameSafe(MinuteHandComponent), *GetNameSafe(SecondHandComponent));
	}

	LastAudibleSecond = FMath::FloorToInt(GetCurrentClockTime());
	UpdateClockVisual();
	UpdateSecondHandSoundAttachment();
}

void AClock::Tick(float DeltaSeconds)
{
	// Skipping Super::Tick prevents the legacy Blueprint ReceiveTick graph from
	// running alongside the native implementation. Preserve Base_Item's useful work.
	(void)DeltaSeconds;
	UpdateMeshForLocalPlayer();
	UpdateClockVisual();
}

void AClock::Use_Implementation(AActor* Character)
{
	ResetClock(Character);
}

void AClock::ResetClock(AActor* Character)
{
	if (!HasAuthority())
	{
		UE_LOG(LogClock, Warning,
			TEXT("[%s] ResetClock must be called on the server (Character=%s)."),
			*GetName(), *GetNameSafe(Character));
		return;
	}

	InteractionResetTime.Normalize();
	SetCurrentTimeSeconds(InteractionResetTime.ToSeconds());
	MulticastPlayResetSound();
}

void AClock::SetStartingTime(FClockTimeOfDay NewStartingTime, bool bResetImmediately)
{
	if (!HasAuthority())
	{
		return;
	}

	NewStartingTime.Normalize();
	StartingTime = NewStartingTime;
	if (bResetImmediately)
	{
		SetCurrentTimeSeconds(StartingTime.ToSeconds());
	}
	else
	{
		ForceNetUpdate();
	}
}

void AClock::SetCurrentTime(FClockTimeOfDay NewCurrentTime)
{
	if (HasAuthority())
	{
		NewCurrentTime.Normalize();
		SetCurrentTimeSeconds(NewCurrentTime.ToSeconds());
	}
}

FClockTimeOfDay AClock::GetDisplayedTime() const
{
	return FClockTimeOfDay::FromSeconds(GetCurrentClockTime());
}

float AClock::GetCurrentClockTime() const
{
	if (!bClockAnchorInitialized)
	{
		return StartingTime.ToSeconds();
	}

	const double ElapsedRealSeconds = FMath::Max(
		0.0,
		GetSynchronizedWorldTime() - ServerTimeAtAnchor);
	const float UnwrappedTime = ClockTimeAtAnchor
		+ GetAnomalousElapsedTime(ElapsedRealSeconds);
	return FMath::Fmod(FMath::Fmod(UnwrappedTime, SecondsPerDay) + SecondsPerDay, SecondsPerDay);
}

void AClock::AssignToRoom(ARoom* NewOwningRoom)
{
	if (!HasAuthority())
	{
		return;
	}

	OwningRoom = NewOwningRoom;
	ApplyRoomCursedState(IsValid(OwningRoom) && OwningRoom->IsCursed());
	ForceNetUpdate();
}

void AClock::ApplyRoomCursedState(bool bRoomIsCursed)
{
	if (HasAuthority())
	{
		if (bRoomIsCursed)
		{
			SetClockSpeedMode(ResolveCursedSpeedMode());
		}
		else
		{
			ClearClockAnomaly();
		}
	}
}

void AClock::SetClockSpeedMode(EClockSpeedMode NewMode)
{
	if (!HasAuthority())
	{
		return;
	}

	EClockAnomalyType NewAnomaly = EClockAnomalyType::Normal;
	if (NewMode == EClockSpeedMode::Fast)
	{
		NewAnomaly = EClockAnomalyType::Fast;
	}
	else if (NewMode == EClockSpeedMode::Slow)
	{
		NewAnomaly = EClockAnomalyType::Slow;
	}

	ConfigureClockAnomaly(
		NewAnomaly,
		ClockAnomalySeed != 0 ? ClockAnomalySeed : static_cast<int32>(GetTypeHash(GetFName())));
}

void AClock::ConfigureClockAnomaly(EClockAnomalyType NewAnomaly, int32 NewAnomalySeed)
{
	if (!HasAuthority())
	{
		return;
	}

	if (ClockAnomalyType == NewAnomaly && ClockAnomalySeed == NewAnomalySeed)
	{
		return;
	}

	ClockTimeAtAnchor = bClockAnchorInitialized
		? GetCurrentClockTime()
		: StartingTime.ToSeconds();
	ServerTimeAtAnchor = GetSynchronizedWorldTime();
	bClockAnchorInitialized = true;
	ClockAnomalyType = NewAnomaly;
	ClockAnomalySeed = NewAnomalySeed;

	switch (ClockAnomalyType)
	{
	case EClockAnomalyType::Fast:
		ClockSpeedMode = EClockSpeedMode::Fast;
		break;
	case EClockAnomalyType::Slow:
		ClockSpeedMode = EClockSpeedMode::Slow;
		break;
	default:
		ClockSpeedMode = EClockSpeedMode::Synchronized;
		break;
	}

	LastAudibleSecond = FMath::FloorToInt(ClockTimeAtAnchor);
	UpdateClockVisual();
	ForceNetUpdate();

	UE_LOG(LogClock, Log, TEXT("[%s] anomaly changed to %s (seed %d) for room %s."),
		*GetName(),
		*StaticEnum<EClockAnomalyType>()->GetNameStringByValue(static_cast<int64>(ClockAnomalyType)),
		ClockAnomalySeed,
		*GetNameSafe(OwningRoom));
}

void AClock::ClearClockAnomaly()
{
	ConfigureClockAnomaly(EClockAnomalyType::Normal, 0);
}

FString AClock::GetClockAnomalyDescription() const
{
	float Primary = 0.0f;
	float Secondary = 0.0f;
	ResolveAnomalyParameters(Primary, Secondary);
	return FString::Printf(
		TEXT("%s (Seed=%d, A=%.2f, B=%.2f)"),
		*StaticEnum<EClockAnomalyType>()->GetDisplayNameTextByValue(
			static_cast<int64>(ClockAnomalyType)).ToString(),
		ClockAnomalySeed,
		Primary,
		Secondary);
}

void AClock::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AClock, StartingTime);
	DOREPLIFETIME(AClock, InteractionResetTime);
	DOREPLIFETIME(AClock, ClockSpeedMode);
	DOREPLIFETIME(AClock, ClockAnomalyType);
	DOREPLIFETIME(AClock, ClockAnomalySeed);
	DOREPLIFETIME(AClock, OwningRoom);
	DOREPLIFETIME(AClock, ClockTimeAtAnchor);
	DOREPLIFETIME(AClock, ServerTimeAtAnchor);
}

void AClock::OnRep_ClockState()
{
	bClockAnchorInitialized = true;
	UpdateClockVisual();
}

void AClock::OnRep_OwningRoom()
{
	ResolveClockHands();
	UpdateClockVisual();
}

void AClock::MulticastPlayResetSound_Implementation()
{
	LastAudibleSecond = FMath::FloorToInt(InteractionResetTime.ToSeconds());
	if (!IsAudibleForLocalPlayer())
	{
		if (SecondHandSoundComponent && SecondHandSoundComponent->IsPlaying())
		{
			SecondHandSoundComponent->Stop();
		}
		return;
	}

	USoundBase* ResetSound = TimeResetSound ? TimeResetSound.Get() : UseSound.Get();
	if (ResetSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ResetSound, GetActorLocation());
	}
}

double AClock::GetSynchronizedWorldTime() const
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

float AClock::GetRateMultiplier(EClockSpeedMode Mode) const
{
	switch (Mode)
	{
	case EClockSpeedMode::Fast:
		return FastRateMultiplier;
	case EClockSpeedMode::Slow:
		return SlowRateMultiplier;
	case EClockSpeedMode::Synchronized:
	default:
		return 1.0f;
	}
}

EClockSpeedMode AClock::ResolveCursedSpeedMode() const
{
	switch (CursedRoomBehavior)
	{
	case EClockCursedBehavior::Slow:
		return EClockSpeedMode::Slow;
	case EClockCursedBehavior::Synchronized:
		return EClockSpeedMode::Synchronized;
	case EClockCursedBehavior::RandomDeviation:
		return (GetTypeHash(GetFName()) & 1u) != 0u
			? EClockSpeedMode::Fast
			: EClockSpeedMode::Slow;
	case EClockCursedBehavior::Fast:
	default:
		return EClockSpeedMode::Fast;
	}
}

USceneComponent* AClock::FindHandComponent(FName ComponentName) const
{
	TInlineComponentArray<USceneComponent*> SceneComponents(this);

	// Prefer the exact runtime component name. In particular, searching for
	// "Second" must never select the native "SecondHandSound" component.
	for (USceneComponent* SceneComponent : SceneComponents)
	{
		if (IsValid(SceneComponent)
			&& SceneComponent != SecondHandSoundComponent.Get()
			&& SceneComponent->GetFName() == ComponentName)
		{
			return SceneComponent;
		}
	}

	// Blueprint SCS templates may retain the _GEN_VARIABLE suffix at runtime.
	const FString GeneratedNamePrefix = ComponentName.ToString() + TEXT("_GEN_VARIABLE");
	for (USceneComponent* SceneComponent : SceneComponents)
	{
		if (IsValid(SceneComponent)
			&& SceneComponent != SecondHandSoundComponent.Get()
			&& SceneComponent->GetName().StartsWith(GeneratedNamePrefix))
		{
			return SceneComponent;
		}
	}

	return nullptr;
}

float AClock::GetAnomalousElapsedTime(double ElapsedRealSeconds) const
{
	const float Elapsed = static_cast<float>(ElapsedRealSeconds);
	float Primary = 0.0f;
	float Secondary = 0.0f;
	ResolveAnomalyParameters(Primary, Secondary);

	switch (ClockAnomalyType)
	{
	case EClockAnomalyType::Reverse:
		return -Elapsed * Primary;

	case EClockAnomalyType::Frozen:
		return 0.0f;

	case EClockAnomalyType::JumpForward:
		return FMath::FloorToFloat(Elapsed / Primary) * Secondary;

	case EClockAnomalyType::JumpBackward:
		return -FMath::FloorToFloat(Elapsed / Primary) * Secondary;

	case EClockAnomalyType::ErraticJumps:
	{
		const int32 Step = FMath::FloorToInt(Elapsed / Primary);
		if (Step <= 0)
		{
			return 0.0f;
		}
		const float SignedUnit = static_cast<float>(HashAnomalyStep(Step) % 2001u) / 1000.0f - 1.0f;
		return SignedUnit * Secondary;
	}

	case EClockAnomalyType::Stutter:
	{
		const float Period = Primary;
		const float MovingFraction = Secondary;
		const float MovingDuration = Period * MovingFraction;
		const float CompleteCycles = FMath::FloorToFloat(Elapsed / Period);
		const float CycleTime = FMath::Fmod(Elapsed, Period);
		return CompleteCycles * Period
			+ FMath::Min(CycleTime, MovingDuration) / MovingFraction;
	}

	case EClockAnomalyType::Fast:
	case EClockAnomalyType::Slow:
		return Elapsed * Primary;

	case EClockAnomalyType::Normal:
	default:
		return Elapsed;
	}
}

float AClock::GetAnomalyAudioRate() const
{
	float Primary = 1.0f;
	float Secondary = 1.0f;
	ResolveAnomalyParameters(Primary, Secondary);

	switch (ClockAnomalyType)
	{
	case EClockAnomalyType::Reverse:
	case EClockAnomalyType::Fast:
	case EClockAnomalyType::Slow:
		return FMath::Clamp(FMath::Abs(Primary), 0.25f, 4.0f);
	case EClockAnomalyType::Stutter:
		return FMath::Clamp(1.0f / Secondary, 0.25f, 4.0f);
	case EClockAnomalyType::JumpForward:
	case EClockAnomalyType::JumpBackward:
		return FMath::Clamp(Secondary / FMath::Max(Primary, 0.01f), 0.25f, 4.0f);
	default:
		return 1.0f;
	}
}

void AClock::ResolveAnomalyParameters(float& OutPrimary, float& OutSecondary) const
{
	FRandomStream Random(ClockAnomalySeed);
	const auto RandomRange = [&Random](const FVector2D& Range)
	{
		return Random.FRandRange(FMath::Min(Range.X, Range.Y), FMath::Max(Range.X, Range.Y));
	};

	OutPrimary = 1.0f;
	OutSecondary = 0.0f;
	switch (ClockAnomalyType)
	{
	case EClockAnomalyType::Reverse:
		OutPrimary = RandomRange(ReverseRateRange);
		break;
	case EClockAnomalyType::Fast:
		OutPrimary = RandomRange(AnomalousFastRateRange);
		break;
	case EClockAnomalyType::Slow:
		OutPrimary = RandomRange(AnomalousSlowRateRange);
		break;
	case EClockAnomalyType::JumpForward:
	case EClockAnomalyType::JumpBackward:
	case EClockAnomalyType::ErraticJumps:
		OutPrimary = FMath::Max(0.1f, RandomRange(JumpIntervalRange));
		OutSecondary = FMath::Max(1.0f, RandomRange(JumpAmountRange));
		break;
	case EClockAnomalyType::Stutter:
		OutPrimary = FMath::Max(0.2f, RandomRange(StutterPeriodRange));
		OutSecondary = Random.FRandRange(0.15f, 0.45f);
		break;
	default:
		break;
	}
}

uint32 AClock::HashAnomalyStep(int32 Step) const
{
	uint32 Value = HashCombineFast(GetTypeHash(ClockAnomalySeed), GetTypeHash(Step));
	Value ^= Value >> 16;
	Value *= 0x7feb352du;
	Value ^= Value >> 15;
	Value *= 0x846ca68bu;
	Value ^= Value >> 16;
	return Value;
}

void AClock::ResolveClockHands()
{
	// Live Coding can preserve the value chosen by the previous prefix-based
	// resolver. Clear that invalid cached self-reference before resolving again.
	if (SecondHandComponent.Get() == SecondHandSoundComponent.Get())
	{
		SecondHandComponent = nullptr;
	}

	if (!IsValid(HourHandComponent))
	{
		HourHandComponent = FindHandComponent(HourHandComponentName);
	}
	if (!IsValid(MinuteHandComponent))
	{
		MinuteHandComponent = FindHandComponent(MinuteHandComponentName);
	}
	if (!IsValid(SecondHandComponent))
	{
		SecondHandComponent = FindHandComponent(SecondHandComponentName);
	}
}

void AClock::UpdateClockVisual()
{
	ResolveClockHands();
	TimeNow = GetCurrentClockTime();
	ApplyHandRotations(TimeNow);

	const int32 CurrentWholeSecond = FMath::FloorToInt(TimeNow);
	if (!IsAudibleForLocalPlayer())
	{
		if (SecondHandSoundComponent && SecondHandSoundComponent->IsPlaying())
		{
			SecondHandSoundComponent->Stop();
		}
		LastAudibleSecond = CurrentWholeSecond;
		return;
	}

	if (LastAudibleSecond != INDEX_NONE && CurrentWholeSecond != LastAudibleSecond)
	{
		if (SecondHandSoundComponent)
		{
			SecondHandSoundComponent->PlaySecondHandTick(
				GetAnomalyAudioRate());
		}
	}
	LastAudibleSecond = CurrentWholeSecond;
}

void AClock::ApplyHandRotations(float SecondsSinceMidnight)
{
	const float WrappedSeconds = FMath::Fmod(
		FMath::Fmod(SecondsSinceMidnight, SecondsPerDay) + SecondsPerDay,
		SecondsPerDay);
	const float WholeSecond = FMath::FloorToFloat(WrappedSeconds);
	const float Second = FMath::Fmod(WholeSecond, 60.0f);
	const float Minute = FMath::Fmod(FMath::FloorToFloat(WholeSecond / 60.0f), 60.0f);
	const float Hour12 = FMath::Fmod(FMath::FloorToFloat(WholeSecond / 3600.0f), 12.0f);

	const float FractionalSecond = WrappedSeconds - WholeSecond;
	const float DisplaySecond = bSmoothHandMovement ? Second + FractionalSecond : Second;
	const float DisplayMinute = bSmoothHandMovement ? Minute + DisplaySecond / 60.0f : Minute;
	const float DisplayHour = bSmoothHandMovement ? Hour12 + DisplayMinute / 60.0f : Hour12;

	auto SetPitch = [this](USceneComponent* Hand, float Pitch)
	{
		if (IsValid(Hand) && Hand != SecondHandSoundComponent.Get())
		{
			FRotator Rotation = HandRotationOffset;
			Rotation.Pitch += Pitch;
			Hand->SetRelativeRotation(Rotation);
		}
	};

	// Same formulas as the submitted Blueprint graph.
	SetPitch(HourHandComponent, DisplayHour * -30.0f);
	SetPitch(MinuteHandComponent, DisplayMinute * -6.0f);
	SetPitch(SecondHandComponent, DisplaySecond * -6.0f);
}

void AClock::UpdateSecondHandSoundAttachment()
{
	if (!SecondHandSoundComponent || GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (IsValid(SecondHandComponent)
		&& SecondHandComponent.Get() != SecondHandSoundComponent.Get()
		&& SecondHandSoundComponent->GetAttachParent() != SecondHandComponent)
	{
		SecondHandSoundComponent->AttachToComponent(
			SecondHandComponent,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
}

bool AClock::IsAudibleForLocalPlayer() const
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	const APlayerController* LocalPlayerController = World
		? World->GetFirstPlayerController()
		: nullptr;
	const AHronoCharacter* LocalPlayer = LocalPlayerController
		? Cast<AHronoCharacter>(LocalPlayerController->GetPawn())
		: nullptr;

	if (!LocalPlayer || !LocalPlayer->IsLocallyControlled())
	{
		return false;
	}

	return ItemTimeline == EItemTimeline::Both
		|| ItemTimeline == LocalPlayer->GetTimeline();
}

void AClock::SetCurrentTimeSeconds(float NewTimeSeconds)
{
	ClockTimeAtAnchor = FMath::Fmod(
		FMath::Fmod(NewTimeSeconds, SecondsPerDay) + SecondsPerDay,
		SecondsPerDay);
	ServerTimeAtAnchor = GetSynchronizedWorldTime();
	bClockAnchorInitialized = true;
	LastAudibleSecond = FMath::FloorToInt(ClockTimeAtAnchor);
	UpdateClockVisual();
	ForceNetUpdate();
}
