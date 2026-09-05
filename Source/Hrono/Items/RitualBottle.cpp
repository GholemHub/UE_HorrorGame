#include "Items/RitualBottle.h"

#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/GameStateBase.h"
#include "HronoCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Ritual/RitualChairAudioReplication.h"

ARitualBottle::ARitualBottle()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(false);
	SetNetUpdateFrequency(30.0f);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	BottlePivot = CreateDefaultSubobject<USceneComponent>(TEXT("BottlePivot"));
	BottlePivot->SetupAttachment(SceneRoot);

	BottleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BottleMesh"));
	BottleMesh->SetupAttachment(BottlePivot);
	BottleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BottleMesh->SetEnableGravity(false);
	BottleMesh->SetSimulatePhysics(false);

	SelectionDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("SelectionDirection"));
	SelectionDirection->SetupAttachment(BottlePivot);
	SelectionDirection->ArrowColor = FColor::Yellow;
	SelectionDirection->ArrowSize = 1.25f;
	SelectionDirection->SetHiddenInGame(true);

	FirstVictimDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("FirstVictimDirection"));
	FirstVictimDirection->SetupAttachment(SceneRoot);
	FirstVictimDirection->ArrowColor = FColor::Green;
	FirstVictimDirection->ArrowSize = 1.5f;
	FirstVictimDirection->SetHiddenInGame(true);

	SecondVictimDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("SecondVictimDirection"));
	SecondVictimDirection->SetupAttachment(SceneRoot);
	SecondVictimDirection->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	SecondVictimDirection->ArrowColor = FColor::Blue;
	SecondVictimDirection->ArrowSize = 1.5f;
	SecondVictimDirection->SetHiddenInGame(true);
}

void ARitualBottle::BeginPlay()
{
	Super::BeginPlay();

	// Blueprint defaults cannot accidentally re-enable Chaos for this actor.
	BottleMesh->SetSimulatePhysics(false);
	BottleMesh->SetEnableGravity(false);
	InitialPivotRotation = BottlePivot->GetRelativeRotation();

	if (SpinState.SequenceId > 0 && IsValid(SpinState.SelectedVictim))
	{
		OnRep_SpinState();
	}
}

void ARitualBottle::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Keep the visual deterministic even if an old Blueprint node tries to turn
	// physics back on after BeginPlay.
	if (BottleMesh->IsSimulatingPhysics())
	{
		BottleMesh->SetSimulatePhysics(false);
		BottleMesh->SetEnableGravity(false);
	}

	if (!SpinState.bIsSpinning)
	{
		return;
	}

	const double CurrentServerTime = GetSynchronizedWorldTime();
	ApplySpinAtTime(CurrentServerTime);

	const double Elapsed = CurrentServerTime - SpinState.StartServerTime;
	if (Elapsed < static_cast<double>(SpinState.Duration))
	{
		return;
	}

	ApplyFinalRotation();
	if (HasAuthority())
	{
		SpinState.bIsSpinning = false;
		ForceNetUpdate();
	}

	// On the server the state is already marked as stopped when Blueprint receives
	// the event, so it may safely start the next ritual step or another spin.
	HandleSpinCompleted();
}

bool ARitualBottle::SpinBottle(AHronoCharacter* FirstVictim, AHronoCharacter* SecondVictim)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[RitualBottle] Spin Bottle ignored for %s because it was not called on the server"),
			*GetName());
		return false;
	}

	if (SpinState.bIsSpinning)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[RitualBottle] %s is already spinning"),
			*GetName());
		return false;
	}

	const bool bFirstIsValid = IsValid(FirstVictim);
	const bool bSecondIsValid = IsValid(SecondVictim);
	if (!bFirstIsValid && !bSecondIsValid)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[RitualBottle] %s cannot spin because both victims are invalid"),
			*GetName());
		return false;
	}

	const bool bChooseFirst = bFirstIsValid && (!bSecondIsValid || FMath::RandBool());
	AHronoCharacter* ChosenVictim = bChooseFirst ? FirstVictim : SecondVictim;
	UArrowComponent* ChosenDirection = bChooseFirst ? FirstVictimDirection : SecondVictimDirection;

	const int32 SafeMinRotations = FMath::Max(1, MinFullRotations);
	const int32 SafeMaxRotations = FMath::Max(SafeMinRotations, MaxFullRotations);
	const int32 FullRotations = FMath::RandRange(SafeMinRotations, SafeMaxRotations);

	const float Jitter = FMath::FRandRange(
		-FMath::Max(0.0f, FinalAngleJitter),
		FMath::Max(0.0f, FinalAngleJitter));
	const float CurrentSelectionYaw = SelectionDirection->GetComponentRotation().Yaw;
	const float DesiredSelectionYaw = ChosenDirection->GetComponentRotation().Yaw + Jitter;
	const float RemainingDegrees = FMath::Fmod(
		FMath::UnwindDegrees(DesiredSelectionYaw - CurrentSelectionYaw) + 360.0f,
		360.0f);

	++SpinState.SequenceId;
	SpinState.bIsSpinning = true;
	SpinState.StartServerTime = GetSynchronizedWorldTime();
	SpinState.Duration = FMath::Max(0.1f, SpinDuration);
	SpinState.StartPivotYaw = BottlePivot->GetRelativeRotation().Yaw;
	SpinState.TotalSpinDegrees = static_cast<float>(FullRotations) * 360.0f + RemainingDegrees;
	SpinState.EasingExponent = FMath::Max(1.0f, DecelerationExponent);
	SpinState.SpinWobbleAmplitude = FMath::Max(0.0f, WobbleAmplitude);
	SpinState.SpinWobbleCycles = FMath::Max(0.0f, WobbleCycles);
	SpinState.SelectedVictim = ChosenVictim;
	SpinState.SelectedVictimIndex = bChooseFirst ? 0 : 1;

	LastCompletedSequence = INDEX_NONE;
	HandleSpinStarted();
	ForceNetUpdate();

	UE_LOG(LogTemp, Log,
		TEXT("[RitualBottle] %s started spin %d towards victim %s (index %d, %.1f degrees)"),
		*GetName(),
		SpinState.SequenceId,
		*GetNameSafe(ChosenVictim),
		SpinState.SelectedVictimIndex,
		SpinState.TotalSpinDegrees);
	return true;
}

