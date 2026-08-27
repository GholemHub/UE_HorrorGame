#include "Enviroment/Room.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Items/Base_Item.h"
#include "Items/Clock.h"
#include "Items/Drag_Item.h"
#include "Items/HotDot.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(LogRoom, Log, All);

namespace RoomClockPatterns
{
	constexpr EClockAnomalyType AnomalyTypes[] =
	{
		EClockAnomalyType::Reverse,
		EClockAnomalyType::Frozen,
		EClockAnomalyType::JumpForward,
		EClockAnomalyType::JumpBackward,
		EClockAnomalyType::ErraticJumps,
		EClockAnomalyType::Stutter,
		EClockAnomalyType::Fast,
		EClockAnomalyType::Slow
	};

	constexpr int32 AnomalyTypeCount = UE_ARRAY_COUNT(AnomalyTypes);
	constexpr int32 OrdinarySeedVariants = 16;
	constexpr int32 CursedPairVariants = 8;

	int32 PositiveModulo(int32 Value, int32 Divisor)
	{
		const int32 Result = Value % Divisor;
		return Result < 0 ? Result + Divisor : Result;
	}

	int32 MakeClockSeed(int32 PatternSeed, const AClock* Clock, int32 Salt)
	{
		uint32 Hash = HashCombine(GetTypeHash(PatternSeed), GetTypeHash(Salt));
		Hash = HashCombine(Hash, GetTypeHash(GetNameSafe(Clock)));
		return Hash == 0 ? 1 : static_cast<int32>(Hash);
	}
}

ARoom::ARoom()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(false);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	RoomVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("RoomVolume"));
	RoomVolume->SetupAttachment(SceneRoot);
	RoomVolume->SetBoxExtent(FVector(300.0f, 300.0f, 150.0f));
	RoomVolume->SetCollisionProfileName(TEXT("Trigger"));
	RoomVolume->SetGenerateOverlapEvents(true);
}

void ARoom::BeginPlay()
{
	Super::BeginPlay();

	RoomVolume->OnComponentBeginOverlap.AddUniqueDynamic(this, &ARoom::HandleRoomBeginOverlap);
	RoomVolume->OnComponentEndOverlap.AddUniqueDynamic(this, &ARoom::HandleRoomEndOverlap);
	BindDoorEvents();

	if (!HasAuthority())
	{
		if (PaintingEvidenceState.PatternIndex != INDEX_NONE)
		{
			DispatchCursedPaintingEvent();
		}
		return;
	}

	InitializeClocks();
	if (ClockAnomalyPatternIndex != INDEX_NONE)
	{
		ApplyClockAnomalyPattern();
	}
	if (HotDotPatternIndex == INDEX_NONE)
	{
		HotDotPatternIndex = FMath::RandHelper(MAX_int32 - 1);
		HotDotPatternSeed = FMath::RandHelper(MAX_int32 - 1) + 1;
	}
	ApplyHotDotPattern();
	if (PaintingEvidenceState.PatternIndex != INDEX_NONE)
	{
		ApplyPaintingEvidencePattern();
	}

	// Blueprint classes are deliberately loaded at BeginPlay instead of while the
	// native CDO is being constructed, avoiding Blueprint dependency re-entry.
	if (!CursedItemClass)
	{
		CursedItemClass = LoadClass<ABase_Item>(
			nullptr,
			TEXT("/Game/_Alex/Pickable/BP_Cursed_Item.BP_Cursed_Item_C"));
	}

	if (!KeyClass)
	{
		KeyClass = LoadClass<ABase_Item>(
			nullptr,
			TEXT("/Game/_Alex/Pickable/BP_Key_Item.BP_Key_Item_C"));
	}

	if (!CursedItemTag.IsValid())
	{
		CursedItemTag = FGameplayTag::RequestGameplayTag(TEXT("Item.Cursed"), false);
	}
}

void ARoom::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindDoorEvents();
	Super::EndPlay(EndPlayReason);
}

