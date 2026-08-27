#include "Items/Dozimetr.h"
#include "Items/HotDot.h"
#include "HronoCharacter.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ADozimetr::ADozimetr()
{
	static ConstructorHelpers::FObjectFinder<USoundBase> DefaultBeepSound(
		TEXT("/Game/HorrorEngine/Audio/Elecronics/S_Beep_Dozimetr.S_Beep_Dozimetr"));
	if (DefaultBeepSound.Succeeded())
	{
		BeepSound = DefaultBeepSound.Object;
	}
}

void ADozimetr::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Also reconcile outside RepNotify. This covers listen-server ownership transfers
	// and makes the detector resilient to owner/state replication arriving in any order.
	RefreshLocalDetectionState();
}

void ADozimetr::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADozimetr, bDosimeterActive);
}

void ADozimetr::On()
{
	if (!HasAuthority())
	{
		return;
	}

	bDosimeterActive = true;
	RefreshLocalDetectionState();
	ForceNetUpdate();
}

void ADozimetr::Off()
{
	if (!HasAuthority())
	{
		return;
	}

	bDosimeterActive = false;
	RefreshLocalDetectionState();
	ForceNetUpdate();
}

void ADozimetr::OnRep_DosimeterActive()
{
	RefreshLocalDetectionState();
}

void ADozimetr::OnRep_OwningCharacter(AHronoCharacter* PreviousOwningCharacter)
{
	Super::OnRep_OwningCharacter(PreviousOwningCharacter);
	RefreshLocalDetectionState();
}

void ADozimetr::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopLocalDetection();
	Super::EndPlay(EndPlayReason);
}

void ADozimetr::RefreshLocalDetectionState(bool bPlayTransitionSound)
{
	const bool bShouldRunLocally = bDosimeterActive && IsHeldByLocalPlayer();
	if (bShouldRunLocally == bLocalDetectionRunning)
	{
		return;
	}

	if (bShouldRunLocally)
	{
		if (bPlayTransitionSound && TurnOnSound)
		{
			UGameplayStatics::PlaySound2D(this, TurnOnSound);
		}
		StartLocalDetection();
	}
	else
	{
		StopLocalDetection();
		if (bPlayTransitionSound && TurnOffSound)
		{
			UGameplayStatics::PlaySound2D(this, TurnOffSound);
		}
	}
}

AHotDot* ADozimetr::FindClosestHotDot() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	AHotDot* ClosestHotDot = nullptr;
	float ClosestDistanceSquared = TNumericLimits<float>::Max();
	const FVector DetectorLocation = GetActorLocation();
	const AHronoCharacter* Character = Cast<AHronoCharacter>(OwningCharacter);
	const EItemTimeline DetectorTimeline = Character
		? Character->GetTimeline()
		: ItemTimeline;

	for (TActorIterator<AHotDot> It(World); It; ++It)
	{
		AHotDot* HotDot = *It;
		if (!IsValid(HotDot) || !HotDot->IsDetectableByDosimeter(DetectorTimeline))
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(
			DetectorLocation,
			HotDot->GetActorLocation());
		if (DistanceSquared < ClosestDistanceSquared)
		{
			ClosestDistanceSquared = DistanceSquared;
			ClosestHotDot = HotDot;
		}
	}

	return ClosestHotDot;
}

void ADozimetr::StartLocalDetection()
{
	if (!bDosimeterActive || !IsHeldByLocalPlayer())
	{
		return;
	}

	bLocalDetectionRunning = true;
	GetWorldTimerManager().ClearTimer(BeepTimerHandle);
	HandleBeep();
}

void ADozimetr::StopLocalDetection()
{
	bLocalDetectionRunning = false;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BeepTimerHandle);
	}
}

void ADozimetr::HandleBeep()
{
	if (!bDosimeterActive || !bLocalDetectionRunning || !IsHeldByLocalPlayer())
	{
		StopLocalDetection();
		return;
	}

	float NextInterval = FMath::Max(MaximumBeepInterval, 0.01f);
	if (const AHotDot* ClosestHotDot = FindClosestHotDot())
	{
		const float Distance = FVector::Distance(
			GetActorLocation(),
			ClosestHotDot->GetActorLocation());
		NextInterval = CalculateBeepInterval(Distance);
		if (BeepSound)
		{
			UGameplayStatics::PlaySound2D(this, BeepSound);
		}
	}

	GetWorldTimerManager().SetTimer(
		BeepTimerHandle,
		this,
		&ADozimetr::HandleBeep,
		NextInterval,
		false);
}

float ADozimetr::CalculateBeepInterval(float Distance) const
{
	const float NearDistance = FMath::Min(MinimumBeepDistance, MaximumBeepDistance);
	const float FarDistance = FMath::Max(MinimumBeepDistance, MaximumBeepDistance);
	const float FastInterval = FMath::Min(MinimumBeepInterval, MaximumBeepInterval);
	const float SlowInterval = FMath::Max(MinimumBeepInterval, MaximumBeepInterval);

	return FMath::GetMappedRangeValueClamped(
		FVector2D(NearDistance, FarDistance),
		FVector2D(FastInterval, SlowInterval),
		Distance);
}

bool ADozimetr::IsHeldByLocalPlayer() const
{
	const AHronoCharacter* Character = Cast<AHronoCharacter>(OwningCharacter);
	const APlayerController* PlayerController = Character
		? Cast<APlayerController>(Character->GetController())
		: nullptr;
	return PlayerController && PlayerController->IsLocalController();
}
