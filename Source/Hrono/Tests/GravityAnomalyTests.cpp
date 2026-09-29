#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Components/GravityAnomalyComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Enviroment/Room.h"
#include "GameFramework/WorldSettings.h"
#include "HronoCharacter.h"
#include "Items/Base_Item.h"
#if WITH_EDITOR
#include "StaticMeshCompiler.h"
#endif

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGravityAnomalyRoomTest,
	"Hrono.Rooms.GravityAnomaly", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGravityAnomalyLandingTest,
	"Hrono.Rooms.GravityAnomalyLanding", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGravityAnomalyRoomTest::RunTest(const FString& Parameters)
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

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ARoom* Room = World->SpawnActor<ARoom>(ARoom::StaticClass(), FVector(0, 0, 500),
		FRotator::ZeroRotator, SpawnParams);
	UGravityAnomalyComponent* Anomaly = Room->GravityAnomaly;
	TestNotNull(TEXT("Room owns native gravity anomaly component"), Anomaly);
	Anomaly->MinEventInterval = 0.1f;
	Anomaly->MaxEventInterval = 0.1f;
	Anomaly->EventChance = 1.0f;
	Anomaly->FallBeforePause = 0.2f;
	Anomaly->MaxFallBeforePause = 0.2f;
	Anomaly->PauseDuration = 0.1f;
	Anomaly->SideImpulseSpeed = 180.0f;
	Anomaly->DownwardImpulseSpeed = 60.0f;
	Anomaly->StrongEventChance = 0.0f;
	Anomaly->MinAffectedObjects = 1;
	Anomaly->MaxAffectedObjects = 1;

	auto SpawnItem = [&](const FVector& Location, bool bTagged)
	{
		ABase_Item* Item = World->SpawnActorDeferred<ABase_Item>(ABase_Item::StaticClass(),
			FTransform(Location), nullptr, nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		Item->ItemMesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,
			TEXT("/Engine/BasicShapes/Cube.Cube")));
		if (bTagged)
		{
			Item->Tags.Add(TEXT("GravityAnomaly"));
		}
		Item->FinishSpawning(FTransform(Location));
		return Item;
	};
	ABase_Item* Tagged = SpawnItem(FVector(0, 0, 500), true);
	ABase_Item* Untagged = SpawnItem(FVector(220, 0, 500), false);
	ABase_Item* Held = SpawnItem(FVector(-220, 0, 500), true);
	Held->bIsPickedUp = true;
	TestTrue(TEXT("Tagged loose prop passes item state policy"), Tagged->CanEnterGravityAnomaly());
	TestTrue(TEXT("Room volume overlaps tagged prop"), Room->RoomVolume->IsOverlappingActor(Tagged));
	TestFalse(TEXT("Held item is not eligible"), Held->CanEnterGravityAnomaly());
	TestFalse(TEXT("Ordinary room is inactive"), Anomaly->IsAnomalyActive());

	Room->SetCursed(true);
	{
		TGuardValue<uint64> RestoreFrameCounter(GFrameCounter, GFrameCounter);
		for (int32 Frame = 0; Frame < 15 && !Anomaly->IsAnomalyActive(); ++Frame)
		{
			++GFrameCounter;
			World->Tick(LEVELTICK_All, 1.0f / 60.0f);
		}
	}
	TestTrue(TEXT("Cursed room starts an event for tagged loose prop"), Anomaly->IsAnomalyActive());
	TestTrue(TEXT("Tagged prop enters server physics"), Tagged->ItemMesh->IsSimulatingPhysics());
	TestTrue(TEXT("Tagged prop initially falls under gravity"), Tagged->ItemMesh->IsGravityEnabled());
	TestFalse(TEXT("Untagged prop remains unaffected"), Untagged->ItemMesh->IsSimulatingPhysics());
	TestFalse(TEXT("Held prop remains unaffected"), Held->ItemMesh->IsSimulatingPhysics());
	const FVector StartLocation = Tagged->ItemMesh->GetComponentLocation();
	{
		TGuardValue<uint64> RestoreFrameCounter(GFrameCounter, GFrameCounter);
		for (int32 Frame = 0; Frame < 6; ++Frame)
		{
			++GFrameCounter;
			World->Tick(LEVELTICK_All, 1.0f / 60.0f);
		}
	}
	TestTrue(TEXT("Prop drops during first 0.2 seconds"),
		Tagged->ItemMesh->GetComponentLocation().Z < StartLocation.Z - 1.0f);
	{
		TGuardValue<uint64> RestoreFrameCounter(GFrameCounter, GFrameCounter);
		for (int32 Frame = 0; Frame < 9; ++Frame)
		{
			++GFrameCounter;
			World->Tick(LEVELTICK_All, 1.0f / 60.0f);
		}
	}
	TestTrue(TEXT("Prop is still active during the freeze"), Anomaly->IsAnomalyActive());
	TestFalse(TEXT("Gravity is disabled during 0.1 second freeze"),
		Tagged->ItemMesh->IsGravityEnabled());
	TestTrue(TEXT("Freeze clears linear velocity"),
		Tagged->ItemMesh->GetPhysicsLinearVelocity().IsNearlyZero(1.0f));
	{
		TGuardValue<uint64> RestoreFrameCounter(GFrameCounter, GFrameCounter);
		for (int32 Frame = 0; Frame < 5; ++Frame)
		{
			++GFrameCounter;
			World->Tick(LEVELTICK_All, 1.0f / 60.0f);
		}
	}
	TestFalse(TEXT("Impulse completes the anomaly"), Anomaly->IsAnomalyActive());
	TestTrue(TEXT("Sideways impulse survives release"),
		Tagged->ItemMesh->GetPhysicsLinearVelocity().Size2D() > 50.0f);
	TestTrue(TEXT("Prop resumes downward motion"),
		Tagged->ItemMesh->GetPhysicsLinearVelocity().Z < 0.0f);

	Room->SetCursed(false);
	TestTrue(TEXT("Release restores ordinary gravity"), Tagged->ItemMesh->IsGravityEnabled());
	TestFalse(TEXT("Inactive component does not tick"), Anomaly->IsComponentTickEnabled());

	// A hand drop must react before the prop has time to hit the floor. BP_Item2
	// is an explicitly opted-in mug, despite its legacy TableRitual category.
	Anomaly->MinEventInterval = 100.0f;
	Anomaly->MaxEventInterval = 100.0f;
	Room->SetCursed(true);
	UClass* MugClass = LoadClass<ABase_Item>(nullptr,
		TEXT("/Game/_Alex/Pickable/BP_Item2.BP_Item2_C"));
	if (TestNotNull(TEXT("BP_Item2 class exists"), MugClass))
	{
		TestTrue(TEXT("BP_Item2 explicitly opts in"),
			MugClass->GetDefaultObject<ABase_Item>()->ActorHasTag(TEXT("GravityAnomaly")));
		UClass* CharacterClass = LoadClass<AHronoCharacter>(nullptr,
			TEXT("/Game/_Alex/HE_CharacterHrono1.HE_CharacterHrono1_C"));
		AHronoCharacter* Player = CharacterClass
			? World->SpawnActor<AHronoCharacter>(CharacterClass, FVector(0, 0, 500),
				FRotator::ZeroRotator, SpawnParams) : nullptr;
		ABase_Item* Mug = World->SpawnActor<ABase_Item>(MugClass,
			FVector(0, 0, 500), FRotator::ZeroRotator, SpawnParams);
		if (TestNotNull(TEXT("Test player"), Player) && TestNotNull(TEXT("BP_Item2 mug"), Mug))
		{
			// A freshly loaded project mesh may still be compiling in commandlet;
			// that temporarily suppresses its physics state, unlike a ready game asset.
#if WITH_EDITOR
			FStaticMeshCompilingManager::Get().FinishCompilation({Mug->ItemMesh->GetStaticMesh()});
#endif
			Mug->ItemTimeline = EItemTimeline::Both;
			if (!TestTrue(TEXT("Actual BP_Item2 pickup succeeds"), Mug->TryPickUp(Player)))
			{
				Room->SetCursed(false);
			}
			else
			{
				Mug->Drop();
				TestFalse(TEXT("Drop leaves held state"), Mug->bIsPickedUp);
				TestTrue(TEXT("Dropped mug passes gravity item policy"),
					Mug->CanEnterGravityAnomaly());
				TestTrue(TEXT("Server drop starts anomaly immediately"), Anomaly->IsAnomalyActive());
				TestTrue(TEXT("Dropped mug remains simulated"), Mug->ItemMesh->IsSimulatingPhysics());
				TestTrue(TEXT("Dropped mug falls before freezing"), Mug->ItemMesh->IsGravityEnabled());
				{
					TGuardValue<uint64> RestoreFrameCounter(GFrameCounter, GFrameCounter);
					for (int32 Frame = 0; Frame < 14; ++Frame)
					{
						++GFrameCounter;
						World->Tick(LEVELTICK_All, 1.0f / 60.0f);
					}
				}
				TestFalse(TEXT("Dropped mug freezes after 0.2 seconds"),
					Mug->ItemMesh->IsGravityEnabled());
				Room->SetCursed(false);
				TestFalse(TEXT("Uncursing stops the active event"), Anomaly->IsAnomalyActive());
				TestTrue(TEXT("Uncursing restores dropped mug gravity"), Mug->ItemMesh->IsGravityEnabled());
				if (TestTrue(TEXT("Mug can be picked up again"), Mug->TryPickUp(Player)))
				{
					Mug->Drop();
					TestFalse(TEXT("Drop in ordinary room does not start anomaly"),
						Anomaly->IsAnomalyActive());
					TestTrue(TEXT("Ordinary drop retains gravity"), Mug->ItemMesh->IsGravityEnabled());
				}
			}
		}
	}
	World->EndPlay(EEndPlayReason::Quit);
	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return true;
}