void ARoom::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARoom, bIsCursed);
	DOREPLIFETIME(ARoom, bPuzzleCompleted);
	DOREPLIFETIME(ARoom, SpawnedKey);
	DOREPLIFETIME(ARoom, ClockAnomalyPatternIndex);
	DOREPLIFETIME(ARoom, ClockAnomalyPatternSeed);
	DOREPLIFETIME(ARoom, HotDotPatternIndex);
	DOREPLIFETIME(ARoom, HotDotPatternSeed);
	DOREPLIFETIME(ARoom, PaintingEvidenceState);
}

void ARoom::SetCursed(bool bNewCursed)
{
	if (!HasAuthority())
	{
		return;
	}

	if (bIsCursed == bNewCursed)
	{
		RefreshClockCursedStates();
		if (ClockAnomalyPatternIndex != INDEX_NONE)
		{
			ApplyClockAnomalyPattern();
		}
		ApplyHotDotPattern();
		ApplyPaintingEvidencePattern();
		return;
	}

	bIsCursed = bNewCursed;
	if (bIsCursed)
	{
		ResetRoomPuzzle();
	}
	else
	{
		CurrentCursedItem = nullptr;
	}

	RefreshClockCursedStates();
	if (ClockAnomalyPatternIndex != INDEX_NONE)
	{
		ApplyClockAnomalyPattern();
	}
	ApplyHotDotPattern();
	ApplyPaintingEvidencePattern();
	DispatchCursedStateChanged();
	ForceNetUpdate();
}

void ARoom::ConfigureClockAnomalies(int32 PatternIndex, int32 PatternSeed)
{
	if (!HasAuthority())
	{
		return;
	}

	ClockAnomalyPatternIndex = FMath::Max(0, PatternIndex);
	ClockAnomalyPatternSeed = PatternSeed != 0 ? PatternSeed : 1;
	ApplyClockAnomalyPattern();
	ForceNetUpdate();
}

FString ARoom::GetClockAnomalySummary() const
{
	TArray<FString> Entries;
	Entries.Reserve(Clocks.Num());
	for (const AActor* ClockActor : Clocks)
	{
		const AClock* Clock = Cast<AClock>(ClockActor);
		if (!IsValid(Clock))
		{
			continue;
		}

		Entries.Add(FString::Printf(
			TEXT("%s [%s]: %s"),
			*Clock->GetName(),
			*StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(Clock->ItemTimeline)),
			*Clock->GetClockAnomalyDescription()));
	}

	return FString::Printf(
		TEXT("Room=%s Cursed=%s Pattern=%d Seed=%d | %s"),
		*GetName(),
		bIsCursed ? TEXT("true") : TEXT("false"),
		ClockAnomalyPatternIndex,
		ClockAnomalyPatternSeed,
		*FString::Join(Entries, TEXT("; ")));
}

int32 ARoom::GetOrdinaryClockPatternCount()
{
	return RoomClockPatterns::AnomalyTypeCount * 2 * RoomClockPatterns::OrdinarySeedVariants;
}

int32 ARoom::GetCursedClockPatternCount()
{
	return RoomClockPatterns::AnomalyTypeCount
		* (RoomClockPatterns::AnomalyTypeCount - 1)
		* RoomClockPatterns::CursedPairVariants;
}

void ARoom::ConfigureHotDots(int32 PatternIndex, int32 PatternSeed)
{
	if (!HasAuthority())
	{
		return;
	}

	HotDotPatternIndex = FMath::Max(0, PatternIndex);
	HotDotPatternSeed = PatternSeed != 0 ? PatternSeed : 1;
	ApplyHotDotPattern();
	ForceNetUpdate();
}

FString ARoom::GetHotDotSummary() const
{
	TArray<FString> Entries;
	for (const AActor* HotDotActor : HotDots)
	{
		if (const AHotDot* HotDot = Cast<AHotDot>(HotDotActor))
		{
			Entries.Add(FString::Printf(
				TEXT("%s=%s"),
				*HotDot->GetName(),
				HotDot->IsHotDotActive() ? TEXT("Active") : TEXT("Inactive")));
		}
	}

	return FString::Printf(
		TEXT("Room=%s Cursed=%s HotDots=[%s]"),
		*GetName(),
		bIsCursed ? TEXT("true") : TEXT("false"),
		*FString::Join(Entries, TEXT(", ")));
}

