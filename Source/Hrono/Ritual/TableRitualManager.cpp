#include "Ritual/TableRitualManager.h"

#include "Enviroment/DoorLockTrigger.h"
#include "Enviroment/OuijaBoard.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/GameStateBase.h"
#include "HronoCharacter.h"
#include "Items/Chair.h"
#include "Items/RitualBottle.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Ritual/RitualCandleActor.h"
#include "Ritual/TableRitualGate.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogTableRitual, Log, All);

ATableRitualManager::ATableRitualManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(false);
	SetNetUpdateFrequency(10.0f);
	static ConstructorHelpers::FObjectFinder<USoundBase> ChairCreak(
		TEXT("/Game/HorrorEngine/Audio/Interactions/S_Creak_06.S_Creak_06"));
	ChairSlideSound = ChairCreak.Object;
}

void ATableRitualManager::BeginPlay()
{
	Super::BeginPlay();
	if (IsValid(OuijaBoard))
	{
		BoardInitialRotation = OuijaBoard->GetActorRotation();
		OuijaBoard->SetReplicateMovement(false);
	}
	if (IsValid(SlidingChair))
	{
		SlidingChair->SetReplicateMovement(false);
	}
	if (HasAuthority())
	{
		RitualState.AttemptsRemaining = FMath::Max(1, InitialAttempts);
		RitualState.BoardYaw = BoardInitialRotation.Yaw;
		if (IsValid(TableChairA))
		{
			TableChairA->SetReplicates(true);
			TableChairA->bAlwaysRelevant = true;
			TableChairA->OnCharacterSat.AddUniqueDynamic(this, &ATableRitualManager::HandleCharacterSat);
		}
		if (IsValid(TableChairB))
		{
			TableChairB->SetReplicates(true);
			TableChairB->bAlwaysRelevant = true;
			TableChairB->OnCharacterSat.AddUniqueDynamic(this, &ATableRitualManager::HandleCharacterSat);
		}
		if (IsValid(RitualBottle))
		{
			RitualBottle->OnVictimSelected.AddUniqueDynamic(
				this, &ATableRitualManager::HandleBottleVictimSelected);
		}
		ForceNetUpdate();
	}
	ApplyPresentation();
}

void ATableRitualManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(PhaseTimer);
	GetWorldTimerManager().ClearTimer(VictimTimeoutTimer);
	if (IsValid(TableChairA))
	{
		TableChairA->OnCharacterSat.RemoveDynamic(this, &ATableRitualManager::HandleCharacterSat);
	}
	if (IsValid(TableChairB))
	{
		TableChairB->OnCharacterSat.RemoveDynamic(this, &ATableRitualManager::HandleCharacterSat);
	}
	if (IsValid(RitualBottle))
	{
		RitualBottle->OnVictimSelected.RemoveDynamic(
			this, &ATableRitualManager::HandleBottleVictimSelected);
	}
	Super::EndPlay(EndPlayReason);
}

void ATableRitualManager::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATableRitualManager, RitualState);
}

double ATableRitualManager::GetServerTime() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return 0.0;
	}
	if (const AGameStateBase* GameState = World->GetGameState())
	{
		return GameState->GetServerWorldTimeSeconds();
	}
	return World->GetTimeSeconds();
}

bool ATableRitualManager::HaveBothSeatedPlayers() const
{
	const AHronoCharacter* A = IsValid(TableChairA) ? TableChairA->GetSitter() : nullptr;
	const AHronoCharacter* B = IsValid(TableChairB) ? TableChairB->GetSitter() : nullptr;
	return IsValid(A) && IsValid(B) && A != B
		&& A->IsSittingOnChair() && B->IsSittingOnChair()
		&& A->GetCurrentChair() == TableChairA
		&& B->GetCurrentChair() == TableChairB
		&& TableRitualGate::AreAllPlayersSeatedAtRitualTable(this);
}

void ATableRitualManager::HandleCharacterSat(AHronoCharacter* Character)
{
	if (HasAuthority() && IsValid(Character)
		&& RitualState.Phase == ETableRitualPhase::Idle)
	{
		TryStartTableRitual();
	}
}

bool ATableRitualManager::TryStartTableRitual()
{
	if (!HasAuthority() || RitualState.Phase != ETableRitualPhase::Idle
		|| !TableRitualGate::IsUnlocked(this) || !HaveBothSeatedPlayers()
		|| !IsValid(SlidingChair) || !IsValid(RitualBottle)
		|| !IsValid(RitualCandle) || !IsValid(OuijaBoard)
		|| !IsValid(VictimRitualPoint))
	{
		return false;
	}

	TableChairA->SetRitualStarted(true);
	TableChairB->SetRitualStarted(true);
	SetPhase(ETableRitualPhase::Preparing);
	GetWorldTimerManager().SetTimer(PhaseTimer, this,
		&ATableRitualManager::BeginChairSlide, FMath::FRandRange(1.0f, 3.0f), false);
	return true;
}

