#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "HronoCharacter.h"
#include "Items/Drag_Item.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/WorldSettings.h"
#include "Engine/LocalPlayer.h"
#include "InputCoreTypes.h"
#include "Components/InputComponent.h"
#include "Camera/CameraComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEmbossPreviewTest,
	"Hrono.Presentation.EmbossPreview",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEmbossPreviewTest::RunTest(const FString& Parameters)
{
	const auto Settings = UWorld::InitializationValues().AllowAudioPlayback(false)
		.CreatePhysicsScene(true).CreateNavigation(false).CreateAISystem(false)
		.ShouldSimulatePhysics(false).SetTransactional(false);
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr, true,
		ERHIFeatureLevel::Num, &Settings);
	GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	World->BeginPlay();
	World->GetWorldSettings()->NotifyBeginPlay();

	UClass* CharacterClass = LoadClass<AHronoCharacter>(nullptr,
		TEXT("/Game/_Alex/HE_CharacterHrono1.HE_CharacterHrono1_C"));
	APlayerController* Controller = World->SpawnActor<APlayerController>();
	AHronoCharacter* Character = CharacterClass
		? World->SpawnActor<AHronoCharacter>(CharacterClass) : nullptr;
	if (TestNotNull(TEXT("Authored character"), Character)
		&& TestNotNull(TEXT("Local controller"), Controller))
	{
		Controller->Player = NewObject<ULocalPlayer>(GEngine);
		Controller->SetAsLocalPlayerController();
		Controller->Possess(Character);
		Character->GetCharacterMovement()->DisableMovement();
		TestTrue(TEXT("Test character has a local owner"), Character->IsLocallyControlled());
		TestTrue(TEXT("Preview material is configured"), IsValid(Character->EmbossPostProcessMaterial));
		TestEqual(TEXT("Emboss scalar name matches MI_Emboss"),
			Character->EmbossIntensityParameterName, FName(TEXT("Emboss Intensity")));
		TestEqual(TEXT("Local intensity begins at zero"), Character->EmbossCurrentIntensity, 0.0f);
		TestEqual(TEXT("Preview duration is 1.5 seconds"), Character->EmbossPreviewSeconds, 1.5f);
		const EItemTimeline InitialTimeline = Character->GetTimeline();

		TGuardValue<uint64> RestoreFrameCounter(GFrameCounter, GFrameCounter);
		const auto Advance = [World](float Seconds)
		{
			const int32 Steps = FMath::CeilToInt(Seconds / 0.1f);
			for (int32 Step = 0; Step < Steps; ++Step)
			{
				++GFrameCounter;
				World->Tick(LEVELTICK_All, Seconds / Steps);
			}
		};

		Character->PlayEmbossPreview();
		Advance(0.5f);
		TestTrue(TEXT("Emboss rises smoothly"), Character->EmbossCurrentIntensity > 0.0f
			&& Character->EmbossCurrentIntensity < 100.0f);
		const float MidIntensity = Character->EmbossCurrentIntensity;
		Character->PlayEmbossPreview();
		TestEqual(TEXT("Repeated press keeps current intensity"),
			Character->EmbossCurrentIntensity, MidIntensity);
		Advance(1.3f);
		TestEqual(TEXT("Emboss reaches full intensity"), Character->EmbossCurrentIntensity, 100.0f);
		Advance(0.1f);
		TestEqual(TEXT("Emboss holds until 1.5 seconds after latest press"),
			Character->EmbossCurrentIntensity, 100.0f);
		Advance(0.2f);
		TestEqual(TEXT("Emboss snaps to zero after 1.5 seconds"),
			Character->EmbossCurrentIntensity, 0.0f);
		TestEqual(TEXT("Visual preview does not change timeline"), Character->GetTimeline(), InitialTimeline);

		ADrag_Item* Door = World->SpawnActor<ADrag_Item>();
		if (TestNotNull(TEXT("Door for proximity cue"), Door))
		{
			Door->SetActorLocation(FVector::ZeroVector);
			Door->bEnableEmbossProximity = true;
			Door->EmbossInnerRadius = 100.0f;
			Door->EmbossOuterRadius = 500.0f;
			Door->EmbossMaxIntensity = 60.0f;
			TestEqual(TEXT("Door center reaches authored maximum"),
				Door->GetEmbossIntensityAtLocation(FVector::ZeroVector), 60.0f);
			TestEqual(TEXT("Door falloff is continuous"),
				Door->GetEmbossIntensityAtLocation(FVector(300.0f, 0.0f, 0.0f)), 30.0f);
			TestEqual(TEXT("Door has no effect outside radius"),
				Door->GetEmbossIntensityAtLocation(FVector(500.0f, 0.0f, 0.0f)), 0.0f);
			Door->bEnableEmbossProximity = false;
			TestEqual(TEXT("Door cue is opt-in"),
				Door->GetEmbossIntensityAtLocation(FVector::ZeroVector), 0.0f);

			Door->SetActorLocation(Character->GetFirstPersonCameraComponent()->GetComponentLocation());
			Door->bEnableEmbossProximity = true;
			Advance(0.1f);
			TestTrue(TEXT("Local camera receives nearby door intensity"),
				Character->EmbossCurrentIntensity > 50.0f);
			Character->PlayEmbossPreview();
			Advance(1.6f);
			TestTrue(TEXT("Pulse expiry keeps the nearby door intensity"),
				Character->EmbossCurrentIntensity > 50.0f
				&& Character->EmbossCurrentIntensity <= 60.0f);
			Door->EmbossMaxIntensity = 250.0f;
			TestEqual(TEXT("Door intensity has no 100-unit ceiling"),
				Door->GetEmbossIntensityAtLocation(Door->GetActorLocation()), 250.0f);
			Advance(0.1f);
			TestTrue(TEXT("Dynamic camera material receives intensity above 100"),
				Character->EmbossCurrentIntensity > 200.0f);
			ADrag_Item* StrongerDoor = World->SpawnActor<ADrag_Item>();
			if (TestNotNull(TEXT("Overlapping stronger door"), StrongerDoor))
			{
				StrongerDoor->SetActorLocation(Door->GetActorLocation());
				StrongerDoor->bEnableEmbossProximity = true;
				StrongerDoor->EmbossMaxIntensity = 300.0f;
				Advance(0.1f);
				TestTrue(TEXT("Overlapping doors keep the strongest value above 100"),
					Character->EmbossCurrentIntensity > 290.0f);
				StrongerDoor->bEnableEmbossProximity = false;
			}
			Door->bEnableEmbossProximity = false;
			Advance(0.1f);
			TestEqual(TEXT("Leaving the door clears only its contribution"),
				Character->EmbossCurrentIntensity, 0.0f);
		}

		const EItemTimeline OtherTimeline = InitialTimeline == EItemTimeline::Past
			? EItemTimeline::Future : EItemTimeline::Past;
		TestTrue(TEXT("Authoritative timeline transition succeeds"),
			Character->TrySetPlayerTimelineOnAuthority(OtherTimeline));
		Advance(0.3f);
		TestTrue(TEXT("Completed timeline transition triggers owner-only pulse"),
			Character->EmbossCurrentIntensity > 0.0f);
		TestTrue(TEXT("Duplicate timeline target is accepted without another transition"),
			Character->TrySetPlayerTimelineOnAuthority(OtherTimeline));
		Advance(1.3f);
		TestEqual(TEXT("Duplicate target did not restart the pulse"),
			Character->EmbossCurrentIntensity, 0.0f);
	}

	World->EndPlay(EEndPlayReason::Quit);
	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return true;
}

#endif