void ARoom::ConfigurePaintingEvidence(int32 PatternIndex, int32 PatternSeed)
{
	if (!HasAuthority())
	{
		return;
	}

	PaintingEvidenceState.PatternIndex = FMath::Max(0, PatternIndex);
	PaintingEvidenceState.PatternSeed = PatternSeed != 0 ? PatternSeed : 1;
	ApplyPaintingEvidencePattern();
	ForceNetUpdate();
}

TArray<AActor*> ARoom::GetSelectedCursedPaintings() const
{
	TArray<AActor*> Result;
	Result.Reserve(PaintingEvidenceState.SelectedPaintings.Num());
	for (AActor* Painting : PaintingEvidenceState.SelectedPaintings)
	{
		if (IsValid(Painting))
		{
			Result.Add(Painting);
		}
	}
	return Result;
}

bool ARoom::IsCursedItemInRoom() const
{
	return IsValid(CurrentCursedItem.Get());
}

bool ARoom::AreAllDoorsClosed() const
{
	int32 ValidDoorCount = 0;
	for (const ADrag_Item* Door : RoomDoors)
	{
		if (!IsValid(Door))
		{
			continue;
		}

		++ValidDoorCount;
		if (!Door->bIsClosed)
		{
			return false;
		}
	}

	return ValidDoorCount > 0 || !bRequireAtLeastOneDoor;
}

bool ARoom::TryCompleteRoomPuzzle()
{
	if (!HasAuthority()
		|| !bIsCursed
		|| bPuzzleCompleted
		|| !IsValid(CurrentCursedItem.Get())
		|| !AreAllDoorsClosed()
		|| !KeyClass)
	{
		return false;
	}

	ABase_Item* CursedItem = CurrentCursedItem;
	FTransform KeyTransform = CursedItem->GetActorTransform();
	KeyTransform.AddToTranslation(KeyTransform.TransformVectorNoScale(KeySpawnOffset));

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.Instigator = CursedItem->GetInstigator();
	SpawnParameters.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ABase_Item* NewKey = GetWorld()->SpawnActor<ABase_Item>(
		KeyClass,
		KeyTransform,
		SpawnParameters);
	if (!IsValid(NewKey))
	{
		UE_LOG(LogRoom, Error,
			TEXT("[%s] Failed to spawn key class %s from cursed item %s"),
			*GetName(), *GetNameSafe(KeyClass.Get()), *GetNameSafe(CursedItem));
		return false;
	}

	bPuzzleCompleted = true;
	SpawnedKey = NewKey;
	CurrentCursedItem = nullptr;

	OnRoomPuzzleCompleted.Broadcast(this, CursedItem, NewKey);
	ReceiveRoomPuzzleCompleted(CursedItem, NewKey);

	UE_LOG(LogRoom, Log,
		TEXT("[%s] Cursed-room puzzle completed. Spawned %s from %s."),
		*GetName(), *GetNameSafe(NewKey), *GetNameSafe(CursedItem));

	if (bConsumeCursedItem && IsValid(CursedItem))
	{
		CursedItem->Destroy();
	}

	ForceNetUpdate();
	return true;
}

void ARoom::ResetRoomPuzzle()
{
	if (!HasAuthority())
	{
		return;
	}

	bPuzzleCompleted = false;
	SpawnedKey = nullptr;
	CurrentCursedItem = nullptr;
	ForceNetUpdate();
}

void ARoom::HandleRoomBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!HasAuthority() || bPuzzleCompleted)
	{
		return;
	}

	ABase_Item* Item = Cast<ABase_Item>(OtherActor);
	if (!IsCursedItem(Item))
	{
		return;
	}

	if (!bIsCursed)
	{
		OnWrongRoomChosen.Broadcast(this, Item);
		ReceiveWrongRoomChosen(Item);
		UE_LOG(LogRoom, Log,
			TEXT("[%s] Cursed item %s was placed in a non-cursed room."),
			*GetName(), *GetNameSafe(Item));
		return;
	}

	CurrentCursedItem = Item;
	TryCompleteRoomPuzzle();
}

void ARoom::HandleRoomEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex)
{
	if (HasAuthority() && OtherActor == CurrentCursedItem.Get())
	{
		CurrentCursedItem = nullptr;
	}
}

