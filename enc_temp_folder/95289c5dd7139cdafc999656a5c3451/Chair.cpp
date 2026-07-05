#include "Chair.h"
// Fill out your copyright notice in the Description page of Project Settings.
#include "HronoCharacter.h"

#include "Items/Chair.h"

AChair::AChair()
{

}

void AChair::Use_Implementation(AActor* Character)
{
    AHronoCharacter* Hrono = Cast<AHronoCharacter>(Character);

    if (!Hrono)
        return;

    Hrono->SitOnChair(this);
}

void AChair::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AChair, bIsSit);
}