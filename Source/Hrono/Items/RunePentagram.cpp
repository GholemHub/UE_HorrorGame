#include "Items/RunePentagram.h"

#include "HronoCharacter.h"
#include "HronoCollisionChannels.h"
#include "Items/Base_Item.h"
#include "Items/Rune_Item.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

namespace RunePentagramNames
{
	const FName SlotOne(TEXT("Pentagram.Slot.One"));
	const FName SlotTwo(TEXT("Pentagram.Slot.Two"));
	const FName SlotThree(TEXT("Pentagram.Slot.Three"));

	const FName Accepted(TEXT("Accepted"));
	const FName NoRuneHeld(TEXT("NoRuneHeld"));
	const FName WrongRune(TEXT("WrongRune"));
	const FName PlacementRejected(TEXT("PlacementRejected"));
	const FName InvalidSetup(TEXT("InvalidSetup"));
	const FName AlreadyComplete(TEXT("AlreadyComplete"));
}

ARunePentagram::ARunePentagram()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	PentagramMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PentagramMesh"));
	PentagramMesh->SetupAttachment(SceneRoot);
	PentagramMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PentagramMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	PentagramMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	PentagramMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, ECR_Block);
	PentagramMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, ECR_Block);

	RuneSlotOne = CreateDefaultSubobject<USceneComponent>(TEXT("RuneSlotOne"));
	RuneSlotOne->SetupAttachment(SceneRoot);

	RuneSlotTwo = CreateDefaultSubobject<USceneComponent>(TEXT("RuneSlotTwo"));
	RuneSlotTwo->SetupAttachment(SceneRoot);

	RuneSlotThree = CreateDefaultSubobject<USceneComponent>(TEXT("RuneSlotThree"));
	RuneSlotThree->SetupAttachment(SceneRoot);

	auto ConfigureSlotCollision = [](UBoxComponent* CollisionBox, USceneComponent* ParentSlot)
	{
		CollisionBox->SetupAttachment(ParentSlot);
		CollisionBox->SetBoxExtent(FVector(18.0f, 18.0f, 12.0f));
		CollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
		CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		CollisionBox->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, ECR_Block);
		CollisionBox->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, ECR_Block);
		CollisionBox->SetGenerateOverlapEvents(false);
	};

	RuneSlotOneCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("RuneSlotOneCollision"));
	ConfigureSlotCollision(RuneSlotOneCollision, RuneSlotOne);

	RuneSlotTwoCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("RuneSlotTwoCollision"));
	ConfigureSlotCollision(RuneSlotTwoCollision, RuneSlotTwo);

	RuneSlotThreeCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("RuneSlotThreeCollision"));
	ConfigureSlotCollision(RuneSlotThreeCollision, RuneSlotThree);
}

void ARunePentagram::BeginPlay()
{
	Super::BeginPlay();

	// Blueprint component overrides can preserve an old/disabled collision preset
	// after native components change. Reapply the interaction channels at runtime.
	ConfigureInteractionCollision(PentagramMesh);
	ConfigureInteractionCollision(RuneSlotOneCollision);
	ConfigureInteractionCollision(RuneSlotTwoCollision);
	ConfigureInteractionCollision(RuneSlotThreeCollision);

	const bool bSetupValid = HasValidRequiredRuneSetup();
	UE_LOG(LogTemp, Warning,
		TEXT("[PentagramDebug] BEGIN Pentagram=%s Authority=%d NetMode=%d Setup=%s Required=[%s, %s, %s]"),
		*GetName(), HasAuthority(), static_cast<int32>(GetNetMode()),
		bSetupValid ? TEXT("VALID") : TEXT("INVALID"),
		*RequiredRuneIdOne.ToString(), *RequiredRuneIdTwo.ToString(), *RequiredRuneIdThree.ToString());

	auto LogPentagramCollision = [this](const TCHAR* Label, const UPrimitiveComponent* Component)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[PentagramDebug] COLLISION %s Component=%s Enabled=%d PastResponse=%d FutureResponse=%d VisibilityResponse=%d"),
			Label,
			*GetNameSafe(Component),
			Component ? static_cast<int32>(Component->GetCollisionEnabled()) : -1,
			Component ? static_cast<int32>(Component->GetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST)) : -1,
			Component ? static_cast<int32>(Component->GetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE)) : -1,
			Component ? static_cast<int32>(Component->GetCollisionResponseToChannel(ECC_Visibility)) : -1);
	};

	LogPentagramCollision(TEXT("Mesh"), PentagramMesh);
	LogPentagramCollision(TEXT("SlotOne"), RuneSlotOneCollision);
	LogPentagramCollision(TEXT("SlotTwo"), RuneSlotTwoCollision);
	LogPentagramCollision(TEXT("SlotThree"), RuneSlotThreeCollision);

	if (HasAuthority() && !bSetupValid)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[RunePentagram] %s requires three non-empty and different Required Rune Id values"),
			*GetName());
	}

	PrintPentagramDebugStatus(6.0f);
}