void ARoom::HandleDoorStateChanged(bool bDoorIsClosed)
{
	if (HasAuthority() && bDoorIsClosed)
	{
		TryCompleteRoomPuzzle();
	}
}

void ARoom::OnRep_IsCursed()
{
	DispatchCursedStateChanged();
}

void ARoom::OnRep_PuzzleCompleted()
{
	if (bPuzzleCompleted)
	{
		ReceiveRoomPuzzleCompleted(nullptr, SpawnedKey);
	}
}

void ARoom::OnRep_PaintingEvidenceState()
{
	if (HasActorBegunPlay())
	{
		DispatchCursedPaintingEvent();
	}
}

bool ARoom::IsCursedItem(const ABase_Item* Item) const
{
	if (!IsValid(Item))
	{
		return false;
	}

	const bool bMatchesClass = CursedItemClass && Item->IsA(CursedItemClass);
	const bool bMatchesTag = CursedItemTag.IsValid() && Item->ItemTags.HasTag(CursedItemTag);
	return bMatchesClass || bMatchesTag;
}

void ARoom::BindDoorEvents()
{
	for (ADrag_Item* Door : RoomDoors)
	{
		if (IsValid(Door))
		{
			Door->OnDoorStateChanged.AddUniqueDynamic(this, &ARoom::HandleDoorStateChanged);
		}
	}
}

void ARoom::UnbindDoorEvents()
{
	for (ADrag_Item* Door : RoomDoors)
	{
		if (IsValid(Door))
		{
			Door->OnDoorStateChanged.RemoveDynamic(this, &ARoom::HandleDoorStateChanged);
		}
	}
}

void ARoom::InitializeClocks()
{
	for (AActor* ClockActor : Clocks)
	{
		AClock* Clock = Cast<AClock>(ClockActor);
		if (IsValid(Clock))
		{
			Clock->AssignToRoom(this);
		}
		else if (IsValid(ClockActor))
		{
			UE_LOG(LogRoom, Warning,
				TEXT("[%s] Contents.Clocks contains %s, but it is not derived from AClock and will be ignored."),
				*GetName(), *GetNameSafe(ClockActor));
		}
	}
}

void ARoom::RefreshClockCursedStates()
{
	for (AActor* ClockActor : Clocks)
	{
		AClock* Clock = Cast<AClock>(ClockActor);
		if (IsValid(Clock))
		{
			Clock->AssignToRoom(this);
		}
	}
}

