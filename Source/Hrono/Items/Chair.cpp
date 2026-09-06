#include "Chair.h"
// Fill out your copyright notice in the Description page of Project Settings.
#include "HronoCharacter.h"
#include "Ritual/TableRitualGate.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

#include "Items/Chair.h"

AChair::AChair()
{
	SitPoint = CreateDefaultSubobject<USceneComponent>(TEXT("SitPoint"));
	StandUpPoint = CreateDefaultSubobject<USceneComponent>(TEXT("StandUpPoint"));

	if (ItemMesh)
	{
		SitPoint->SetupAttachment(ItemMesh);
		StandUpPoint->SetupAttachment(ItemMesh);
	}
}

void AChair::NotifyCharacterSat(AHronoCharacter* Character)
{
	OnCharacterSat.Broadcast(Character);
}

void AChair::Use_Implementation(AActor* Character)
{
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

    // bIsSit still reflects the state *before* this toggle, so play the sound
    // matching the action the player is about to perform.
    UGameplayStatics::PlaySoundAtLocation(this, bIsSit ? StandUpSound : SitSound, GetActorLocation());

    Hrono->SitOnChair(this);
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

	UGameplayStatics::PlaySoundAtLocation(
		this,
		SitSound,
		GetActorLocation());
	return true;
}

void AChair::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AChair, bIsSit);
    DOREPLIFETIME(AChair, CurrentSitter);
}