void ARunePentagram::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bShowDebugStatusOnScreen || !GEngine)
	{
		return;
	}

	const FColor StatusColor = bPentagramCompleted
		? FColor::Green
		: (HasValidRequiredRuneSetup() ? FColor::Yellow : FColor::Red);
	const uint64 MessageKey = static_cast<uint64>(GetUniqueID()) + 710000ULL;
	GEngine->AddOnScreenDebugMessage(
		MessageKey,
		0.2f,
		StatusColor,
		GetPentagramDebugStatus(),
		false,
		FVector2D(0.85f, 0.85f));
}

void ARunePentagram::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARunePentagram, PlacedRuneOne);
	DOREPLIFETIME(ARunePentagram, PlacedRuneTwo);
	DOREPLIFETIME(ARunePentagram, PlacedRuneThree);
	DOREPLIFETIME(ARunePentagram, bPentagramCompleted);
	DOREPLIFETIME(ARunePentagram, CompletingPlayer);
	DOREPLIFETIME(ARunePentagram, CompletingPlayerNewTimeline);
}

void ARunePentagram::Interact_Implementation(AActor* Interactor)
{
	AHronoCharacter* Character = Cast<AHronoCharacter>(Interactor);
	ABase_Item* CurrentItem = Character ? Character->GetHeldItem() : nullptr;
	UE_LOG(LogTemp, Warning,
		TEXT("[PentagramDebug] INTERACT Pentagram=%s Authority=%d Interactor=%s Character=%d CurrentItem=%s ItemClass=%s"),
		*GetName(), HasAuthority(), *GetNameSafe(Interactor), Character != nullptr,
		*GetNameSafe(CurrentItem), *GetNameSafe(CurrentItem ? CurrentItem->GetClass() : nullptr));

	if (!HasAuthority())
	{
		// AHronoCharacter::Server_InteractWithEnvironment routes the normal E key
		// here on authority. Direct client calls are intentionally ignored.
		return;
	}

	if (!Character)
	{
		SetLastInteractionDebug(
			FString::Printf(TEXT("Rejected: interactor %s is not HronoCharacter"), *GetNameSafe(Interactor)),
			false);
		return;
	}

	SetLastInteractionDebug(
		FString::Printf(TEXT("E received from %s; selected item: %s"),
			*GetNameSafe(Character), *GetNameSafe(CurrentItem)),
		true);
	TryInsertCurrentRune(Character);
}