void ARoom::ApplyClockAnomalyPattern()
{
	if (!HasAuthority() || ClockAnomalyPatternIndex == INDEX_NONE)
	{
		return;
	}

	TArray<AClock*> ValidClocks;
	ValidClocks.Reserve(Clocks.Num());
	for (AActor* ClockActor : Clocks)
	{
		AClock* Clock = Cast<AClock>(ClockActor);
		if (IsValid(Clock))
		{
			Clock->AssignToRoom(this);
			Clock->ClearClockAnomaly();
			ValidClocks.Add(Clock);
		}
	}

	if (ValidClocks.IsEmpty())
	{
		UE_LOG(LogRoom, Verbose, TEXT("[%s] Clock pattern skipped: no clocks are assigned."), *GetName());
		return;
	}

	using namespace RoomClockPatterns;
	if (bIsCursed)
	{
		const int32 FutureTypeIndex = PositiveModulo(ClockAnomalyPatternIndex, AnomalyTypeCount);
		const int32 DifferentTypeOffset = 1
			+ PositiveModulo(ClockAnomalyPatternIndex / AnomalyTypeCount, AnomalyTypeCount - 1);
		const int32 PastTypeIndex = (FutureTypeIndex + DifferentTypeOffset) % AnomalyTypeCount;

		bool bHasFutureClock = false;
		bool bHasPastClock = false;
		for (int32 ClockIndex = 0; ClockIndex < ValidClocks.Num(); ++ClockIndex)
		{
			AClock* Clock = ValidClocks[ClockIndex];
			const bool bUsePastPattern = Clock->ItemTimeline == EItemTimeline::Past
				|| (Clock->ItemTimeline == EItemTimeline::Both && (ClockIndex % 2) != 0);
			const EClockAnomalyType Type = AnomalyTypes[bUsePastPattern ? PastTypeIndex : FutureTypeIndex];
			const int32 Seed = MakeClockSeed(ClockAnomalyPatternSeed, Clock, ClockIndex + 101);
			Clock->ConfigureClockAnomaly(Type, Seed);

			bHasPastClock |= Clock->ItemTimeline == EItemTimeline::Past;
			bHasFutureClock |= Clock->ItemTimeline == EItemTimeline::Future;
		}

		if (!bHasPastClock || !bHasFutureClock)
		{
			UE_LOG(LogRoom, Warning,
				TEXT("[%s] Cursed clock pattern is active, but the Clocks array needs at least one Past and one Future clock (Past=%s Future=%s)."),
				*GetName(), bHasPastClock ? TEXT("yes") : TEXT("no"), bHasFutureClock ? TEXT("yes") : TEXT("no"));
		}
	}
	else
	{
		const int32 AnomalyTypeIndex = PositiveModulo(ClockAnomalyPatternIndex, AnomalyTypeCount);
		const bool bPreferPast = PositiveModulo(ClockAnomalyPatternIndex / AnomalyTypeCount, 2) != 0;
		const EItemTimeline PreferredTimeline = bPreferPast ? EItemTimeline::Past : EItemTimeline::Future;
		const int32 SeedVariant = ClockAnomalyPatternIndex / (AnomalyTypeCount * 2);

		TArray<int32> CandidateIndices;
		for (int32 ClockIndex = 0; ClockIndex < ValidClocks.Num(); ++ClockIndex)
		{
			if (ValidClocks[ClockIndex]->ItemTimeline == PreferredTimeline)
			{
				CandidateIndices.Add(ClockIndex);
			}
		}

		if (CandidateIndices.IsEmpty())
		{
			for (int32 ClockIndex = 0; ClockIndex < ValidClocks.Num(); ++ClockIndex)
			{
				if (ValidClocks[ClockIndex]->ItemTimeline != EItemTimeline::Both)
				{
					CandidateIndices.Add(ClockIndex);
				}
			}
		}
		if (CandidateIndices.IsEmpty())
		{
			CandidateIndices.Add(0);
		}

		const uint32 CandidateHash = HashCombine(
			GetTypeHash(ClockAnomalyPatternSeed),
			GetTypeHash(SeedVariant));
		const int32 CandidateChoice = static_cast<int32>(CandidateHash % CandidateIndices.Num());
		const int32 SelectedClockIndex = CandidateIndices[CandidateChoice];
		AClock* SelectedClock = ValidClocks[SelectedClockIndex];
		SelectedClock->ConfigureClockAnomaly(
			AnomalyTypes[AnomalyTypeIndex],
			MakeClockSeed(ClockAnomalyPatternSeed, SelectedClock, SeedVariant + 211));
	}

	UE_LOG(LogRoom, Log, TEXT("%s"), *GetClockAnomalySummary());
}

void ARoom::ApplyHotDotPattern()
{
	if (!HasAuthority() || HotDotPatternIndex == INDEX_NONE)
	{
		return;
	}

	TArray<AHotDot*> ValidHotDots;
	ValidHotDots.Reserve(HotDots.Num());
	for (AActor* HotDotActor : HotDots)
	{
		if (AHotDot* HotDot = Cast<AHotDot>(HotDotActor))
		{
			ValidHotDots.Add(HotDot);
		}
		else if (IsValid(HotDotActor))
		{
			UE_LOG(LogRoom, Warning,
				TEXT("[%s] Contents.HotDots contains %s, but it is not derived from AHotDot and will be ignored."),
				*GetName(), *GetNameSafe(HotDotActor));
		}
	}

	if (ValidHotDots.IsEmpty())
	{
		return;
	}

	if (bIsCursed)
	{
		for (AHotDot* HotDot : ValidHotDots)
		{
			HotDot->SetHotDotActive(true);
		}
	}
	else
	{
		const int32 ActiveIndex = RoomClockPatterns::PositiveModulo(
			HotDotPatternIndex,
			ValidHotDots.Num());
		for (int32 Index = 0; Index < ValidHotDots.Num(); ++Index)
		{
			ValidHotDots[Index]->SetHotDotActive(Index == ActiveIndex);
		}
	}

	if (ValidHotDots.Num() != 2)
	{
		UE_LOG(LogRoom, Warning,
			TEXT("[%s] HotDots expects exactly 2 valid points, but %d were assigned."),
			*GetName(), ValidHotDots.Num());
	}

	UE_LOG(LogRoom, Log, TEXT("%s"), *GetHotDotSummary());
}

