#include "Ritual/TableRitualGate.h"

#include "EngineUtils.h"
#include "Engine/World.h"
#include "HronoCharacter.h"
#include "Items/Base_Item.h"
#include "Items/Chair.h"

DEFINE_LOG_CATEGORY_STATIC(LogTableRitualGate, Log, All);

namespace
{
	const FName CursedImageClassPackage(TEXT("/Game/_Alex/Paints/BP_CursedImage_Item"));
	const FName TableRitualManagerClassName(TEXT("BP_TableRitualManager_C"));
	constexpr float RitualChairDetectionRadius = 500.0f;

	TSet<TWeakObjectPtr<const UWorld>> UnlockedWorlds;

	void RemoveExpiredWorlds()
	{
		for (auto It = UnlockedWorlds.CreateIterator(); It; ++It)
		{
			if (!It->IsValid())
			{
				It.RemoveCurrent();
			}
		}
	}

	bool IsCursedImageClass(const UClass* ItemClass)
	{
		return IsValid(ItemClass)
			&& ItemClass->GetOutermost()
			&& ItemClass->GetOutermost()->GetFName() == CursedImageClassPackage;
	}

	bool IsChairBesideTableRitualManager(const AChair& Chair)
	{
		const UWorld* World = Chair.GetWorld();
		if (!IsValid(World))
		{
			return false;
		}

		const float RadiusSquared = FMath::Square(RitualChairDetectionRadius);
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			const AActor* Candidate = *It;
			if (IsValid(Candidate)
				&& Candidate->GetClass()->GetFName() == TableRitualManagerClassName
				&& FVector::DistSquared(Chair.GetActorLocation(), Candidate->GetActorLocation())
					<= RadiusSquared)
			{
				return true;
			}
		}

		return false;
	}
}

void TableRitualGate::NotifySuccessfulPickup(
	const ABase_Item& Item,
	const AHronoCharacter& Character)
{
	UWorld* World = Item.GetWorld();
	if (!IsValid(World)
		|| World->GetNetMode() == NM_Client
		|| !IsCursedImageClass(Item.GetClass())
		|| Item.OwningCharacter != &Character
		|| !Item.bIsPickedUp)
	{
		return;
	}

	RemoveExpiredWorlds();
	if (UnlockedWorlds.Contains(World))
	{
		return;
	}

	UnlockedWorlds.Add(World);
	UE_LOG(LogTableRitualGate, Log,
		TEXT("[TableRitualGate] Unlocked by %s picking up %s"),
		*Character.GetName(),
		*Item.GetName());
}

bool TableRitualGate::IsUnlocked(const UObject* WorldContextObject)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	if (!IsValid(World))
	{
		return false;
	}

	RemoveExpiredWorlds();
	return UnlockedWorlds.Contains(World);
}

bool TableRitualGate::CanUseChair(const AChair& Chair)
{
	return !IsChairBesideTableRitualManager(Chair) || IsUnlocked(&Chair);
}
