#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/RadioVoiceTestController.h"
#include "Audio/HronoAudioSettingsSubsystem.h"
#include "AudioDevice.h"
#include "AudioDeviceManager.h"
#include "Components/InputComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "HronoCharacter.h"
#include "InputCoreTypes.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "UObject/StrongObjectPtr.h"
#include <limits>

namespace AudioVoiceTests
{
struct FGameScope
{
	TStrongObjectPtr<UGameInstance> Game;
	UWorld* World;
	bool bShutdown = false;
	FGameScope() : Game(NewObject<UGameInstance>(GEngine))
	{
		Game->InitializeStandalone();
		World = Game->GetWorld();
		World->InitializeActorsForPlay(FURL());
	}
	~FGameScope()
	{
		Shutdown();
		World->DestroyWorld(false);
		GEngine->DestroyWorldContext(World);
	}
	void Shutdown()
	{
		if (!bShutdown) Game->Shutdown();
		bShutdown = true;
	}
};

void Slider(UHronoMainMenuWidget* Widget, const TCHAR* Name, float Value)
{
	UFunction* Function = Widget->FindFunctionChecked(Name);
	Widget->ProcessEvent(Function, &Value);
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRadioTransmissionOwnerTest,
	"Hrono.AudioVoice.TransmissionOwner", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRadioTransmissionOwnerTest::RunTest(const FString& Parameters)
{
	using namespace AudioVoiceTests;
	FGameScope Scope;
	ARadioVoiceTestController* Controller = Scope.World->SpawnActor<ARadioVoiceTestController>();
	Controller->Player = NewObject<ULocalPlayer>(GEngine);
	UClass* CharacterClass = LoadClass<AHronoCharacter>(nullptr, TEXT("/Game/_Alex/HE_CharacterHrono1.HE_CharacterHrono1_C"));
	if (!TestNotNull(TEXT("Gameplay character"), CharacterClass)) return false;
	FActorSpawnParameters Spawn;
	Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AHronoCharacter* First = Scope.World->SpawnActor<AHronoCharacter>(CharacterClass, FVector::ZeroVector, FRotator::ZeroRotator, Spawn);
	AHronoCharacter* Second = Scope.World->SpawnActor<AHronoCharacter>(CharacterClass, FVector(500,0,0), FRotator::ZeroRotator, Spawn);
	Controller->SetPawn(First);
	TestFalse(TEXT("Initial request OFF"), Controller->IsRadioTransmissionRequested());
	Controller->ToggleRadioTransmission();
	TestTrue(TEXT("Early toggle retains intent"), Controller->IsRadioTransmissionRequested());
	TestFalse(TEXT("No capture before receiver is ready"), Controller->IsRadioTransmissionActive());
	TestFalse(TEXT("UI stays OFF before readiness"), First->MicroStatus);
	Controller->bBackendReady = true;
	Controller->NotifyVoiceReceiverReady(Second);
	TestFalse(TEXT("Stale other-pawn readiness cannot start capture"), Controller->IsRadioTransmissionActive());
	Controller->NotifyVoiceReceiverReady(First);
	TestTrue(TEXT("Actual receiver callback applies queued request"), Controller->IsRadioTransmissionActive());
	TestTrue(TEXT("UI agrees with active state"), First->MicroStatus);
	const int32 Commands = Controller->CaptureCommands.Num();
	Controller->SetRadioTransmissionEnabled(true);
	Controller->RefreshForTest();
	TestEqual(TEXT("Duplicate ON and readiness poll do not restart capture"), Controller->CaptureCommands.Num(), Commands);
	Controller->ToggleRadioTransmission();
	TestFalse(TEXT("Second toggle OFF"), Controller->IsRadioTransmissionRequested());
	TestFalse(TEXT("Second toggle stops applied state"), Controller->IsRadioTransmissionActive());
	Controller->StartTalking();
	TestTrue(TEXT("Inherited StartTalking routes through canonical owner"), Controller->IsRadioTransmissionRequested());
	Controller->StopTalking();
	TestFalse(TEXT("Inherited StopTalking clears the canonical request"), Controller->IsRadioTransmissionRequested());
	Controller->ToggleSpeaking(true);
	TestTrue(TEXT("Console speaking uses canonical applied state"), Controller->IsRadioTransmissionActive());
	Controller->bBackendReady = false;
	Controller->RefreshForTest();
	TestFalse(TEXT("Session/backend loss stops capture"), Controller->IsRadioTransmissionActive());
	TestFalse(TEXT("Session/backend loss clears UI"), First->MicroStatus);
	TestTrue(TEXT("Temporary readiness loss keeps explicit request"), Controller->IsRadioTransmissionRequested());
	Controller->bBackendReady = true;
	Controller->RefreshForTest();
	TestTrue(TEXT("Readiness recovery reapplies intent"), Controller->IsRadioTransmissionActive());
	Controller->SetPawn(Second);
	TestFalse(TEXT("Replacement pawn waits for its own receiver"), Controller->IsRadioTransmissionActive());
	TestFalse(TEXT("Old pawn UI cleared"), First->MicroStatus);
	Controller->NotifyVoiceReceiverReady(First);
	TestFalse(TEXT("Late old-pawn callback stays ignored"), Controller->IsRadioTransmissionActive());
	Controller->NotifyVoiceReceiverReady(Second);
	TestTrue(TEXT("New-pawn callback resumes intended transmission"), Second->MicroStatus);
	Controller->SetPawn(nullptr);
	TestFalse(TEXT("Pawn loss clears request"), Controller->IsRadioTransmissionRequested());
	TestFalse(TEXT("Pawn loss stops capture"), Controller->IsRadioTransmissionActive());
	Controller->ToggleSpeaking(true);
	TestFalse(TEXT("Speaking without a character cannot create a request"), Controller->IsRadioTransmissionRequested());
	// Receiver setup is replica-local and may precede possession on a client.
	Second->NotifyVoiceReceiverReady();
	TestTrue(TEXT("Receiver remembers setup before a controller exists"), Second->HasConfiguredVoiceReceiver());
	Controller->SetPawn(Second);
	Controller->SetRadioTransmissionEnabled(true);
	TestTrue(TEXT("Late possession consumes existing receiver readiness"), Controller->IsRadioTransmissionActive());
	Controller->SetPawn(nullptr);

	Controller->InputComponent = NewObject<UInputComponent>(Controller);
	Controller->BindInputForTest();
	int32 BCount = 0, VCount = 0;
	for (const FInputKeyBinding& Key : Controller->InputComponent->KeyBindings)
	{
		if (Key.KeyEvent == IE_Pressed && Key.Chord.Key == EKeys::B) ++BCount;
		if (Key.KeyEvent == IE_Pressed && Key.Chord.Key == EKeys::V) ++VCount;
	}
	TestEqual(TEXT("One B controller binding"), BCount, 1);
	TestEqual(TEXT("One V controller binding"), VCount, 1);
	Controller->SetPawn(First);
	Controller->NotifyVoiceReceiverReady(First);
	for (const FKey PressedKey : { EKeys::V, EKeys::B, EKeys::V, EKeys::B })
	{
		const bool bExpected = !Controller->IsRadioTransmissionRequested();
		for (const FInputKeyBinding& Binding : Controller->InputComponent->KeyBindings)
			if (Binding.KeyEvent == IE_Pressed && Binding.Chord.Key == PressedKey)
				Binding.KeyDelegate.Execute(PressedKey);
		TestEqual(TEXT("V/B sequence shares one requested state"), Controller->IsRadioTransmissionRequested(), bExpected);
		TestEqual(TEXT("V/B sequence shares one applied state"), Controller->IsRadioTransmissionActive(), bExpected);
		TestEqual(TEXT("V/B sequence keeps UI synchronized"), First->MicroStatus, bExpected);
	}
	Controller->SetPawn(nullptr);
	ARadioVoiceTestController* Remote = Scope.World->SpawnActor<ARadioVoiceTestController>();
	Remote->SetPawn(First);
	Remote->NotifyVoiceReceiverReady(First);
	Remote->SetRadioTransmissionEnabled(true);
	TestFalse(TEXT("Remote/non-local controller cannot transmit"), Remote->IsRadioTransmissionRequested());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUserAudioMixLifetimeTest,
	"Hrono.AudioVoice.UserAudioMixLifetime", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUserAudioMixLifetimeTest::RunTest(const FString& Parameters)
{
	using namespace AudioVoiceTests;
	FGameScope Scope;
	UHronoAudioSettingsSubsystem* Audio = Scope.Game->GetSubsystem<UHronoAudioSettingsSubsystem>();
	if (!TestNotNull(TEXT("Persistent GameInstance audio owner"), Audio)) return false;
	FAudioDeviceManager* Manager = GEngine->GetAudioDeviceManager();
	if (!TestNotNull(TEXT("Audio device manager (run without -nosound)"), Manager)) return false;
	FAudioDeviceParams DeviceParams;
	DeviceParams.AssociatedWorld = Scope.World;
	DeviceParams.Scope = EAudioDeviceScope::Unique;
	DeviceParams.bIsNonRealtime = true;
	FAudioDeviceHandle Device = Manager->RequestAudioDevice(DeviceParams);
	if (!TestTrue(TEXT("Offline renderer created without speaker output"), Device.IsValid())) return false;
	Scope.World->SetAudioDevice(Device);
	Audio->SetVolumes(0.5f, 0.25f, 0.75f, 0.0f);
	Device->Update(true);
	Device->Update(true);
	auto Volume = [&](const TCHAR* Path)
	{
		USoundClass* Class = LoadObject<USoundClass>(nullptr, Path);
		const FSoundClassProperties* Properties = Device->GetSoundClassCurrentProperties(Class);
		return Properties ? Properties->Volume : -1.0f;
	};
	TestEqual(TEXT("Music uses Master multiplied by Music"), Volume(TEXT("/Game/_Alex/Audio/SC_HronoMusic.SC_HronoMusic")), 0.125f);
	TestEqual(TEXT("SFX uses Master multiplied by SFX"), Volume(TEXT("/Game/_Alex/Audio/SC_HronoSFX.SC_HronoSFX")), 0.375f);
	TestEqual(TEXT("Legacy effects inherit SFX"), Volume(TEXT("/Game/HorrorEngine/Audio/_SoundSettings/SC_Effects.SC_Effects")), 0.375f);
	TestEqual(TEXT("Weather inherits SFX"), Volume(TEXT("/Game/UltraDynamicSky/Sound/UDS_Weather.UDS_Weather")), 0.375f);
	TestEqual(TEXT("Voice follows Master independently of SFX"), Volume(TEXT("/Game/_Alex/Audio/SC_HronoVoice.SC_HronoVoice")), 0.5f);
	Audio->ApplyToWorld(Scope.World);
	Audio->ApplyToWorld(Scope.World);
	Device->Update(true);
	TestEqual(TEXT("Repeated apply does not multiply attenuation"), Volume(TEXT("/Game/_Alex/Audio/SC_HronoMusic.SC_HronoMusic")), 0.125f);
	for (const auto& Mix : Device->GetSoundMixModifiers())
		if (Mix.Key->GetOuter() == Audio) TestEqual(TEXT("Owner pushes mix exactly once"), Mix.Value.ActiveRefCount, 1u);

	// Real widget preview, Cancel and destruction must use the same owner.
	TStrongObjectPtr<UAudioMenuTestWidget> Menu(CreateWidget<UAudioMenuTestWidget>(Scope.Game.Get()));
	if (!TestNotNull(TEXT("Native menu fixture"), Menu.Get())) return false;
	Menu->ConstructForTest();
	Slider(Menu.Get(), TEXT("HandleMusicVolumeChanged"), 0.0f);
	TestEqual(TEXT("Menu slider previews through persistent owner"), Audio->MusicVolume, 0.0f);
	Menu->CancelSettings();
	TestEqual(TEXT("Cancel restores category volume"), Audio->MusicVolume, 0.25f);
	Slider(Menu.Get(), TEXT("HandleSfxVolumeChanged"), 0.0f);
	Menu->DestructForTest();
	TestEqual(TEXT("Closing an uncommitted preview restores volume"), Audio->SfxVolume, 0.75f);
	Device->Update(true);
	for (const auto& Mix : Device->GetSoundMixModifiers())
		if (Mix.Key->GetOuter() == Audio) TestEqual(TEXT("Widget lifetime adds no extra mix reference"), Mix.Value.ActiveRefCount, 1u);
	Audio->SetVolumes(0.5f, 0.0f, 1.0f, 0.0f);
	Device->Update(true);
	TestEqual(TEXT("Music zero really mutes music branch"), Volume(TEXT("/Game/_Alex/Audio/SC_HronoMusic.SC_HronoMusic")), 0.0f);
	TestEqual(TEXT("Music zero leaves SFX branch unchanged"), Volume(TEXT("/Game/_Alex/Audio/SC_HronoSFX.SC_HronoSFX")), 0.5f);
	Audio->SetVolumes(0.0f, 1.0f, 1.0f, 0.0f);
	Device->Update(true);
	TestEqual(TEXT("Master zero mutes voice"), Volume(TEXT("/Game/_Alex/Audio/SC_HronoVoice.SC_HronoVoice")), 0.0f);
	Audio->SetVolumes(-1.0f, 2.0f, std::numeric_limits<float>::quiet_NaN(), 0.0f);
	TestEqual(TEXT("Negative saved volume clamped"), Audio->MasterVolume, 0.0f);
	TestEqual(TEXT("Oversized saved volume clamped"), Audio->MusicVolume, 1.0f);
	TestEqual(TEXT("Non-finite volume rejected"), Audio->SfxVolume, 1.0f);
	Scope.Shutdown();
	Device->Update(true);
	for (const auto& Mix : Device->GetSoundMixModifiers())
		if (Mix.Key->GetOuter() == Audio) TestEqual(TEXT("Deinitialize releases mix push"), Mix.Value.ActiveRefCount, 0u);
	return true;
}

#endif
