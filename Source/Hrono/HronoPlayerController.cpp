// Copyright Epic Games, Inc. All Rights Reserved.


#include "HronoPlayerController.h"
#include "Audio/HronoAudioSettingsSubsystem.h"
#include "Engine/GameInstance.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "HronoCameraManager.h"
#include "HronoCharacter.h"
#include "Blueprint/UserWidget.h"
#include "Hrono.h"
#include "Engine/Engine.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/VoiceInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "TimerManager.h"
#include "Widgets/Input/SVirtualJoystick.h"

AHronoPlayerController::AHronoPlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = AHronoCameraManager::StaticClass();
}

void AHronoPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UHronoAudioSettingsSubsystem* Audio = GameInstance->GetSubsystem<UHronoAudioSettingsSubsystem>())
			Audio->ApplyToWorld(GetWorld());
	}
	if (IsLocalPlayerController())
	{
		GetWorldTimerManager().SetTimer(RadioReadinessTimer, this,
			&ThisClass::ApplyRadioTransmissionState, 0.5f, true);
	}

	
	// only spawn touch controls on local player controllers
	if (ShouldUseTouchControls() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogHrono, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void AHronoPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Keep radio voice independent of Blueprint/Enhanced Input assets. This
		// controller is instantiated for both the listen server and remote clients,
		// so each local player transmits through the same online voice path.
		InputComponent->BindKey(EKeys::V, IE_Pressed, this, &ThisClass::ToggleRadioTransmission);
		InputComponent->BindKey(EKeys::B, IE_Pressed, this, &ThisClass::ToggleRadioTransmission);

		// Add Input Mapping Context
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
	
}

void AHronoPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(RadioReadinessTimer);
	if (IsLocalPlayerController())
	{
		bRadioTransmissionEnabled = false;
		bRadioTransmissionActive = false;
		ApplyRadioCapture(false);
		ResetRadioRegistration();
		if (AHronoCharacter* HronoCharacter = Cast<AHronoCharacter>(GetPawn()))
		{
			HronoCharacter->MicroStatus = false;
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AHronoPlayerController::ToggleRadioTransmission()
{
	SetRadioTransmissionEnabled(!bRadioTransmissionEnabled);
}

void AHronoPlayerController::SetRadioTransmissionEnabled(bool bEnabled)
{
	if (!IsLocalPlayerController()) return;
	bRadioTransmissionEnabled = bEnabled && IsValid(Cast<AHronoCharacter>(GetPawn()));
	ApplyRadioTransmissionState();
}

void AHronoPlayerController::ToggleSpeaking(bool bSpeaking)
{
	SetRadioTransmissionEnabled(bSpeaking);
}

void AHronoPlayerController::ApplyRadioCapture(bool bEnabled)
{
	// Bypass our override only here, after validating the canonical request.
	Super::ToggleSpeaking(bEnabled);
}

void AHronoPlayerController::NotifyVoiceReceiverReady(APawn* ReadyPawn)
{
	if (!IsLocalPlayerController() || !IsValid(ReadyPawn) || ReadyPawn != GetPawn()) return;
	bVoiceReceiverReady = true;
	ApplyRadioTransmissionState();
}

void AHronoPlayerController::ResetRadioRegistration()
{
	if (const IOnlineVoicePtr Voice = RegisteredRadioVoice.Pin(); Voice.IsValid() && RegisteredRadioUser >= 0)
		Voice->StopNetworkedVoice(static_cast<uint8>(RegisteredRadioUser));
	RegisteredRadioVoice.Reset();
	RegisteredRadioUser = INDEX_NONE;
}

void AHronoPlayerController::SetPawn(APawn* InPawn)
{
	if (GetPawn() == InPawn) return;
	if (AHronoCharacter* PreviousCharacter = Cast<AHronoCharacter>(GetPawn()))
	{
		PreviousCharacter->MicroStatus = false;
	}
	bVoiceReceiverReady = false;
	Super::SetPawn(InPawn);
	if (const AHronoCharacter* HronoPawn = Cast<AHronoCharacter>(InPawn))
		bVoiceReceiverReady = HronoPawn->HasConfiguredVoiceReceiver();
	if (IsLocalPlayerController())
	{
		if (!InPawn)
		{
			bRadioTransmissionEnabled = false;
			ResetRadioRegistration();
		}
		ApplyRadioTransmissionState();
	}
}

void AHronoPlayerController::ApplyRadioTransmissionState()
{
	if (!IsLocalPlayerController())
	{
		return;
	}
	AHronoCharacter* HronoCharacter = Cast<AHronoCharacter>(GetPawn());
	// A replica's Controller pointer can arrive after its receiver callback.
	if (IsValid(HronoCharacter) && HronoCharacter->HasConfiguredVoiceReceiver())
		bVoiceReceiverReady = true;
	const bool bReady = IsValid(HronoCharacter) && bVoiceReceiverReady && PrepareRadioVoice();
	const bool bShouldTransmit = bRadioTransmissionEnabled && bReady;
	if (bRadioTransmissionActive != bShouldTransmit)
	{
		bRadioTransmissionActive = bShouldTransmit;
		ApplyRadioCapture(bShouldTransmit);
		UE_LOG(LogHrono, Log, TEXT("Radio microphone %s for %s."),
			bShouldTransmit ? TEXT("enabled") : TEXT("disabled"), *GetName());
	}
	if (HronoCharacter) HronoCharacter->MicroStatus = bRadioTransmissionActive;
}

bool AHronoPlayerController::PrepareRadioVoice()
{
	const ULocalPlayer* LocalPlayer = GetLocalPlayer();
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!LocalPlayer || !Subsystem)
	{
		ResetRadioRegistration();
		return false;
	}

	const int32 LocalUserNum = LocalPlayer->GetControllerId();
	const IOnlineSessionPtr Sessions = Subsystem->GetSessionInterface();
	const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
	// The OSS voice tick does not process capture OR playback without a session.
	if (!Sessions.IsValid() || Sessions->GetNumSessions() == 0 || !Identity.IsValid()
		|| LocalUserNum < 0 || LocalUserNum > MAX_uint8
		|| !Identity->GetUniquePlayerId(LocalUserNum).IsValid())
	{
		ResetRadioRegistration();
		return false;
	}

	const IOnlineVoicePtr Voice = Subsystem->GetVoiceInterface();
	if (!Voice.IsValid())
	{
		ResetRadioRegistration();
		return false;
	}
	if (RegisteredRadioVoice.Pin() != Voice || RegisteredRadioUser != LocalUserNum)
	{
		ResetRadioRegistration();
		const bool bRegistered = Voice->RegisterLocalTalker(LocalUserNum);
		// Even a failed registration can enable the networked-voice flag.
		Voice->StopNetworkedVoice(static_cast<uint8>(LocalUserNum));
		if (!bRegistered) return false;
		RegisteredRadioVoice = Voice;
		RegisteredRadioUser = LocalUserNum;
		// A newly registered backend needs a fresh Start, even if requested ON
		// survived a subsystem/session replacement.
		bRadioTransmissionActive = false;
		UE_LOG(LogHrono, Log, TEXT("Radio voice ready: subsystem=%s localUser=%d netMode=%d."),
			*Subsystem->GetSubsystemName().ToString(), LocalUserNum, static_cast<int32>(GetNetMode()));
	}

	// A client's PlayerStates may arrive after its session's initial voice setup.
	// Ensure receiving works for both the host and joined clients, keeping mute settings.
	if (const AGameStateBase* GameState = GetWorld()->GetGameState())
	{
		const FUniqueNetIdPtr LocalId = Identity->GetUniquePlayerId(LocalUserNum);
		for (const APlayerState* RemotePlayer : GameState->PlayerArray)
		{
			if (IsValid(RemotePlayer) && RemotePlayer->GetUniqueId().IsValid()
				&& *RemotePlayer->GetUniqueId() != *LocalId)
			{
				Voice->RegisterRemoteTalker(*RemotePlayer->GetUniqueId());
			}
		}
	}

	return Voice->IsHeadsetPresent(LocalUserNum);
}

void AHronoPlayerController::HronoVoiceStatus()
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	const IOnlineVoicePtr Voice = Subsystem ? Subsystem->GetVoiceInterface() : nullptr;
	const IOnlineSessionPtr Sessions = Subsystem ? Subsystem->GetSessionInterface() : nullptr;
	UE_LOG(LogHrono, Warning, TEXT("Radio voice status: subsystem=%s sessions=%d local=%d user=%d requested=%d active=%d receiverReady=%d\n%s"),
		Subsystem ? *Subsystem->GetSubsystemName().ToString() : TEXT("None"),
		Sessions.IsValid() ? Sessions->GetNumSessions() : 0,
		IsLocalPlayerController(), GetLocalPlayer() ? GetLocalPlayer()->GetControllerId() : INDEX_NONE,
		bRadioTransmissionEnabled, bRadioTransmissionActive, bVoiceReceiverReady,
		Voice.IsValid() ? *Voice->GetVoiceDebugState() : TEXT("Voice interface unavailable"));
}

bool AHronoPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