void ARoom::ApplyPaintingEvidencePattern()
{
	if (!HasAuthority() || PaintingEvidenceState.PatternIndex == INDEX_NONE)
	{
		return;
	}

	TArray<ABase_Item*> PastPaintings;
	TArray<ABase_Item*> FuturePaintings;
	for (AActor* PaintingActor : Paintings)
	{
		ABase_Item* Painting = Cast<ABase_Item>(PaintingActor);
		if (!IsValid(Painting))
		{
			if (IsValid(PaintingActor))
			{
				UE_LOG(LogRoom, Warning,
					TEXT("[%s] Contents.Paintings contains %s, but it is not derived from ABase_Item and will be ignored."),
					*GetName(), *GetNameSafe(PaintingActor));
			}
			continue;
		}

		if (Painting->ItemTimeline == EItemTimeline::Past)
		{
			PastPaintings.Add(Painting);
		}
		else if (Painting->ItemTimeline == EItemTimeline::Future)
		{
			FuturePaintings.Add(Painting);
		}
	}

	PaintingEvidenceState.SelectedPaintings.Reset();
	const auto SelectPainting = [this](const TArray<ABase_Item*>& Candidates, int32 Salt) -> ABase_Item*
	{
		if (Candidates.IsEmpty())
		{
			return nullptr;
		}
		const uint32 SelectionHash = HashCombine(
			GetTypeHash(PaintingEvidenceState.PatternSeed),
			GetTypeHash(Salt));
		return Candidates[static_cast<int32>(SelectionHash % Candidates.Num())];
	};

	if (bIsCursed)
	{
		if (ABase_Item* PastPainting = SelectPainting(PastPaintings, 301))
		{
			PaintingEvidenceState.SelectedPaintings.Add(PastPainting);
		}
		if (ABase_Item* FuturePainting = SelectPainting(FuturePaintings, 401))
		{
			PaintingEvidenceState.SelectedPaintings.Add(FuturePainting);
		}

		if (PaintingEvidenceState.SelectedPaintings.Num() != 2)
		{
			UE_LOG(LogRoom, Warning,
				TEXT("[%s] Cursed painting evidence requires at least one Past and one Future painting (Past=%d Future=%d)."),
				*GetName(), PastPaintings.Num(), FuturePaintings.Num());
		}
	}
	else
	{
		const int32 OrdinaryResult = RoomClockPatterns::PositiveModulo(
			PaintingEvidenceState.PatternIndex,
			3);
		ABase_Item* SelectedPainting = nullptr;
		if (OrdinaryResult == 1)
		{
			SelectedPainting = SelectPainting(PastPaintings, 501);
		}
		else if (OrdinaryResult == 2)
		{
			SelectedPainting = SelectPainting(FuturePaintings, 601);
		}

		if (SelectedPainting)
		{
			PaintingEvidenceState.SelectedPaintings.Add(SelectedPainting);
		}
	}

	if (HasActorBegunPlay())
	{
		DispatchCursedPaintingEvent();
	}
	ForceNetUpdate();
	UE_LOG(LogRoom, Log,
		TEXT("[%s] Painting evidence selected: Cursed=%s Count=%d Pattern=%d Seed=%d."),
		*GetName(),
		bIsCursed ? TEXT("true") : TEXT("false"),
		PaintingEvidenceState.SelectedPaintings.Num(),
		PaintingEvidenceState.PatternIndex,
		PaintingEvidenceState.PatternSeed);
}

void ARoom::DispatchCursedPaintingEvent()
{
	OnCursedPainting(GetSelectedCursedPaintings());
}

void ARoom::DispatchCursedStateChanged()
{
	OnCursedStateChanged.Broadcast(this, bIsCursed);
	ReceiveCursedStateChanged(bIsCursed);
}
