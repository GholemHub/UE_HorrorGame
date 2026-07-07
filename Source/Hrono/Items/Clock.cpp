// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Clock.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

void AClock::Use_Implementation(AActor* Character)
{

    TimeNow = 0.0f;

    UGameplayStatics::PlaySoundAtLocation(this, UseSound, GetActorLocation());
}