void ATableRitualManager::AcceptBottleVictimChoiceLegacy(bool bSecondVictim)
{
	if (HasAuthority() && IsValid(RitualBottle))
	{
		const int32 ExpectedIndex = bSecondVictim ? 1 : 0;
		if (RitualBottle->GetSelectedVictimIndex() == ExpectedIndex)
		{
			HandleBottleVictimSelected(RitualBottle->GetSelectedVictim(), ExpectedIndex);
		}
	}
}

void ATableRitualManager::CompleteVictimReturnLegacy()
{
	if (HasAuthority() && IsValid(RitualState.Victim))
	{
		CompleteVictimReturn(RitualState.Victim);
	}
}

void ATableRitualManager::SetPhase(ETableRitualPhase NewPhase)
{
	RitualState.Phase = NewPhase;
	ApplyPresentation();
	ForceNetUpdate();
	UE_LOG(LogTableRitual, Log, TEXT("%s phase=%d attempts=%d victim=%s"),
		*GetName(), static_cast<int32>(NewPhase), RitualState.AttemptsRemaining,
		*GetNameSafe(RitualState.Victim));
}

void ATableRitualManager::ResetToIdle()
{
	GetWorldTimerManager().ClearTimer(PhaseTimer);
	GetWorldTimerManager().ClearTimer(VictimTimeoutTimer);
	if (IsValid(TableChairA)) TableChairA->SetRitualStarted(false);
	if (IsValid(TableChairB)) TableChairB->SetRitualStarted(false);
	for (AChair* Chair : {TableChairA.Get(), TableChairB.Get()})
	{
		if (IsValid(Chair) && !IsValid(Chair->GetSitter()))
		{
			Chair->bIsSit = false;
			Chair->SetSitter(nullptr);
			Chair->ForceNetUpdate();
		}
	}
	RitualState.Victim = nullptr;
	RitualState.VictimIndex = INDEX_NONE;
	bVictimReturnHandled = false;
	SetPhase(ETableRitualPhase::Idle);
}

void ATableRitualManager::BeginChairSlide()
{
	if (!HasAuthority() || RitualState.Phase != ETableRitualPhase::Preparing)
	{
		return;
	}
	if (!HaveBothSeatedPlayers() || !IsValid(SlidingChair))
	{
		ResetToIdle();
		return;
	}
	RitualState.ChairStart = SlidingChair->GetActorLocation();
	RitualState.ChairTarget = RitualState.ChairStart
		- SlidingChair->GetActorForwardVector() * FMath::Max(0.0f, ChairSlideDistance);
	RitualState.ChairSlideDuration = FMath::Max(0.05f, ChairSlideDuration);
	RitualState.ChairSlideServerTime = GetServerTime();
	SetPhase(ETableRitualPhase::SlidingChair);
	MulticastChairSlideCreak();
	GetWorldTimerManager().SetTimer(PhaseTimer, this,
		&ATableRitualManager::FinishChairSlide, RitualState.ChairSlideDuration, false);
}

void ATableRitualManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (RitualState.Phase != ETableRitualPhase::SlidingChair || !IsValid(SlidingChair))
	{
		return;
	}
	const float Alpha = FMath::Clamp(static_cast<float>(
		(GetServerTime() - RitualState.ChairSlideServerTime)
		/ FMath::Max(0.05f, RitualState.ChairSlideDuration)), 0.0f, 1.0f);
	const float Eased = FMath::InterpEaseInOut(0.0f, 1.0f, Alpha, 2.0f);
	SlidingChair->SetActorLocation(FMath::Lerp(
		RitualState.ChairStart, RitualState.ChairTarget, Eased));
}

void ATableRitualManager::FinishChairSlide()
{
	if (!HasAuthority() || RitualState.Phase != ETableRitualPhase::SlidingChair)
	{
		return;
	}
	if (IsValid(SlidingChair)) SlidingChair->SetActorLocation(RitualState.ChairTarget);
	SetPhase(ETableRitualPhase::LightingCandles);
	GetWorldTimerManager().SetTimer(PhaseTimer, this,
		&ATableRitualManager::BeginCandleLighting, FMath::FRandRange(1.0f, 3.0f), false);
}

void ATableRitualManager::BeginCandleLighting()
{
	if (!HasAuthority() || RitualState.Phase != ETableRitualPhase::LightingCandles)
	{
		return;
	}
	if (IsValid(RitualCandle)) RitualCandle->StartRitualLighting();
	GetWorldTimerManager().SetTimer(PhaseTimer, this,
		&ATableRitualManager::BeginBottleSpin, FMath::FRandRange(3.0f, 5.0f), false);
}

void ATableRitualManager::BeginBottleSpin()
{
	if (!HasAuthority() || (RitualState.Phase != ETableRitualPhase::LightingCandles
		&& RitualState.Phase != ETableRitualPhase::Retrying))
	{
		return;
	}
	if (!HaveBothSeatedPlayers() || !IsValid(RitualBottle))
	{
		ResetToIdle();
		return;
	}
	// Preserve the authored bottle arrow mapping: first points to ChairB, second to ChairA.
	if (RitualBottle->SpinBottle(TableChairB->GetSitter(), TableChairA->GetSitter()))
	{
		SetPhase(ETableRitualPhase::SpinningBottle);
	}
	else
	{
		ResetToIdle();
	}
}

