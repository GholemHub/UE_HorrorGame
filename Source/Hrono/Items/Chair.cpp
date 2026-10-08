#include "Chair.h"
#include "Audio/HronoAudioPolicy.h"
// Fill out your copyright notice in the Description page of Project Settings.
#include "HronoCharacter.h"
#include "Ritual/TableRitualGate.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

#include "Items/Chair.h"

AChair::AChair()
{
	ItemType = EItemType::Chair;
	bUseInteractionHighlight = false;
	static ConstructorHelpers::FObjectFinder<USoundBase> SeatCreak(
		TEXT("/Game/HorrorEngine/Audio/Interactions/S_Creak_06.S_Creak_06"));
	NativeSeatFallback = SeatCreak.Object;

	SitPoint = CreateDefaultSubobject<USceneComponent>(TEXT("SitPoint"));
	StandUpPoint = CreateDefaultSubobject<USceneComponent>(TEXT("StandUpPoint"));

	if (ItemMesh)
	{
		SitPoint->SetupAttachment(ItemMesh);
		StandUpPoint->SetupAttachment(ItemMesh);
	}
}

void AChair::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority()
		&& TableRitualGate::IsUnlocked(this)
		&& TableRitualGate::IsTableRitualChair(*this))
	{
		SetRitualGuidanceUnlocked(true);
	}
}

void AChair::SetRitualGuidanceUnlocked(bool bUnlocked)
{
	if (!HasAuthority() || bRitualGuidanceUnlocked == bUnlocked)
	{
		return;
	}

	if (bUnlocked)
	{
		// Ritual guidance must reach both timelines even when a placed Blueprint
		// overrode replication defaults or the chair is outside normal relevancy.
		SetReplicates(true);
		bAlwaysRelevant = true;
	}

	bRitualGuidanceUnlocked = bUnlocked;
	OnRep_RitualGuidanceUnlocked();
	FlushNetDormancy();
	ForceNetUpdate();
}

void AChair::OnRep_RitualGuidanceUnlocked()
{
	if (!bRitualGuidanceUnlocked)
	{
		SetInteractionContextHighlighted(false);
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PlayerController = It->Get();
		if (!IsValid(PlayerController) || !PlayerController->IsLocalController())
		{
			continue;
		}

		if (AHronoCharacter* LocalCharacter = Cast<AHronoCharacter>(PlayerController->GetPawn()))
		{
			LocalCharacter->RefreshRitualChairGuidanceNow();
		}
	}
}

void AChair::NotifyCharacterSat(AHronoCharacter* Character)
{
	OnCharacterSat.Broadcast(Character);
}

void AChair::SetSitter(AHronoCharacter* Character)
{
	if (!HasAuthority() || CurrentSitter == Character) return;
	CurrentSitter = Character;
	MulticastSeatSound(IsValid(Character), ItemTimeline);
	ForceNetUpdate();
}

void AChair::SetRitualStarted(bool bStarted)
{
	if (!HasAuthority() || IsRitualStarted == bStarted) return;
	SetReplicates(true);
	bAlwaysRelevant = true;
	IsRitualStarted = bStarted;
	ForceNetUpdate();
}

void AChair::MulticastSeatSound_Implementation(bool bSeated, EItemTimeline EventTimeline)
{
	if (HronoAudioPolicy::CanHear(this, EventTimeline))
	{
		USoundBase* Sound = bSeated ? SitSound.Get() : StandUpSound.Get();
		if (!IsValid(Sound)) Sound = NativeSeatFallback.Get();
		if (IsValid(Sound)) UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation());
	}
}

void AChair::Use_Implementation(AActor* Character)
{
	if (!HasAuthority())
	{
		return;
	}

    AHronoCharacter* Hrono = Cast<AHronoCharacter>(Character);

    if (!Hrono)
        return;

	if (!TableRitualGate::CanUseChair(*this))
	{
		UE_LOG(LogTemp, Log,
			TEXT("[TableRitualGate] %s cannot seat %s until the cursed image is picked up"),
			*GetName(), *GetNameSafe(Hrono));
		return;
	}

    Hrono->SitOnChair(this);
}

bool AChair::TryPickUp(AHronoCharacter* Character)
{
	if (!HasAuthority()
		|| !IsValid(Character)
		|| (ItemTimeline != EItemTimeline::Both
			&& ItemTimeline != Character->GetTimeline()))
	{
		return false;
	}

	Use(Character);
	return false;
}

bool AChair::OnBacktToRitualTable(AHronoCharacter* SelectedCharacter)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Chair] OnBacktToRitualTable ignored for %s because it was not called on the server"),
			*GetName());
		return false;
	}

	if (!IsValid(SelectedCharacter))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Chair] OnBacktToRitualTable failed for %s: Selected Character is invalid"),
			*GetName());
		return false;
	}

	// Prefer the exact chair reserved before the victim was moved to the ritual
	// point. Falling back to this chair preserves manual/non-ritual use.
	const bool bWasSeated = IsValid(SelectedCharacter->GetReservedRitualChair())
		? SelectedCharacter->ReturnToReservedRitualChair()
		: SelectedCharacter->ForceSitOnChair(this);
	if (!bWasSeated)
	{
		return false;
	}

	return true;
}

void AChair::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AChair, bIsSit);
    DOREPLIFETIME(AChair, CurrentSitter);
	DOREPLIFETIME(AChair, bRitualGuidanceUnlocked);
	DOREPLIFETIME(AChair, IsRitualStarted);
}
