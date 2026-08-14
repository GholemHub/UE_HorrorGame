// Fill out your copyright notice in the Description page of Project Settings.


#include "Enviroment/Light_Env.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

// Sets default values
ALight_Env::ALight_Env()
{
	// Flicker is timer-driven, so a room full of these actors has no per-frame actor tick cost.
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;
	Light = CreateDefaultSubobject<USpotLightComponent>(TEXT("Light"));
	Light->SetupAttachment(RootComponent);

	// Conservative defaults for roughly 15 placed lights: a tight, short-range, non-shadowing
	// spot light creates pools of sickly moonlight without the cost of large dynamic point lights.
	Light->SetMobility(EComponentMobility::Movable);
	Light->SetIntensity(1200.0f);
	Light->SetLightColor(FLinearColor(0.34f, 0.48f, 0.58f));
	Light->SetAttenuationRadius(650.0f);
	Light->SetInnerConeAngle(24.0f);
	Light->SetOuterConeAngle(38.0f);
	Light->SetCastShadows(false);
	Light->SetAffectTranslucentLighting(false);
	Light->SetVolumetricScatteringIntensity(0.35f);
	bReplicates = true;
}

// Called when the game starts or when spawned
void ALight_Env::BeginPlay()
{
	Super::BeginPlay();

	BaseIntensity = Light->Intensity;
	bLightEnabled = Light->IsVisible();
	if (bLightEnabled && bFlickering)
	{
		StartFlicker();
	}
}

void ALight_Env::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopFlicker();
	Super::EndPlay(EndPlayReason);
}

void ALight_Env::OnSwith(bool NewState)
{
	SetLightEnabled(NewState);
}

void ALight_Env::SetLightEnabled(bool bNewState)
{
	if (bLightEnabled == bNewState)
	{
		return;
	}

	bLightEnabled = bNewState;
	Light->SetVisibility(bLightEnabled);

	if (bLightEnabled)
	{
		Light->SetIntensity(BaseIntensity > 0.0f ? BaseIntensity : Light->Intensity);
		if (bFlickering)
		{
			StartFlicker();
		}
	}
	else
	{
		StopFlicker();
	}

	if (USoundBase* Sound = bLightEnabled ? LightOnSound : LightOffSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation());
	}
}

void ALight_Env::SetFlickering(bool bEnabled)
{
	bFlickering = bEnabled;
	if (bFlickering && bLightEnabled)
	{
		StartFlicker();
	}
	else
	{
		StopFlicker();
		if (BaseIntensity > 0.0f)
		{
			Light->SetIntensity(BaseIntensity);
		}
	}
}

void ALight_Env::StartFlicker()
{
	if (!GetWorld() || GetWorldTimerManager().IsTimerActive(FlickerTimerHandle))
	{
		return;
	}

	ApplyFlicker();
}

void ALight_Env::StopFlicker()
{
	if (GetWorld())
	{
		GetWorldTimerManager().ClearTimer(FlickerTimerHandle);
	}
}

void ALight_Env::ApplyFlicker()
{
	if (!bLightEnabled || !bFlickering || !GetWorld())
	{
		StopFlicker();
		return;
	}

	const float MinBrightness = FMath::Min(FlickerMinBrightness, FlickerMaxBrightness);
	const float MaxBrightness = FMath::Max(FlickerMinBrightness, FlickerMaxBrightness);
	Light->SetIntensity(BaseIntensity * FMath::FRandRange(MinBrightness, MaxBrightness));

	const float MinInterval = FMath::Max(0.04f, FMath::Min(FlickerIntervalMin, FlickerIntervalMax));
	const float MaxInterval = FMath::Max(MinInterval, FMath::Max(FlickerIntervalMin, FlickerIntervalMax));
	GetWorldTimerManager().SetTimer(FlickerTimerHandle, this, &ALight_Env::ApplyFlicker,
		FMath::FRandRange(MinInterval, MaxInterval), false);
}