bool ARunePentagram::TryInsertCurrentRune(AHronoCharacter* Character)
{
	ARune_Item* Rune = Character
		? Cast<ARune_Item>(Character->GetHeldItem())
		: nullptr;

	if (bPentagramCompleted)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PentagramDebug] REJECT AlreadyComplete Rune=%s"), *GetNameSafe(Rune));
		MulticastRuneInteractionResult(
			false, Rune, NAME_None, RunePentagramNames::AlreadyComplete);
		return false;
	}

	if (!HasValidRequiredRuneSetup())
	{
		UE_LOG(LogTemp, Error,
			TEXT("[PentagramDebug] REJECT InvalidSetup Required=[%s, %s, %s]"),
			*RequiredRuneIdOne.ToString(), *RequiredRuneIdTwo.ToString(), *RequiredRuneIdThree.ToString());
		MulticastRuneInteractionResult(
			false, Rune, NAME_None, RunePentagramNames::InvalidSetup);
		return false;
	}

	if (!Rune)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[RunePentagram] Player=%s interacted without a selected Rune_Item"),
			*GetNameSafe(Character));
		MulticastRuneInteractionResult(
			false, nullptr, NAME_None, RunePentagramNames::NoRuneHeld);
		return false;
	}

	return TryPlaceRuneInMatchingSlot(Rune, Character);
}

bool ARunePentagram::TryPlaceRuneInMatchingSlot(
	ARune_Item* Rune,
	AHronoCharacter* PlacingCharacter)
{
	if (!IsValid(Rune) || !IsValid(PlacingCharacter))
	{
		return false;
	}

	TObjectPtr<ARune_Item>* TargetPlacedRune = nullptr;
	USceneComponent* TargetSlot = nullptr;
	FName TargetSlotId = NAME_None;
	FName TargetRequiredId = NAME_None;

	if (!PlacedRuneOne && Rune->RuneId == RequiredRuneIdOne)
	{
		TargetPlacedRune = &PlacedRuneOne;
		TargetSlot = RuneSlotOne;
		TargetSlotId = RunePentagramNames::SlotOne;
		TargetRequiredId = RequiredRuneIdOne;
	}
	else if (!PlacedRuneTwo && Rune->RuneId == RequiredRuneIdTwo)
	{
		TargetPlacedRune = &PlacedRuneTwo;
		TargetSlot = RuneSlotTwo;
		TargetSlotId = RunePentagramNames::SlotTwo;
		TargetRequiredId = RequiredRuneIdTwo;
	}
	else if (!PlacedRuneThree && Rune->RuneId == RequiredRuneIdThree)
	{
		TargetPlacedRune = &PlacedRuneThree;
		TargetSlot = RuneSlotThree;
		TargetSlotId = RunePentagramNames::SlotThree;
		TargetRequiredId = RequiredRuneIdThree;
	}

	if (!TargetPlacedRune || !TargetSlot)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[RunePentagram] WRONG Rune=%s RuneId=%s for %s"),
			*GetNameSafe(Rune), *Rune->RuneId.ToString(), *GetName());
		MulticastRuneInteractionResult(
			false, Rune, NAME_None, RunePentagramNames::WrongRune);
		return false;
	}

	UE_LOG(LogTemp, Warning,
		TEXT("[PentagramDebug] MATCH Rune=%s RuneId=%s -> Slot=%s Required=%s"),
		*GetNameSafe(Rune), *Rune->RuneId.ToString(),
		*TargetSlotId.ToString(), *TargetRequiredId.ToString());

	Rune->PlaceRuneInPentagram(
		this,
		TargetSlotId,
		TargetRequiredId,
		TargetSlot->GetComponentTransform(),
		3);

	if (!Rune->bPlacedInPentagram || Rune->PlacedPentagram != this)
	{
		MulticastRuneInteractionResult(
			false, Rune, TargetSlotId, RunePentagramNames::PlacementRejected);
		return false;
	}

	*TargetPlacedRune = Rune;
	ForceNetUpdate();

	UE_LOG(LogTemp, Log,
		TEXT("[RunePentagram] INSERTED Rune=%s RuneId=%s Slot=%s Progress=%d/3"),
		*GetNameSafe(Rune), *Rune->RuneId.ToString(), *TargetSlotId.ToString(),
		GetPlacedRuneCount());

	MulticastRuneInteractionResult(
		true, Rune, TargetSlotId, RunePentagramNames::Accepted);
	CheckPentagramCompletion(PlacingCharacter);
	return true;
}

