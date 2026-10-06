#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Enviroment/PianoActor.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/WorldSettings.h"
#include "HronoCharacter.h"
#include "HronoCollisionChannels.h"
#include "Sound/SoundBase.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPianoInteractionTest, "Hrono.Audio.PianoInteraction",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPianoInteractionTest::RunTest(const FString& Parameters)
{
	const auto Settings = UWorld::InitializationValues().AllowAudioPlayback(false)
		.CreatePhysicsScene(true).CreateNavigation(false).CreateAISystem(false)
		.ShouldSimulatePhysics(true).SetTransactional(false);
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr, true,
		ERHIFeatureLevel::Num, &Settings);
	GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	World->BeginPlay();
	World->GetWorldSettings()->NotifyBeginPlay();

	UClass* PianoClass = LoadClass<APianoActor>(nullptr, TEXT("/Game/_Alex/BP_Piano.BP_Piano_C"));
	if (TestNotNull(TEXT("authored BP_Piano"), PianoClass))
	{
		const APianoActor* Defaults = PianoClass->GetDefaultObject<APianoActor>();
		TestNotNull(TEXT("piano mesh assigned"), Defaults->PianoMesh->GetStaticMesh().Get());
		TestNotNull(TEXT("piano sound assigned"), Defaults->PianoSound.Get());
		TestNotNull(TEXT("piano attenuation assigned"), Defaults->SoundAttenuation.Get());
		TestEqual(TEXT("Past trace blocks the piano box"),
			Defaults->InteractionBox->GetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST), ECR_Block);
		TestEqual(TEXT("Future trace blocks the piano box"),
			Defaults->InteractionBox->GetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE), ECR_Block);

		FActorSpawnParameters Spawn;
		Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		UClass* CharacterClass = LoadClass<AHronoCharacter>(nullptr,
			TEXT("/Game/_Alex/HE_CharacterHrono1.HE_CharacterHrono1_C"));
		AHronoCharacter* Player = CharacterClass ? World->SpawnActor<AHronoCharacter>(
			CharacterClass, FVector::ZeroVector, FRotator::ZeroRotator, Spawn) : nullptr;
		APlayerController* Controller = World->SpawnActor<APlayerController>();
		if (TestNotNull(TEXT("gameplay character"), Player)
			&& TestNotNull(TEXT("test controller"), Controller))
		{
			Controller->Possess(Player);
			Player->CharacterTimeline = EItemTimeline::Past;
			const UCameraComponent* Camera = Player->GetFirstPersonCameraComponent();
			if (TestNotNull(TEXT("character camera"), Camera))
			{
				const FVector NearLocation = Camera->GetComponentLocation()
					+ Camera->GetForwardVector() * 200.0f;
				APianoActor* Piano = World->SpawnActor<APianoActor>(PianoClass,
					NearLocation, FRotator::ZeroRotator, Spawn);
				if (TestNotNull(TEXT("spawned piano"), Piano))
				{
					TestTrue(TEXT("piano has authority in test world"), Piano->HasAuthority());
					TestTrue(TEXT("near piano passes existing server interaction check"),
						Player->CanInteractWithActorOnServer(Piano));
					const double Before = Piano->GetLastPlayTimeSeconds();
					Player->Server_InteractWithEnvironment(Piano);
					const double FirstPlay = Piano->GetLastPlayTimeSeconds();
					TestTrue(TEXT("server press starts the one-shot"), FirstPlay > Before);
					Player->Server_InteractWithEnvironment(Piano);
					TestEqual(TEXT("rapid second press respects cooldown"),
						Piano->GetLastPlayTimeSeconds(), FirstPlay);
					Piano->SetActorLocation(NearLocation + Camera->GetForwardVector() * 1500.0f);
					TestFalse(TEXT("distant piano fails server interaction check"),
						Player->CanInteractWithActorOnServer(Piano));
					Player->Server_InteractWithEnvironment(Piano);
					TestEqual(TEXT("distant press does not play"),
						Piano->GetLastPlayTimeSeconds(), FirstPlay);
				}
			}
		}
	}
	World->EndPlay(EEndPlayReason::Quit);
	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return true;
}

#endif
