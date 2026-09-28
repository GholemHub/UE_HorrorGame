// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Radio.h"
#include "Audio/HronoAudioLibrary.h"

UAudioComponent* ARadio::ReplaceRadioSound(UAudioComponent* Previous, USoundBase* Sound)
{
	return UHronoAudioLibrary::ReplaceSoundOnActor(Previous, Sound, this);
}