void ARitualBottle::ResetBottle()
{
	if (!HasAuthority())
	{
		return;
	}

	SpinState.bIsSpinning = false;
	SpinState.StartServerTime = 0.0;
	SpinState.Duration = 0.0f;
	SpinState.StartPivotYaw = InitialPivotRotation.Yaw;
	SpinState.TotalSpinDegrees = 0.0f;
	SpinState.EasingExponent = FMath::Max(1.0f, DecelerationExponent);
	SpinState.SpinWobbleAmplitude = 0.0f;
	SpinState.SpinWobbleCycles = 0.0f;
	SpinState.SelectedVictim = nullptr;
	SpinState.SelectedVictimIndex = INDEX_NONE;
	BottlePivot->SetRelativeRotation(InitialPivotRotation);
	LastStartedSequence = INDEX_NONE;
	LastCompletedSequence = INDEX_NONE;
	ForceNetUpdate();
}

void ARitualBottle::ClearSelectedVictim()
{
	if (!HasAuthority() || SpinState.bIsSpinning)
	{
		return;
	}

	SpinState.SelectedVictim = nullptr;
	SpinState.SelectedVictimIndex = INDEX_NONE;
	ForceNetUpdate();
}

void ARitualBottle::OnRep_SpinState()
{
	if (!IsValid(SpinState.SelectedVictim))
	{
		if (!SpinState.bIsSpinning && FMath::IsNearlyZero(SpinState.TotalSpinDegrees))
		{
			BottlePivot->SetRelativeRotation(InitialPivotRotation);
		}
		return;
	}

	HandleSpinStarted();
	if (SpinState.bIsSpinning)
	{
		ApplySpinAtTime(GetSynchronizedWorldTime());
	}
	else
	{
		ApplyFinalRotation();
		HandleSpinCompleted();
	}
}

double ARitualBottle::GetSynchronizedWorldTime() const
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

void ARitualBottle::ApplySpinAtTime(double CurrentServerTime)
{
	const float Duration = FMath::Max(0.1f, SpinState.Duration);
	const float LinearAlpha = FMath::Clamp(
		static_cast<float>((CurrentServerTime - SpinState.StartServerTime) / Duration),
		0.0f,
		1.0f);
	const float Exponent = FMath::Max(1.0f, SpinState.EasingExponent);
	const float EasedAlpha = 1.0f - FMath::Pow(1.0f - LinearAlpha, Exponent);

	const float Wobble = FMath::Sin(EasedAlpha * SpinState.SpinWobbleCycles * 2.0f * PI)
		* SpinState.SpinWobbleAmplitude
		* (1.0f - EasedAlpha);
	const float NewYaw = SpinState.StartPivotYaw
		+ SpinState.TotalSpinDegrees * EasedAlpha
		+ Wobble;

	FRotator PivotRotation = BottlePivot->GetRelativeRotation();
	PivotRotation.Yaw = NewYaw;
	BottlePivot->SetRelativeRotation(PivotRotation);
}

void ARitualBottle::ApplyFinalRotation()
{
	FRotator PivotRotation = BottlePivot->GetRelativeRotation();
	PivotRotation.Yaw = SpinState.StartPivotYaw + SpinState.TotalSpinDegrees;
	BottlePivot->SetRelativeRotation(PivotRotation);
}

void ARitualBottle::HandleSpinStarted()
{
	if (LastStartedSequence == SpinState.SequenceId)
	{
		return;
	}

	LastStartedSequence = SpinState.SequenceId;
	PlayRitualChairStartAudioForRemoteClient(this, GetActorLocation());
	OnSpinStarted.Broadcast(this);
	BP_OnBottleSpinStarted();
}

void ARitualBottle::HandleSpinCompleted()
{
	AHronoCharacter* CompletedVictim = SpinState.SelectedVictim;
	const int32 CompletedVictimIndex = SpinState.SelectedVictimIndex;
	const int32 CompletedSequence = SpinState.SequenceId;
	if (LastCompletedSequence == CompletedSequence || !IsValid(CompletedVictim))
	{
		return;
	}

	LastCompletedSequence = CompletedSequence;
	OnVictimSelected.Broadcast(CompletedVictim, CompletedVictimIndex);
	BP_OnBottleVictimSelected(CompletedVictim, CompletedVictimIndex);
	OnSpinFinished.Broadcast(CompletedVictim, CompletedVictimIndex);
	BP_OnBottleSpinFinished(CompletedVictim, CompletedVictimIndex);

	UE_LOG(LogTemp, Log,
		TEXT("[RitualBottle] %s finished spin %d. Selected victim: %s (index %d)"),
		*GetName(),
		CompletedSequence,
		*GetNameSafe(CompletedVictim),
		CompletedVictimIndex);
}

void ARitualBottle::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARitualBottle, SpinState);
}
