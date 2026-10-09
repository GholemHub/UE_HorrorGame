#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "AI/MannequinDemon.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/WorldSettings.h"
#include "HronoCharacter.h"
#include "Items/Base_Item.h"
#include "ScareDirector.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMannequinFearTest, "Hrono.AI.MannequinFear",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMannequinFearTest::RunTest(const FString& Parameters)
{
	const auto Settings = UWorld::InitializationValues().AllowAudioPlayback(false)
		.CreatePhysicsScene(true).CreateNavigation(false).CreateAISystem(false)
		.ShouldSimulatePhysics(false).SetTransactional(false);
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr, true,
		ERHIFeatureLevel::Num, &Settings);
	GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
	AScareDirector* Director = World->SpawnActor<AScareDirector>();
	Director->bChooseCursedRoomOnBeginPlay = false;
	Director->PassiveThreatPerInterval = 0.0f;
	Director->BabajClass = AActor::StaticClass(); // Harmless fixture; verifies actual SpawnActor timing.
	Director->BabajSpawnPoints.Add(World->SpawnActor<ABase_Item>());
	Director->HuntDurationMin = Director->HuntDurationMax = 120.0f;
	Director->EndingStateDuration = 0.0f;
	Director->MinimumHuntCooldown = 120.0f;
	World->InitializeActorsForPlay(FURL());
	World->BeginPlay();
	World->GetWorldSettings()->NotifyBeginPlay();
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AMannequinDemon* Demon = World->SpawnActor<AMannequinDemon>(AMannequinDemon::StaticClass(),
		FVector(0, 0, 150), FRotator::ZeroRotator, Params);
	UClass* AuthoredClass = LoadClass<AMannequinDemon>(nullptr,
		TEXT("/Game/_Alex/AI/Mannequin/BP_MannequinDemon.BP_MannequinDemon_C"));
	if (TestNotNull(TEXT("Placed mannequin Blueprint exists"), AuthoredClass))
	{
		USkeletalMeshComponent* AuthoredMesh = AuthoredClass->GetDefaultObject<AMannequinDemon>()->GetMesh();
		// Authored BP currently mixes the original body with retargeted animations.
		// Use the matching test mesh without changing either authored asset.
		Demon->GetMesh()->SetSkeletalMeshAsset(LoadObject<USkeletalMesh>(nullptr,
			TEXT("/Game/_Alex/AI/Mannequin/ь2/SKM_KillerDollBase_Manny_IdleVerified.SKM_KillerDollBase_Manny_IdleVerified")));
		Demon->GetMesh()->SetAnimInstanceClass(AuthoredMesh->GetAnimClass());
		Demon->FearAnimation = Cast<UAnimSequence>(AuthoredMesh->AnimationData.AnimToPlay);
	}
	Demon->FearPlayRate = 0.1f;
	Demon->BabaiRecoveryDelay = 0.0f;
	Demon->State = EMannequinState::Spawned;
	Demon->ProcessEvent(Demon->FindFunctionChecked(TEXT("OnRep_State")), nullptr);
	UClass* PlayerClass = LoadClass<AHronoCharacter>(nullptr,
		TEXT("/Game/_Alex/HE_CharacterHrono1.HE_CharacterHrono1_C"));
	AHronoCharacter* Player = World->SpawnActor<AHronoCharacter>(PlayerClass,
		FVector(0, 400, 150), FRotator::ZeroRotator, Params);
	APlayerController* PC = World->SpawnActor<APlayerController>();
	PC->Possess(Player);
	Player->GetCharacterMovement()->DisableMovement();
	Player->TrySetPlayerTimelineOnAuthority(EItemTimeline::Future);
	Demon->ForceTarget(Player);
	const auto LookAt = [Demon](APlayerController* Controller, AHronoCharacter* Viewer, bool bLook)
	{
		FRotator Rotation = (Demon->GetActorLocation() + FVector(0, 0, 20)
			- Viewer->GetFirstPersonCameraComponent()->GetComponentLocation()).Rotation();
		if (!bLook) Rotation.Yaw += 180.0f;
		Controller->SetControlRotation(Rotation);
	};
	const auto Advance = [World](float Seconds)
	{
		TGuardValue<uint64> RestoreFrameCounter(GFrameCounter, GFrameCounter);
		for (int32 Frame = 0; Frame < FMath::RoundToInt(Seconds * 60.0f); ++Frame)
		{
			++GFrameCounter;
			World->Tick(LEVELTICK_All, 1.0f / 60.0f);
		}
	};
	LookAt(PC, Player, true);
	TestTrue(TEXT("Real hunt accepted"), Director->RequestTriggeredHunt(EItemTimeline::Both, false));
	TestEqual(TEXT("Real hunt first enters anticipation"), Director->GetHuntState(), EGhostHuntState::Anticipation);
	TestEqual(TEXT("Fear begins immediately before spawn"), Demon->State, EMannequinState::SubmissiveToBabai);
	TestFalse(TEXT("Watching cannot pause fear entry"), Demon->FearPlayback.bPaused);
	TestFalse(TEXT("Fear evaluates immediately while watched"), Demon->GetMesh()->bPauseAnims != 0);
	TestEqual(TEXT("Fear capsule is passable"), Demon->GetCapsuleComponent()->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
	TestEqual(TEXT("Fear body is passable"), Demon->GetMesh()->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
	TestFalse(TEXT("Retarget cannot unlock fear"), Demon->ForceTarget(Player));
	TestFalse(TEXT("Duplicate hunt cannot reset the countdown"), Director->RequestTriggeredHunt());
	const FVector FearLocation = Demon->GetActorLocation();
	Advance(4.8f);
	TestNull(TEXT("No Babai before five seconds"), Director->HuntDemon.Get());
	TestTrue(TEXT("Watched animation advances throughout anticipation"), Demon->GetFearAnimationElapsed() > 4.7f);
	if (UAnimSingleNodeInstance* Single = Demon->GetMesh()->GetSingleNodeInstance())
		TestTrue(TEXT("Long intro fits into five seconds despite slow configured rate"),
			Single->GetCurrentTime() > Demon->FearAnimation->GetPlayLength() * 0.94f);
	Advance(0.4f);
	TestNotNull(TEXT("Babai spawns after five seconds while watched"), Director->HuntDemon.Get());
	TestTrue(TEXT("Fear cannot translate the capsule"), Demon->GetActorLocation().Equals(FearLocation, 0.01f));
	TestEqual(TEXT("Movement remains disabled"), Demon->GetCharacterMovement()->MovementMode, MOVE_None);
	LookAt(PC, Player, false);
	Advance(0.8f);
	TestFalse(TEXT("Looking away does not change playback"), Demon->FearPlayback.bPaused);
	TestTrue(TEXT("Fear clock continues"), Demon->GetFearAnimationElapsed() > 0.2f);
	if (TestNotNull(TEXT("Fixture sequence assigned"), Demon->FearAnimation.Get()))
	{
		TestNotNull(TEXT("Native fear player takes over from AnimBP"), Demon->GetMesh()->GetSingleNodeInstance());
	}
	LookAt(PC, Player, true);
	Advance(0.2f);
	const float BeforeGaze = Demon->GetFearAnimationElapsed();
	Advance(0.8f);
	TestTrue(TEXT("Renewed gaze cannot freeze the clock"), Demon->GetFearAnimationElapsed() > BeforeGaze + 0.7f);
	TestTrue(TEXT("Manifestation locks the final fear pose"), Demon->FearPlayback.bHoldingPose);
	if (UAnimSingleNodeInstance* Single = Demon->GetMesh()->GetSingleNodeInstance())
		TestTrue(TEXT("Pose stays at the final frame while watched"), FMath::IsNearlyEqual(
			Single->GetCurrentTime(), Demon->FearAnimation->GetPlayLength(), 0.01f));

	// Initial snapshot/relevancy restoration must seek to the current frame, not replay the startle.
	AMannequinDemon* ObserverCopy = World->SpawnActor<AMannequinDemon>();
	ObserverCopy->GetMesh()->SetSkeletalMeshAsset(Demon->GetMesh()->GetSkeletalMeshAsset());
	ObserverCopy->FearAnimation = Demon->FearAnimation;
	ObserverCopy->FearPlayRate = Demon->FearPlayRate;
	ObserverCopy->FearPlayback = Demon->FearPlayback;
	ObserverCopy->ProcessEvent(ObserverCopy->FindFunctionChecked(TEXT("OnRep_FearPlayback")), nullptr);
	TestTrue(TEXT("Late snapshot restores fixed world position even with disabled movement"),
		ObserverCopy->GetActorLocation().Equals(Demon->FearPlayback.Location, 0.01f));
	if (TestNotNull(TEXT("Late snapshot creates a pose player"), ObserverCopy->GetMesh()->GetSingleNodeInstance()))
	{
		TestTrue(TEXT("Late snapshot restores the frozen frame"), FMath::IsNearlyEqual(
			ObserverCopy->GetMesh()->GetSingleNodeInstance()->GetCurrentTime(), Demon->FearAnimation->GetPlayLength(), 0.01f));
		TestFalse(TEXT("Snapshot is sampled, never autonomous one-shot playback"),
			ObserverCopy->GetMesh()->GetSingleNodeInstance()->IsPlaying());
	}
	TestEqual(TEXT("Late snapshot disables capsule before state notify"), ObserverCopy->GetCapsuleComponent()->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
	TestEqual(TEXT("Late snapshot disables mesh before state notify"), ObserverCopy->GetMesh()->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
	ObserverCopy->Destroy();

	AHronoCharacter* Other = World->SpawnActor<AHronoCharacter>(PlayerClass,
		FVector(400, 0, 150), FRotator::ZeroRotator, Params);
	APlayerController* OtherPC = World->SpawnActor<APlayerController>();
	OtherPC->Possess(Other);
	Other->GetCharacterMovement()->DisableMovement();
	Other->TrySetPlayerTimelineOnAuthority(EItemTimeline::Future);
	LookAt(PC, Player, false);
	LookAt(OtherPC, Other, true);
	Advance(0.6f);
	TestFalse(TEXT("Second player cannot pause fear either"), Demon->FearPlayback.bPaused);
	Director->EndHunt();
	Advance(0.4f);
	TestFalse(TEXT("Fear exits after danger even while watched"), Demon->FearPlayback.bActive);
	TestEqual(TEXT("Capsule collision restored on recovery"), Demon->GetCapsuleComponent()->GetCollisionEnabled(), ECollisionEnabled::QueryAndPhysics);
	TestEqual(TEXT("Locomotion AnimBP restored"), Demon->GetMesh()->GetAnimationMode(), EAnimationMode::AnimationBlueprint);
	TestFalse(TEXT("Restored mesh is not permanently paused"), Demon->GetMesh()->bPauseAnims != 0);

	if (Director->HuntDemon) Director->HuntDemon->Destroy();
	Director->SetHuntDemon(nullptr);
	TestTrue(TEXT("Next real hunt accepted"), Director->RequestTriggeredHunt(EItemTimeline::Both, false, true));
	Advance(1.0f);
	Director->EndHunt();
	Advance(5.5f);
	TestNull(TEXT("Cancelling anticipation cancels the pending spawn"), Director->HuntDemon.Get());
	TestFalse(TEXT("Cancelled danger releases fear"), Demon->FearPlayback.bActive);

	Director->RequestTriggeredHunt(EItemTimeline::Both, false, true);
	Demon->DeactivateMannequin();
	TestFalse(TEXT("Deactivation clears replicated fear"), Demon->FearPlayback.bActive);
	TestFalse(TEXT("Deactivation stops presentation tick"), Demon->IsActorTickEnabled());
	Director->EndHunt();
	World->EndPlay(EEndPlayReason::Quit);
	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return true;
}

#endif