void ATableRitualManager::HandleBottleVictimSelected(
	AHronoCharacter* Character, int32 VictimIndex)
{
	if (!HasAuthority() || RitualState.Phase != ETableRitualPhase::SpinningBottle
		|| !IsValid(Character) || !IsValid(VictimRitualPoint)
		|| !IsValid(RitualBottle)
		|| Character != RitualBottle->GetSelectedVictim()
		|| VictimIndex != RitualBottle->GetSelectedVictimIndex()
		|| !Character->MoveFromChairToRitualPoint(VictimRitualPoint))
	{
		return;
	}
	bVictimReturnHandled = false;
	RitualState.Victim = Character;
	RitualState.VictimIndex = VictimIndex;
	RitualState.bBoardVisible = true;
	RitualState.BoardYaw = VictimIndex == 0 ? 0.0f : 180.0f;
	SetPhase(ETableRitualPhase::VictimChosen);
	GetWorldTimerManager().SetTimer(VictimTimeoutTimer, this,
		&ATableRitualManager::HandleVictimReturnTimeout,
		FMath::Max(5.0f, VictimReturnTimeout), false);
}

bool ATableRitualManager::IsSelectedVictim(const AHronoCharacter* Character) const
{
	return HasAuthority() && RitualState.Phase == ETableRitualPhase::VictimChosen
		&& !bVictimReturnHandled && IsValid(Character)
		&& RitualState.Victim == Character;
}

bool ATableRitualManager::CompleteVictimReturn(AHronoCharacter* Character)
{
	if (!IsSelectedVictim(Character)
		|| !Character->bIsAtRitualPoint
		|| !IsValid(Character->GetReservedRitualChair())
		|| !Character->ReturnToReservedRitualChair())
	{
		return false;
	}
	bVictimReturnHandled = true;
	GetWorldTimerManager().ClearTimer(VictimTimeoutTimer);
	for (TActorIterator<ADoorLockTrigger> It(GetWorld()); It; ++It)
	{
		if (IsValid(*It)) It->UnlockTriggeredDoors();
	}
	RitualState.AttemptsRemaining = FMath::Max(0, RitualState.AttemptsRemaining - 1);
	RitualState.Victim = nullptr;
	RitualState.VictimIndex = INDEX_NONE;
	if (RitualState.AttemptsRemaining == 0)
	{
		SetPhase(ETableRitualPhase::Exhausted);
		if (IsValid(OuijaBoard)) OuijaBoard->TypeWordAutomatically(TEXT("Death"), true, false);
	}
	else
	{
		SetPhase(ETableRitualPhase::Retrying);
		GetWorldTimerManager().SetTimer(PhaseTimer, this,
			&ATableRitualManager::BeginRetry, FMath::FRandRange(1.0f, 3.0f), false);
	}
	return true;
}

void ATableRitualManager::BeginRetry()
{
	if (!HasAuthority() || RitualState.Phase != ETableRitualPhase::Retrying) return;
	if (IsValid(RitualCandle)) RitualCandle->ReportRitualMistake();
	GetWorldTimerManager().SetTimer(PhaseTimer, this,
		&ATableRitualManager::FinishRetryDelay, 1.0f, false);
}

void ATableRitualManager::FinishRetryDelay()
{
	BeginBottleSpin();
}

void ATableRitualManager::HandleVictimReturnTimeout()
{
	if (!HasAuthority() || RitualState.Phase != ETableRitualPhase::VictimChosen) return;
	AHronoCharacter* Character = RitualState.Victim;
	if (IsValid(Character) && Character->bIsAtRitualPoint
		&& CompleteVictimReturn(Character))
	{
		UE_LOG(LogTableRitual, Warning,
			TEXT("%s recovered timed-out victim return for %s"),
			*GetName(), *GetNameSafe(Character));
		return;
	}
	ResetToIdle();
}

void ATableRitualManager::OnRep_RitualState()
{
	if (HasActorBegunPlay()) ApplyPresentation();
}

void ATableRitualManager::ApplyPresentation()
{
	SetActorTickEnabled(RitualState.Phase == ETableRitualPhase::SlidingChair);
	if (IsValid(OuijaBoard))
	{
		OuijaBoard->SetActorHiddenInGame(!RitualState.bBoardVisible);
		FRotator Rotation = BoardInitialRotation;
		Rotation.Yaw = RitualState.BoardYaw;
		OuijaBoard->SetActorRotation(Rotation);
	}
	if (IsValid(SlidingChair) && RitualState.ChairSlideDuration > 0.0f
		&& RitualState.Phase != ETableRitualPhase::SlidingChair)
	{
		SlidingChair->SetActorLocation(RitualState.ChairTarget);
	}
}

void ATableRitualManager::MulticastChairSlideCreak_Implementation()
{
	if (IsValid(ChairSlideSound) && IsValid(SlidingChair))
	{
		UGameplayStatics::PlaySoundAtLocation(
			this, ChairSlideSound, SlidingChair->GetActorLocation());
	}
}
