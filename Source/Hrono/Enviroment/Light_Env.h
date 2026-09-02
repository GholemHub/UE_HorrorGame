// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SpotLightComponent.h"
#include "TimerManager.h"
#include "Light_Env.generated.h"

UCLASS()
class HRONO_API ALight_Env : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALight_Env();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** The actual light source. Spot lights are deliberately used because they are cheaper than point lights. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Light")
	TObjectPtr<USpotLightComponent> Light;

	/**
	 * Enables the low-cost horror flicker. This uses a timer only while active;
	 * the actor never ticks every frame. Use SetFlickering when changing it during play.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flicker")
	bool bFlickering = false;

	/** Lowest brightness multiplier used by an active flicker. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flicker", meta=(ClampMin="0.0", ClampMax="1.0"))
	float FlickerMinBrightness = 0.35f;

	/** Highest brightness multiplier used by an active flicker. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flicker", meta=(ClampMin="0.0", ClampMax="2.0"))
	float FlickerMaxBrightness = 1.0f;

	/** Random delay range between flicker updates. Keeping this above 0.04 avoids frame-rate-like updates. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flicker", meta=(ClampMin="0.04"))
	float FlickerIntervalMin = 0.06f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flicker", meta=(ClampMin="0.04"))
	float FlickerIntervalMax = 0.16f;

	/** Played when this light turns on. Assign any sound in Blueprint. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> LightOnSound;

	/** Played when this light turns off. Assign any sound in Blueprint. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> LightOffSound;
public:	
	/** Turns the light on/off and starts or stops flickering as needed. */
	UFUNCTION(BlueprintCallable, Category="Light")
	void SetLightEnabled(bool bNewState);

	/** Backwards-compatible entry point used by the existing switch actor. */
	UFUNCTION(BlueprintCallable, Category="Light")
	void OnSwith(bool NewState);

	/** Use this when enabling/disabling bFlickering during play. */
	UFUNCTION(BlueprintCallable, Category="Flicker")
	void SetFlickering(bool bEnabled);

	UFUNCTION(BlueprintPure, Category="Light")
	bool IsLightEnabled() const { return bLightEnabled; }

	UFUNCTION(BlueprintPure, Category="Flicker")
	bool IsFlickering() const { return bFlickering; }

private:
	void StartFlicker();
	void StopFlicker();
	void ApplyFlicker();

	FTimerHandle FlickerTimerHandle;
	float BaseIntensity = 0.0f;
	bool bLightEnabled = true;
};
