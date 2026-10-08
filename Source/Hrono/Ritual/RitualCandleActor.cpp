#include "Ritual/RitualCandleActor.h"

#include "Components/RitualCandleComponent.h"

ARitualCandleActor::ARitualCandleActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	RitualCandle = CreateDefaultSubobject<URitualCandleComponent>(TEXT("RitualCandle"));
}

void ARitualCandleActor::StartRitualLighting()
{
	if (HasAuthority() && IsValid(RitualCandle))
	{
		RitualCandle->StartLighting();
	}
}

void ARitualCandleActor::ReportRitualMistake()
{
	if (HasAuthority() && IsValid(RitualCandle))
	{
		RitualCandle->ReportMistake();
	}
}
