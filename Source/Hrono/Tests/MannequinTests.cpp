#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "AI/MannequinDemon.h"
#include "HronoCharacter.h"
#include "Camera/CameraComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/WorldSettings.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "HronoCollisionChannels.h"
#include "Items/Base_Item.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMannequinContractTest, "Hrono.AI.MannequinContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMannequinContractTest::RunTest(const FString& Parameters)
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
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AMannequinDemon* Demon = World->SpawnActor<AMannequinDemon>(AMannequinDemon::StaticClass(),
		FVector(0, 0, 150), FRotator::ZeroRotator, Params);
	AHronoCharacter* Viewer = nullptr;
	TestNotNull(TEXT("Mannequin exists"), Demon);
	if (Demon)
	{
		TestEqual(TEXT("Dormant by default"), Demon->State, EMannequinState::Dormant);
		TestEqual(TEXT("Mannequin is fixed to Future"), Demon->MannequinTimeline, EItemTimeline::Future);
		TestEqual(TEXT("Observation begins empty"), Demon->Observer, EMannequinObserver::None);
		TestFalse(TEXT("Legacy dormant visibility flag defaults off"), Demon->bShowDormantMeshForTesting);
		TestFalse(TEXT("No local viewer does not select physical audience"), Demon->GetMesh()->IsVisible());
		TestFalse(TEXT("Capture-only duplicate stays hidden"), Demon->MonocleVisual->IsVisible());
		TestEqual(TEXT("Approach clearance is 10 cm"), Demon->ApproachClearance, 10.0f);
		TestEqual(TEXT("Dormant capsule does not collide"),
			Demon->GetCapsuleComponent()->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
		TestEqual(TEXT("Dormant physical mesh does not collide"),
			Demon->GetMesh()->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
		TestEqual(TEXT("Dormant movement is disabled"),
			Demon->GetCharacterMovement()->MovementMode, MOVE_None);
		const FVector DormantLocation = Demon->GetActorLocation();
		{
			TGuardValue<uint64> RestoreFrameCounter(GFrameCounter, GFrameCounter);
			for (int32 Frame = 0; Frame < 30; ++Frame)
			{
				++GFrameCounter;
				World->Tick(LEVELTICK_All, 1.0f / 60.0f);
			}
		}
		TestTrue(TEXT("Dormant Mannequin does not fall through the floor"),
			Demon->GetActorLocation().Equals(DormantLocation, 0.1f));
		UFunction* StateRepFunction = Demon->FindFunctionChecked(TEXT("OnRep_State"));
		Demon->State = EMannequinState::Spawned;
		Demon->ProcessEvent(StateRepFunction, nullptr);
		TestFalse(TEXT("No local viewer still has no physical audience"), Demon->GetMesh()->IsVisible());
		TestEqual(TEXT("Active capsule collides"), Demon->GetCapsuleComponent()->GetCollisionEnabled(),
			ECollisionEnabled::QueryAndPhysics);
		Demon->State = EMannequinState::Observing;
		Demon->ProcessEvent(StateRepFunction, nullptr);
		TestFalse(TEXT("Observed mannequin has no viewer in this world"), Demon->GetMesh()->IsVisible());
		Demon->State = EMannequinState::Dormant;
		Demon->ProcessEvent(StateRepFunction, nullptr);
		TestFalse(TEXT("Deactivated mannequin has no viewer in this world"), Demon->GetMesh()->IsVisible());
		TestFalse(TEXT("No target prevents activation"), Demon->ActivateMannequin());
		TestTrue(TEXT("Rejected activation provides a reason"),
			Demon->LastActivationFailure.Contains(TEXT("eligible player")));
		TestFalse(TEXT("Null cannot become target"), Demon->ForceTarget(nullptr));
		TestFalse(TEXT("Null containment item rejected"), Demon->TryContainMannequin(nullptr));
		AActor* Untagged = World->SpawnActor<AActor>(AActor::StaticClass(),
			FVector(0, 0, 150), FRotator::ZeroRotator, Params);
		TestFalse(TEXT("Ordinary actor cannot contain dormant Mannequin"),
			Demon->TryContainMannequin(Untagged));
		TestEqual(TEXT("Failed actions preserve state"), Demon->State, EMannequinState::Dormant);
		UClass* CharacterClass = LoadClass<AHronoCharacter>(nullptr,
			TEXT("/Game/_Alex/HE_CharacterHrono1.HE_CharacterHrono1_C"));
		if (TestNotNull(TEXT("Authored player exists"), CharacterClass))
		{
			APlayerController* Controller = World->SpawnActor<APlayerController>();
			Viewer = World->SpawnActor<AHronoCharacter>(CharacterClass,
				FVector(0, 300, 150), FRotator::ZeroRotator, Params);
			if (TestNotNull(TEXT("Local controller exists"), Controller)
				&& TestNotNull(TEXT("Local viewer exists"), Viewer))
			{
				Controller->Possess(Viewer);
				TestEqual(TEXT("Transient world resolves the local viewer"),
					World->GetFirstPlayerController(), Controller);
				TestTrue(TEXT("Viewer can enter Future"),
					Viewer->TrySetPlayerTimelineOnAuthority(EItemTimeline::Future));
				Demon->ProcessEvent(Demon->FindFunctionChecked(TEXT("OnRep_Targets")), nullptr);
				TestTrue(TEXT("Future viewer sees physical mesh while dormant"),
					Demon->GetMesh()->IsVisible());
				TestFalse(TEXT("Future viewer does not see spectral mesh"),
					Demon->MonocleVisual->IsVisible());
				Demon->State = EMannequinState::Spawned;
				Demon->ProcessEvent(StateRepFunction, nullptr);
				TestTrue(TEXT("Future viewer sees active physical mesh"),
					Demon->GetMesh()->IsVisible());
				TestTrue(TEXT("Viewer can enter Past"),
					Viewer->TrySetPlayerTimelineOnAuthority(EItemTimeline::Past));
				Demon->ProcessEvent(Demon->FindFunctionChecked(TEXT("OnRep_Targets")), nullptr);
				TestFalse(TEXT("Past viewer cannot see physical mesh"),
					Demon->GetMesh()->IsVisible());
				TestFalse(TEXT("Past player cannot become physical target"), Demon->ForceTarget(Viewer));
				TestEqual(TEXT("Target rejection cannot swap the mannequin timeline"),
					Demon->MannequinTimeline, EItemTimeline::Future);
				TestFalse(TEXT("Without held monocle spectral mesh is hidden"),
					Demon->MonocleVisual->IsVisible());
				TestTrue(TEXT("Spectral mesh is capture-only"),
					Demon->MonocleVisual->bVisibleInSceneCaptureOnly != 0);
			}
		}
	}
	TestFalse(TEXT("Base item does not repel Mannequin"),
		ABase_Item::StaticClass()->GetDefaultObject<ABase_Item>()->bCanRepelMannequin);
	UClass* MonocleClass = LoadClass<ABase_Item>(nullptr,
		TEXT("/Game/_Alex/Pickable/BP_Monocle.BP_Monocle_C"));
	if (TestNotNull(TEXT("Authored monocle exists"), MonocleClass))
	{
		TestTrue(TEXT("Only authored monocle opts into rescue"),
			MonocleClass->GetDefaultObject<ABase_Item>()->bCanRepelMannequin);
		ABase_Item* Monocle = World->SpawnActor<ABase_Item>(MonocleClass,
			FVector(0, 200, 150), FRotator::ZeroRotator, Params);
		TestNotNull(TEXT("Spawned monocle exposes SceneCaptureComponent2D to server validation"),
			Monocle ? Monocle->FindComponentByClass<USceneCaptureComponent2D>() : nullptr);
		if (Viewer && Demon && Monocle)
		{
			Monocle->ItemTimeline = Viewer->GetTimeline();
			Monocle->EnableFloatingPickup();
			const FVector Eye = Viewer->GetFirstPersonCameraComponent()->GetComponentLocation();
			Monocle->SetActorLocation(Eye + FVector(0, 150, 0)
				- Monocle->ItemMesh->GetRelativeLocation());
			struct { ABase_Item* Item; } PickupArgs{Monocle};
			Viewer->ProcessEvent(Viewer->FindFunctionChecked(TEXT("ServerPickupItem")), &PickupArgs);
			if (TestEqual(TEXT("Viewer legitimately holds authored monocle"), Viewer->GetHeldItem(), Monocle))
			{
				Demon->ProcessEvent(Demon->FindFunctionChecked(TEXT("OnRep_Targets")), nullptr);
				TestFalse(TEXT("Opposite-timeline physical mesh stays hidden with monocle"),
					Demon->GetMesh()->IsVisible());
				TestTrue(TEXT("Held monocle enables capture-only spectral mesh"),
					Demon->MonocleVisual->IsVisible());
				UClass* PlayerClass = Viewer->GetClass();
				AHronoCharacter* FuturePlayer = World->SpawnActor<AHronoCharacter>(PlayerClass,
					FVector(0, -300, 150), FRotator(0, 90, 0), Params);
				APlayerController* FutureController = World->SpawnActor<APlayerController>();
				if (TestNotNull(TEXT("Future target exists"), FuturePlayer)
					&& TestNotNull(TEXT("Future target controller exists"), FutureController))
				{
					FutureController->Possess(FuturePlayer);
					FutureController->SetControlRotation(FRotator(0, 90, 0));
					FuturePlayer->TrySetPlayerTimelineOnAuthority(EItemTimeline::Future);
					TestTrue(TEXT("Future player can become target"), Demon->ForceTarget(FuturePlayer));
					TestEqual(TEXT("Past viewer becomes monocle partner"), Demon->CurrentPartner.Get(), Viewer);
					USceneCaptureComponent2D* Capture = Monocle->FindComponentByClass<USceneCaptureComponent2D>();
					const FVector CaptureFrom = Viewer->GetFirstPersonCameraComponent()->GetComponentLocation();
					Capture->SetWorldLocation(CaptureFrom);
					Capture->SetWorldRotation((Demon->GetActorLocation() + FVector(0, 0, 20)
						- CaptureFrom).Rotation());
					TestTrue(TEXT("Past monocle sees centered mannequin without obstruction"),
						Demon->IsPlayerObservingMannequinThroughMonocle(Viewer));
					Capture->SetWorldRotation(FRotator(0, 90, 0));
					TestFalse(TEXT("Monocle looking outside circular capture cannot observe"),
						Demon->IsPlayerObservingMannequinThroughMonocle(Viewer));
					const float SavedAperture = Demon->MonocleApertureRadius;
					Demon->MonocleApertureRadius = 0.1f;
					FRotator OffCentre = (Demon->GetActorLocation() + FVector(0, 0, 20)
						- CaptureFrom).Rotation();
					OffCentre.Yaw += 20.0f;
					Capture->SetWorldRotation(OffCentre);
					TestFalse(TEXT("Visible target outside central lens circle cannot freeze Mannequin"),
						Demon->IsPlayerObservingMannequinThroughMonocle(Viewer));
					Demon->MonocleApertureRadius = SavedAperture;
					Capture->SetWorldRotation((Demon->GetActorLocation() + FVector(0, 0, 20)
						- CaptureFrom).Rotation());
					AStaticMeshActor* Wall = World->SpawnActor<AStaticMeshActor>(
						FVector(0, 150, 150), FRotator::ZeroRotator, Params);
					if (TestNotNull(TEXT("Sight blocker exists"), Wall))
					{
						Wall->GetStaticMeshComponent()->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,
							TEXT("/Engine/BasicShapes/Cube.Cube")));
						Wall->SetActorScale3D(FVector(0.2f, 2.0f, 2.0f));
						Wall->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
						Wall->GetStaticMeshComponent()->SetCollisionResponseToChannel(
							COLLISION_CHANNEL_PAWN_FUTURE, ECR_Block);
						TestFalse(TEXT("Future-channel wall blocks Past monocle sight"),
							Demon->IsPlayerObservingMannequinThroughMonocle(Viewer));
					}
				}
				Viewer->ProcessEvent(Viewer->FindFunctionChecked(TEXT("ServerDropCurrentItem")), nullptr);
				Demon->ProcessEvent(Demon->FindFunctionChecked(TEXT("OnRep_Targets")), nullptr);
				TestFalse(TEXT("Dropping monocle disables spectral mesh"),
					Demon->MonocleVisual->IsVisible());
			}
		}
	}
	UClass* AuthoredMannequinClass = LoadClass<AMannequinDemon>(nullptr,
		TEXT("/Game/_Alex/AI/BP_MannequinDemon.BP_MannequinDemon_C"));
	if (TestNotNull(TEXT("Authored mannequin exists"), AuthoredMannequinClass))
	{
		AMannequinDemon* AuthoredDemon = World->SpawnActor<AMannequinDemon>(
			AuthoredMannequinClass, FVector(400, 0, 150), FRotator::ZeroRotator, Params);
		if (TestNotNull(TEXT("Authored mannequin spawns"), AuthoredDemon))
		{
			TestTrue(TEXT("Capture-only model aligns with authored physical model"),
				AuthoredDemon->MonocleVisual->GetRelativeTransform().Equals(
					AuthoredDemon->GetMesh()->GetRelativeTransform(), 0.1f));
		}
	}
	World->EndPlay(EEndPlayReason::Quit);
	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return true;
}

#endif