bool ARunePentagram::HasValidRequiredRuneSetup() const
{
	return !RequiredRuneIdOne.IsNone() &&
		!RequiredRuneIdTwo.IsNone() &&
		!RequiredRuneIdThree.IsNone() &&
		RequiredRuneIdOne != RequiredRuneIdTwo &&
		RequiredRuneIdOne != RequiredRuneIdThree &&
		RequiredRuneIdTwo != RequiredRuneIdThree;
}

int32 ARunePentagram::GetPlacedRuneCount() const
{
	return (IsValid(PlacedRuneOne) ? 1 : 0) +
		(IsValid(PlacedRuneTwo) ? 1 : 0) +
		(IsValid(PlacedRuneThree) ? 1 : 0);
}

void ARunePentagram::CheckPentagramCompletion(AHronoCharacter* PlayerWhoPlacedRune)
{
	if (!HasAuthority() || bPentagramCompleted || GetPlacedRuneCount() != 3)
	{
		return;
	}
	if (!IsValid(PlayerWhoPlacedRune))
	{
		UE_LOG(LogTemp, Error,
			TEXT("[RunePentagram] Cannot complete %s: third-rune player is invalid"),
			*GetName());
		return;
	}

	CompletingPlayer = PlayerWhoPlacedRune;
	const EItemTimeline PreviousTimeline = CompletingPlayer->GetTimeline();
	const EItemTimeline RequestedTimeline = PreviousTimeline == EItemTimeline::Past
		? EItemTimeline::Future
		: EItemTimeline::Past;

	// Use the character's server-authoritative API so mirroring, carried items,
	// collision and same-timeline player visibility are updated together.
	CompletingPlayer->SetPlayerTimeline(RequestedTimeline);
	CompletingPlayerNewTimeline = CompletingPlayer->GetTimeline();

	bPentagramCompleted = true;
	ForceNetUpdate();
	DeliverThirdRuneEvents();

	UE_LOG(LogTemp, Warning,
		TEXT("[RunePentagram] COMPLETE Pentagram=%s ThirdRunePlayer=%s Timeline=%s->%s"),
		*GetName(), *GetNameSafe(CompletingPlayer),
		*StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(PreviousTimeline)),
		*StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(CompletingPlayerNewTimeline)));
}

void ARunePentagram::MulticastRuneInteractionResult_Implementation(
	bool bAccepted,
	ARune_Item* Rune,
	FName SlotId,
	FName ResultReason)
{
	const FString ResultText = FString::Printf(
		TEXT("%s: %s | Rune=%s RuneId=%s Slot=%s"),
		bAccepted ? TEXT("ACCEPTED") : TEXT("REJECTED"),
		*ResultReason.ToString(),
		*GetNameSafe(Rune),
		Rune ? *Rune->RuneId.ToString() : TEXT("None"),
		*SlotId.ToString());
	SetLastInteractionDebug(ResultText, bAccepted);
	UE_LOG(LogTemp, Warning, TEXT("[PentagramDebug] RESULT %s"), *ResultText);
	OnRuneInteractionResult.Broadcast(bAccepted, Rune, SlotId, ResultReason);
}

void ARunePentagram::OnRep_RuneSlots()
{
	RefreshReplicatedRuneAttachments();
}

