#include "Chair.h"
// Fill out your copyright notice in the Description page of Project Settings.
#include "HronoCharacter.h"
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

void AChair::Use_Implementation(AActor* Character)
{
    AHronoCharacter* Hrono = Cast<AHronoCharacter>(Character);

    if (!Hrono)
        return;

    // bIsSit still reflects the state *before* this toggle, so play the sound
    // matching the action the player is about to perform.
    UGameplayStatics::PlaySoundAtLocation(this, bIsSit ? StandUpSound : SitSound, GetActorLocation());

    Hrono->SitOnChair(this);
}

void AChair::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AChair, bIsSit);
}