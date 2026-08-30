#include "Enviroment/DoorLockTrigger.h"

#include "Components/BoxComponent.h"
#include "HronoCharacter.h"
#include "Items/Drag_Item.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(LogDoorLockTrigger, Log, All);

ADoorLockTrigger::ADoorLockTrigger()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(false);

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;
	TriggerBox->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
	TriggerBox->SetGenerateOverlapEvents(true);
}

void ADoorLockTrigger::BeginPlay()
{
	Super::BeginPlay();
	TriggerBox->OnComponentBeginOverlap.AddUniqueDynamic(
		this,
		&ADoorLockTrigger::HandleTriggerBeginOverlap);
}

void ADoorLockTrigger::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		ReleaseOwnedLocks();
	}

	Super::EndPlay(EndPlayReason);
}

void ADoorLockTrigger::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADoorLockTrigger, bHasTriggered);
	DOREPLIFETIME(ADoorLockTrigger, bDoorsUnlocked);
	DOREPLIFETIME(ADoorLockTrigger, LastTriggeringActor);
}

void ADoorLockTrigger::HandleTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!HasAuthority() || !IsValid(Cast<AHronoCharacter>(OtherActor)))
	{
		return;
	}

	ActivateDoorLockWithActor(OtherActor);
}

bool ADoorLockTrigger::ActivateDoorLock()
{
	return ActivateDoorLockWithActor(nullptr);
}

bool ADoorLockTrigger::ActivateDoorLockWithActor(AActor* TriggeringActor)
{
	if (!HasAuthority())
	{
		UE_LOG(LogDoorLockTrigger, Warning,
			TEXT("ActivateDoorLock ignored for %s because it was not called on the server"),
			*GetName());
		return false;
	}

	// The Blueprint event describes the overlap/action request, not whether the
	// doors changed state. It must remain usable after a one-shot lock or unlock.
	LastTriggeringActor = TriggeringActor;
	MulticastNotifyTriggered(TriggeringActor);
	ForceNetUpdate();

	if (bDoorsUnlocked || (bTriggerOnlyOnce && bHasTriggered))
	{
		return false;
	}

	bHasTriggered = true;
	int32 LockedDoorCount = 0;

	for (ADrag_Item* Door : Doors)
	{
		if (!IsValid(Door) || TriggeredDoors.Contains(Door))
		{
			continue;
		}

		TriggeredDoors.Add(Door);
		Door->RegisterDoorTriggerLock(true);
		++LockedDoorCount;

		if (bCloseDoorsWhenTriggered)
		{
			Door->AnimateDoor(false);
		}
	}

	UE_LOG(LogDoorLockTrigger, Log,
		TEXT("%s activated and locked %d door(s)"),
		*GetName(),
		LockedDoorCount);

	OnDoorsTriggered.Broadcast();
	ForceNetUpdate();
	return true;
}

void ADoorLockTrigger::MulticastNotifyTriggered_Implementation(AActor* TriggeringActor)
{
	OnTriggered(TriggeringActor);
}

bool ADoorLockTrigger::UnlockTriggeredDoors()
{
	if (!HasAuthority())
	{
		UE_LOG(LogDoorLockTrigger, Warning,
			TEXT("UnlockTriggeredDoors ignored for %s because it was not called on the server"),
			*GetName());
		return false;
	}

	if (bDoorsUnlocked)
	{
		return false;
	}

	bDoorsUnlocked = true;
	ReleaseOwnedLocks();

	UE_LOG(LogDoorLockTrigger, Log,
		TEXT("%s released all of its door locks"),
		*GetName());

	OnDoorsUnlocked.Broadcast();
	ForceNetUpdate();
	return true;
}

void ADoorLockTrigger::ReleaseOwnedLocks()
{
	for (ADrag_Item* Door : TriggeredDoors)
	{
		if (IsValid(Door))
		{
			Door->RegisterDoorTriggerLock(false);
		}
	}

	TriggeredDoors.Reset();
}
