// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "HronoPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;

/**
 *  Simple first person Player Controller
 *  Manages the input mapping context.
 *  Overrides the Player Camera Manager class.
 */
UCLASS(abstract, config="Game")
class HRONO_API AHronoPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	/** Constructor */
	AHronoPlayerController();

	/** Radio input and console diagnostics share the same local voice setup. */
	UFUNCTION(Exec, BlueprintCallable, Category = "Voice|Radio")
	void ToggleRadioTransmission();

	UFUNCTION(Exec)
	void HronoVoiceStatus();

	virtual void SetPawn(APawn* InPawn) override;

protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Always stop transmitting if the local controller is removed. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;

	/** Validate the session and register the microphone owner before starting capture. */
	bool PrepareRadioVoice();

	/** Reapply the switch after Blueprint/session bootstrap and pawn replacement. */
	void ApplyRadioTransmissionState();

	/** Late client Blueprint voice setup can run after the first possession tick. */
	FTimerHandle RadioBootstrapTimer;

	/** The controller owns transmission state; pawn defaults may change on respawn. */
	bool bRadioTransmissionEnabled = false;
};
