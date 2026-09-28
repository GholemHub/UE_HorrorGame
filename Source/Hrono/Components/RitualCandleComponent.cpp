#include "Components/RitualCandleComponent.h"

#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"
#include "TimerManager.h"

URitualCandleComponent::URitualCandleComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	FlameComponentNames = {TEXT("CandleFlame"), TEXT("CandleFlame1"), TEXT("CandleFlame2")};
}

void URitualCandleComponent::BeginPlay()
{
	Super::BeginPlay();
	if (AActor* Owner = GetOwner(); IsValid(Owner) && Owner->GetNetMode() != NM_Client
		&& !Owner->GetIsReplicated())
	{
		// A placed Blueprint instance may retain an old non-replicating override.
		Owner->SetReplicates(true);
	}
	ApplyVisualState();
}

void URitualCandleComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(IgnitionTimerHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void URitualCandleComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(URitualCandleComponent, VisualState);
}

void URitualCandleComponent::StartLightingForActor(AActor* CandleActor)
{
	if (IsValid(CandleActor))
	{
		if (URitualCandleComponent* Candle = CandleActor->FindComponentByClass<URitualCandleComponent>())
		{
			Candle->StartLighting();
		}
	}
}

void URitualCandleComponent::ReportMistakeForActor(AActor* CandleActor)
{
	if (IsValid(CandleActor))
	{
		if (URitualCandleComponent* Candle = CandleActor->FindComponentByClass<URitualCandleComponent>())
		{
			Candle->ReportMistake();
		}
	}
}

void URitualCandleComponent::StartLighting()
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || Owner->GetNetMode() == NM_Client || !Owner->HasAuthority()
		|| FlameComponentNames.IsEmpty())
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(IgnitionTimerHandle);
	NextIgnitionIndex = 0;
	SetVisualState(0, 0);
	GetWorld()->GetTimerManager().SetTimer(
		IgnitionTimerHandle, this, &URitualCandleComponent::IgniteNextFlame,
		FMath::Max(0.05f, IgnitionInterval), true);
}

void URitualCandleComponent::IgniteNextFlame()
{
	if (!IsValid(GetOwner()) || GetOwner()->GetNetMode() == NM_Client
		|| !GetOwner()->HasAuthority())
	{
		return;
	}
	const int32 Count = FMath::Min(FlameComponentNames.Num(), 8);
	if (NextIgnitionIndex >= Count)
	{
		GetWorld()->GetTimerManager().ClearTimer(IgnitionTimerHandle);
		return;
	}
	SetVisualState(GetLitMask() | static_cast<uint8>(1u << NextIgnitionIndex),
		GetHiddenBodyMask());
	++NextIgnitionIndex;
	if (NextIgnitionIndex >= Count)
	{
		GetWorld()->GetTimerManager().ClearTimer(IgnitionTimerHandle);
	}
}

void URitualCandleComponent::ReportMistake()
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || Owner->GetNetMode() == NM_Client || !Owner->HasAuthority())
	{
		return;
	}
	GetWorld()->GetTimerManager().ClearTimer(IgnitionTimerHandle);
	for (int32 Index = 0; Index < FMath::Min(FlameComponentNames.Num(), 8); ++Index)
	{
		const uint8 Bit = static_cast<uint8>(1u << Index);
		if ((GetLitMask() & Bit) != 0)
		{
			SetVisualState(GetLitMask() & ~Bit, GetHiddenBodyMask() | Bit);
			return;
		}
	}
}

void URitualCandleComponent::SetVisualState(uint8 NewLitMask, uint8 NewHiddenBodyMask)
{
	// Publish both visual aspects together so late join never sees a partial ritual state.
	const int32 NewState = static_cast<int32>(NewLitMask)
		| (static_cast<int32>(NewHiddenBodyMask) << 8);
	if (VisualState == NewState)
	{
		return;
	}
	VisualState = NewState;
	ApplyVisualState();
	if (AActor* Owner = GetOwner())
	{
		Owner->ForceNetUpdate();
	}
}

void URitualCandleComponent::OnRep_VisualState()
{
	ApplyVisualState();
}

USceneComponent* URitualCandleComponent::FindFlame(FName ComponentName) const
{
	if (AActor* Owner = GetOwner())
	{
		TInlineComponentArray<USceneComponent*> Components(Owner);
		for (USceneComponent* Component : Components)
		{
			if (IsValid(Component) && Component->GetFName() == ComponentName)
			{
				return Component;
			}
		}
	}
	return nullptr;
}

void URitualCandleComponent::ApplyVisualState()
{
	for (int32 Index = 0; Index < FMath::Min(FlameComponentNames.Num(), 8); ++Index)
	{
		if (USceneComponent* Flame = FindFlame(FlameComponentNames[Index]))
		{
			const bool bLit = (GetLitMask() & (1u << Index)) != 0;
			const bool bBodyVisible = (GetHiddenBodyMask() & (1u << Index)) == 0;
			if (USceneComponent* CandleBody = Flame->GetAttachParent())
			{
				CandleBody->SetVisibility(bBodyVisible, true);
			}
			Flame->SetVisibility(bLit, true);
			if (UParticleSystemComponent* Particle = Cast<UParticleSystemComponent>(Flame))
			{
				if (bLit && !Particle->IsActive())
				{
					Particle->ActivateSystem(true);
				}
				else if (!bLit && Particle->IsActive())
				{
					Particle->DeactivateSystem();
				}
			}
		}
	}
}