bool FGravityAnomalyLandingTest::RunTest(const FString& Parameters)
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

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ARoom* Room = World->SpawnActor<ARoom>(ARoom::StaticClass(), FVector(0, 0, 500),
		FRotator::ZeroRotator, SpawnParams);
	Room->RoomVolume->SetBoxExtent(FVector(500, 500, 500));
	UGravityAnomalyComponent* Anomaly = Room->GravityAnomaly;
	Anomaly->EventChance = 1.0f;
	Anomaly->MinEventInterval = 100.0f;
	Anomaly->MaxEventInterval = 100.0f;
	Anomaly->FallBeforePause = 1.0f;
	Anomaly->MaxFallBeforePause = 1.0f;
	Room->SetCursed(true);

	UClass* MugClass = LoadClass<ABase_Item>(nullptr,
		TEXT("/Game/_Alex/Pickable/BP_Item2.BP_Item2_C"));
	UClass* CharacterClass = LoadClass<AHronoCharacter>(nullptr,
		TEXT("/Game/_Alex/HE_CharacterHrono1.HE_CharacterHrono1_C"));
	AHronoCharacter* Player = CharacterClass
		? World->SpawnActor<AHronoCharacter>(CharacterClass, FVector(0, 0, 550),
			FRotator::ZeroRotator, SpawnParams) : nullptr;
	ABase_Item* Mug = MugClass
		? World->SpawnActor<ABase_Item>(MugClass, FVector(0, 0, 550),
			FRotator::ZeroRotator, SpawnParams) : nullptr;
	if (TestNotNull(TEXT("Landing test player"), Player)
		&& TestNotNull(TEXT("Landing test BP_Item2"), Mug))
	{
#if WITH_EDITOR
		FStaticMeshCompilingManager::Get().FinishCompilation({Mug->ItemMesh->GetStaticMesh()});
#endif
		Mug->ItemTimeline = EItemTimeline::Both;
		if (TestTrue(TEXT("Pickup before landing test"), Mug->TryPickUp(Player)))
		{
			Mug->Drop();
			TestTrue(TEXT("First throw starts an anomaly attempt"), Anomaly->IsAnomalyActive());
			const FVector DropLocation = Mug->ItemMesh->GetComponentLocation();
			AStaticMeshActor* Floor = World->SpawnActor<AStaticMeshActor>(
				AStaticMeshActor::StaticClass(), DropLocation - FVector(0, 0, 125),
				FRotator::ZeroRotator, SpawnParams);
			Floor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
			Floor->GetStaticMeshComponent()->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,
				TEXT("/Engine/BasicShapes/Cube.Cube")));
			Floor->GetStaticMeshComponent()->SetWorldScale3D(FVector(5, 5, 1));
			Floor->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			uint64 SimulatedFrame = GFrameCounter;
			auto TickFrames = [&World, &SimulatedFrame](int32 Count)
			{
				TGuardValue<uint64> RestoreFrameCounter(GFrameCounter, GFrameCounter);
				for (int32 Frame = 0; Frame < Count; ++Frame)
				{
					GFrameCounter = ++SimulatedFrame;
					World->Tick(LEVELTICK_All, 1.0f / 60.0f);
				}
			};
			TickFrames(90);
			AddInfo(FString::Printf(TEXT("Landing pose: start=%s end=%s velocity=%s"),
				*DropLocation.ToCompactString(),
				*Mug->ItemMesh->GetComponentLocation().ToCompactString(),
				*Mug->ItemMesh->GetPhysicsLinearVelocity().ToCompactString()));
			TestFalse(TEXT("Landing cancels the pending impulse"), Anomaly->IsAnomalyActive());
			TestTrue(TEXT("Landed prop keeps ordinary gravity"), Mug->ItemMesh->IsGravityEnabled());
			TestTrue(TEXT("Prop reached the floor before its one-second trigger"),
				Mug->ItemMesh->GetComponentLocation().Z < DropLocation.Z - 35.0f
				&& Mug->ItemMesh->GetComponentLocation().Z > DropLocation.Z - 100.0f);
			TestTrue(TEXT("Landing did not add a sideways impulse"),
				Mug->ItemMesh->GetPhysicsLinearVelocity().Size2D() < 30.0f);

			Anomaly->SetEnabled(false);
			Anomaly->MinEventInterval = 0.1f;
			Anomaly->MaxEventInterval = 0.1f;
			Anomaly->SetEnabled(true);
			TickFrames(20);
			TestFalse(TEXT("Periodic timer cannot re-arm the grounded prop"),
				Anomaly->IsAnomalyActive());

			Anomaly->SetEnabled(false);
			Anomaly->MinEventInterval = 100.0f;
			Anomaly->MaxEventInterval = 100.0f;
			Anomaly->FallBeforePause = 0.2f;
			Anomaly->MaxFallBeforePause = 0.2f;
			Anomaly->SetEnabled(true);
			if (TestTrue(TEXT("Grounded mug can be picked up again"), Mug->TryPickUp(Player)))
			{
				Mug->Drop();
				TestTrue(TEXT("New throw starts a new anomaly attempt"), Anomaly->IsAnomalyActive());
				TickFrames(15);
				TestFalse(TEXT("Second throw freezes while still airborne"),
					Mug->ItemMesh->IsGravityEnabled());
				TickFrames(4);
				TestTrue(TEXT("Second throw gets a sideways impulse"),
					Mug->ItemMesh->GetPhysicsLinearVelocity().Size2D() > 50.0f);

				// Without a floor, the sampled pause must begin inside the 0.2-1.0 s
				// window rather than always at the old fixed 0.2 s point.
				Floor->SetActorEnableCollision(false);
				Floor->Destroy();
				TickFrames(1);
				Anomaly->FallBeforePause = 0.2f;
				Anomaly->MaxFallBeforePause = 1.0f;
				if (TestTrue(TEXT("Mug can be thrown for a third attempt"), Mug->TryPickUp(Player)))
				{
					Mug->Drop();
					TickFrames(11);
					TestTrue(TEXT("Random delay never freezes before 0.2 seconds"),
						Anomaly->IsAnomalyActive() && Mug->ItemMesh->IsGravityEnabled());
					int32 FallFrames = 11;
					while (FallFrames < 65 && Anomaly->IsAnomalyActive()
						&& Mug->ItemMesh->IsGravityEnabled())
					{
						TickFrames(1);
						++FallFrames;
					}
					AddInfo(FString::Printf(TEXT("Random delay: frames=%d active=%d gravity=%d pose=%s velocity=%s"),
						FallFrames, Anomaly->IsAnomalyActive() ? 1 : 0,
						Mug->ItemMesh->IsGravityEnabled() ? 1 : 0,
						*Mug->ItemMesh->GetComponentLocation().ToCompactString(),
						*Mug->ItemMesh->GetPhysicsLinearVelocity().ToCompactString()));
					TestTrue(TEXT("Random delay freezes a falling item by one second"),
						Anomaly->IsAnomalyActive() && !Mug->ItemMesh->IsGravityEnabled()
						&& FallFrames >= 12 && FallFrames <= 62);
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
