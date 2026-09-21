// Copyright Epic Games, Inc. All Rights Reserved.


#include "HronoPlayerController.h"
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
	if (IsLocalPlayerController())
	{
		// Blueprint RegisterAllLocalTalkers implicitly enables voice. Restore the
		// radio switch after all pawn/Blueprint BeginPlay initialization completes.
		GetWorldTimerManager().SetTimerForNextTick(this, &ThisClass::ApplyRadioTransmissionState);
		GetWorldTimerManager().SetTimer(RadioBootstrapTimer, this,
			&ThisClass::ApplyRadioTransmissionState, 0.5f, false);
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
	GetWorldTimerManager().ClearTimer(RadioBootstrapTimer);
	if (IsLocalPlayerController())
	{
		StopTalking();
		bRadioTransmissionEnabled = false;
		if (AHronoCharacter* HronoCharacter = Cast<AHronoCharacter>(GetPawn()))
		{
			HronoCharacter->MicroStatus = false;
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AHronoPlayerController::ToggleRadioTransmission()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	AHronoCharacter* HronoCharacter = Cast<AHronoCharacter>(GetPawn());
	if (!HronoCharacter)
	{
		UE_LOG(LogHrono, Warning, TEXT("Cannot toggle radio microphone without an Hrono character."));
		return;
	}

	if (!bRadioTransmissionEnabled && !PrepareRadioVoice())
	{
		// A failed registration can still set the OSS networked-voice flag.
		StopTalking();
		HronoCharacter->MicroStatus = false;
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 8.0f, FColor::Red,
				TEXT("Microphone unavailable. Check the game session, Steam and Windows microphone permissions."));
		}
		HronoVoiceStatus();
		return;
	}

	bRadioTransmissionEnabled = !bRadioTransmissionEnabled;
	ApplyRadioTransmissionState();
	UE_LOG(LogHrono, Log, TEXT("Radio microphone %s for %s."),
		bRadioTransmissionEnabled ? TEXT("enabled") : TEXT("disabled"), *GetName());
}

void AHronoPlayerController::SetPawn(APawn* InPawn)
{
	if (AHronoCharacter* PreviousCharacter = Cast<AHronoCharacter>(GetPawn()))
	{
		PreviousCharacter->MicroStatus = false;
	}
	Super::SetPawn(InPawn);
	if (IsLocalPlayerController())
	{
		if (!InPawn)
		{
			GetWorldTimerManager().ClearTimer(RadioBootstrapTimer);
			bRadioTransmissionEnabled = false;
			StopTalking();
		}
		else
		{
			GetWorldTimerManager().SetTimerForNextTick(this, &ThisClass::ApplyRadioTransmissionState);
			// Client Blueprint InitVoiceChat is delayed after possession. Reapply
			// the current switch once it settles, preserving any early V press.
			GetWorldTimerManager().SetTimer(RadioBootstrapTimer, this,
				&ThisClass::ApplyRadioTransmissionState, 0.5f, false);
		}
	}
}

void AHronoPlayerController::ApplyRadioTransmissionState()
{
	if (!IsLocalPlayerController())
	{
		return;
	}
	if (AHronoCharacter* HronoCharacter = Cast<AHronoCharacter>(GetPawn()))
	{
		HronoCharacter->MicroStatus = bRadioTransmissionEnabled;
	}
	if (bRadioTransmissionEnabled)
	{
		// RegisterLocalTalker starts processing before assigning the capture owner
		// in UE 5.8. Start again AFTER registration so capture actually starts.
		StartTalking();
	}
	else
	{
		StopTalking();
	}
}

bool AHronoPlayerController::PrepareRadioVoice()
{
	const ULocalPlayer* LocalPlayer = GetLocalPlayer();
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!LocalPlayer || !Subsystem)
	{
		UE_LOG(LogHrono, Warning, TEXT("Radio voice: no local player or online subsystem."));
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
		UE_LOG(LogHrono, Warning, TEXT("Radio voice: session or local online identity is not ready (%s, user %d)."),
			*Subsystem->GetSubsystemName().ToString(), LocalUserNum);
		return false;
	}

	const IOnlineVoicePtr Voice = Subsystem->GetVoiceInterface();
	if (!Voice.IsValid() || !Voice->RegisterLocalTalker(LocalUserNum))
	{
		UE_LOG(LogHrono, Warning, TEXT("Radio voice: microphone registration failed (%s, user %d). Check the default Windows capture device and microphone access for desktop apps."),
			*Subsystem->GetSubsystemName().ToString(), LocalUserNum);
		return false;
	}

	// RegisterLocalTalker implicitly enables transmission. Restore the off state
	// until ToggleRadioTransmission explicitly starts the registered capture owner.
	Voice->StopNetworkedVoice(static_cast<uint8>(LocalUserNum));

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

	UE_LOG(LogHrono, Log, TEXT("Radio voice ready: subsystem=%s localUser=%d netMode=%d."),
		*Subsystem->GetSubsystemName().ToString(), LocalUserNum, static_cast<int32>(GetNetMode()));
	return true;
}

void AHronoPlayerController::HronoVoiceStatus()
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	const IOnlineVoicePtr Voice = Subsystem ? Subsystem->GetVoiceInterface() : nullptr;
	const IOnlineSessionPtr Sessions = Subsystem ? Subsystem->GetSessionInterface() : nullptr;
	UE_LOG(LogHrono, Warning, TEXT("Radio voice status: subsystem=%s sessions=%d local=%d user=%d requested=%d\n%s"),
		Subsystem ? *Subsystem->GetSubsystemName().ToString() : TEXT("None"),
		Sessions.IsValid() ? Sessions->GetNumSessions() : 0,
		IsLocalPlayerController(), GetLocalPlayer() ? GetLocalPlayer()->GetControllerId() : INDEX_NONE,
		bRadioTransmissionEnabled, Voice.IsValid() ? *Voice->GetVoiceDebugState() : TEXT("Voice interface unavailable"));
}

bool AHronoPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