void ARunePentagram::RefreshReplicatedRuneAttachments()
{
	auto AttachRuneToSlot = [this](ARune_Item* Rune, USceneComponent* Slot)
	{
		if (!IsValid(Rune) || !IsValid(Slot))
		{
			return;
		}

		Rune->SetActorTransform(Slot->GetComponentTransform(), false, nullptr, ETeleportType::TeleportPhysics);
		Rune->AttachToComponent(Slot, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	};

	AttachRuneToSlot(PlacedRuneOne, RuneSlotOne);
	AttachRuneToSlot(PlacedRuneTwo, RuneSlotTwo);
	AttachRuneToSlot(PlacedRuneThree, RuneSlotThree);
}

void ARunePentagram::OnRep_PentagramCompleted()
{
	if (bPentagramCompleted)
	{
		DeliverThirdRuneEvents();
	}
}

void ARunePentagram::OnRep_CompletingPlayer()
{
	if (bPentagramCompleted)
	{
		DeliverThirdRuneEvents();
	}
}

void ARunePentagram::DeliverThirdRuneEvents()
{
	if (bThirdRuneEventsDelivered || !bPentagramCompleted || !IsValid(CompletingPlayer) ||
		CompletingPlayerNewTimeline == EItemTimeline::Both)
	{
		return;
	}

	bThirdRuneEventsDelivered = true;
	BP_OnAllThreeRunesInserted(CompletingPlayer);
	BroadcastPentagramCompleted();
}

void ARunePentagram::BroadcastPentagramCompleted()
{
	SetLastInteractionDebug(TEXT("RITUAL COMPLETE: all 3 correct runes inserted"), true);
	OnPentagramCompleted.Broadcast();
	BP_OnPentagramCompleted();
}

FString ARunePentagram::GetPentagramDebugStatus() const
{
	auto SlotState = [](const ARune_Item* Rune, FName RequiredId) -> FString
	{
		return IsValid(Rune)
			? FString::Printf(TEXT("PLACED %s (%s)"), *GetNameSafe(Rune), *Rune->RuneId.ToString())
			: FString::Printf(TEXT("EMPTY, needs %s"), *RequiredId.ToString());
	};

	const TCHAR* NetworkState = HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT");
	const bool bCollisionReady =
		RuneSlotOneCollision && RuneSlotOneCollision->GetCollisionEnabled() != ECollisionEnabled::NoCollision &&
		RuneSlotTwoCollision && RuneSlotTwoCollision->GetCollisionEnabled() != ECollisionEnabled::NoCollision &&
		RuneSlotThreeCollision && RuneSlotThreeCollision->GetCollisionEnabled() != ECollisionEnabled::NoCollision;

	return FString::Printf(
		TEXT("PENTAGRAM [%s] %s | Setup:%s Collision:%s\n1: %s\n2: %s\n3: %s\nCompleting Player: %s -> %s\nLast: %s"),
		NetworkState,
		bPentagramCompleted ? TEXT("COMPLETE") : TEXT("WAITING"),
		HasValidRequiredRuneSetup() ? TEXT("OK") : TEXT("INVALID"),
		bCollisionReady ? TEXT("ON") : TEXT("OFF"),
		*SlotState(PlacedRuneOne, RequiredRuneIdOne),
		*SlotState(PlacedRuneTwo, RequiredRuneIdTwo),
		*SlotState(PlacedRuneThree, RequiredRuneIdThree),
		*GetNameSafe(CompletingPlayer),
		*StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(CompletingPlayerNewTimeline)),
		*LastInteractionDebug);
}

void ARunePentagram::PrintPentagramDebugStatus(float Duration)
{
	if (!GEngine)
	{
		return;
	}

	const FColor StatusColor = bPentagramCompleted
		? FColor::Green
		: (HasValidRequiredRuneSetup() ? FColor::Yellow : FColor::Red);
	GEngine->AddOnScreenDebugMessage(
		-1,
		FMath::Max(0.1f, Duration),
		StatusColor,
		GetPentagramDebugStatus());
}

void ARunePentagram::ConfigureInteractionCollision(UPrimitiveComponent* Component) const
{
	if (!IsValid(Component))
	{
		return;
	}

	Component->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Component->SetCollisionResponseToAllChannels(ECR_Ignore);
	Component->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	Component->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, ECR_Block);
	Component->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, ECR_Block);
}

void ARunePentagram::SetLastInteractionDebug(const FString& NewStatus, bool bSuccess)
{
	LastInteractionDebug = NewStatus;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			4.0f,
			bSuccess ? FColor::Green : FColor::Red,
			FString::Printf(TEXT("PENTAGRAM: %s"), *NewStatus));
	}
}
