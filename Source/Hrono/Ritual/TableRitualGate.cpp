#include "Ritual/TableRitualGate.h"

#include "EngineUtils.h"
#include "Engine/World.h"
#include "HronoCharacter.h"
#include "Items/Base_Item.h"
#include "Items/Chair.h"
#include "Ritual/TableRitualManager.h"
#include "UObject/Package.h"

DEFINE_LOG_CATEGORY_STATIC(LogTableRitualGate, Log, All);

namespace
{
	const FName CursedImageClassPackage(TEXT("/Game/_Alex/Paints/BP_CursedImage_Item"));
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

	bool IsConfiguredTableRitualChair(const AChair& Chair)
	{
		const UWorld* World = Chair.GetWorld();
		if (!IsValid(World))
		{
			return false;
		}

		for (TActorIterator<ATableRitualManager> It(World); It; ++It)
		{
			const ATableRitualManager* Candidate = *It;
			if (IsValid(Candidate)
				&& (Candidate->TableChairA == &Chair || Candidate->TableChairB == &Chair))
			{
				return true;
			}
		}

		return false;
	}
}

bool TableRitualGate::IsCursedImage(const ABase_Item& Item)
{
	return IsCursedImageClass(Item.GetClass());
}

void TableRitualGate::NotifySuccessfulPickup(
	const ABase_Item& Item,
	const AHronoCharacter& Character)
{
	UWorld* World = Item.GetWorld();
	if (!IsValid(World)
		|| World->GetNetMode() == NM_Client
		|| !IsCursedImage(Item)
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
	for (TActorIterator<AChair> It(World); It; ++It)
	{
		AChair* Chair = *It;
		if (IsValid(Chair) && IsConfiguredTableRitualChair(*Chair))
		{
			Chair->SetRitualGuidanceUnlocked(true);
		}
	}

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
	return !IsConfiguredTableRitualChair(Chair) || IsUnlocked(&Chair);
}

bool TableRitualGate::IsTableRitualChair(const AChair& Chair)
{
	return IsConfiguredTableRitualChair(Chair);
}

bool TableRitualGate::IsRitualInProgress(const UObject* WorldContextObject)
{
	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	if (!IsValid(World))
	{
		return false;
	}

	for (TActorIterator<AChair> It(World); It; ++It)
	{
		const AChair* Chair = *It;
		if (IsValid(Chair)
			&& Chair->IsRitualStarted
			&& IsConfiguredTableRitualChair(*Chair))
		{
			return true;
		}
	}

	return false;
}

bool TableRitualGate::AreAllPlayersSeatedAtRitualTable(const UObject* WorldContextObject)
{
	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	if (!IsValid(World) || World->GetNetMode() == NM_Client)
	{
		return false;
	}

	int32 GameplayPlayerCount = 0;
	for (TActorIterator<AHronoCharacter> It(World); It; ++It)
	{
		const AHronoCharacter* Character = *It;
		if (!IsValid(Character) || !IsValid(Character->GetController()))
		{
			continue;
		}

		++GameplayPlayerCount;
		const AChair* Chair = Character->GetCurrentChair();
		if (!Character->IsSittingOnChair()
			|| !IsValid(Chair)
			|| !IsConfiguredTableRitualChair(*Chair))
		{
			return false;
		}
	}

	return GameplayPlayerCount > 0;
}
