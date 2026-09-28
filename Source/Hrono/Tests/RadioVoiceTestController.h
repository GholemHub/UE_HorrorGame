#pragma once

#include "CoreMinimal.h"
#include "HronoPlayerController.h"
#include "UI/HronoMainMenuWidget.h"
#include "RadioVoiceTestController.generated.h"

/** Fake only the external voice backend; exercise the real controller state machine. */
UCLASS(Transient, NotBlueprintable)
class ARadioVoiceTestController : public AHronoPlayerController
{
	GENERATED_BODY()
public:
	bool bBackendReady = false;
	TArray<bool> CaptureCommands;
	void RefreshForTest() { ApplyRadioTransmissionState(); }
	void BindInputForTest() { SetupInputComponent(); }
protected:
	virtual bool PrepareRadioVoice() override { return bBackendReady; }
	virtual void ApplyRadioCapture(bool bEnabled) override { CaptureCommands.Add(bEnabled); }
};

/** Exposes widget lifecycle without adding it to the user's viewport or writing settings. */
UCLASS(Transient, NotBlueprintable)
class UAudioMenuTestWidget : public UHronoMainMenuWidget
{
	GENERATED_BODY()
public:
	void ConstructForTest() { NativeConstruct(); }
	void DestructForTest() { NativeDestruct(); }
};
