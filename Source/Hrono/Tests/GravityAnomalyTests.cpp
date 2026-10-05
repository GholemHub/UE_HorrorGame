#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Components/GravityAnomalyComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Enviroment/Room.h"
#include "GameFramework/PlayerController.h"
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

namespace
{
	UWorld* MakePhysicsWorld()
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
		return World;
	}

	void TickPhysics(UWorld* World, int32 Frames, uint64& SimulatedFrame)
	{
		TGuardValue<uint64> RestoreFrameCounter(GFrameCounter, GFrameCounter);
		for (int32 Frame = 0; Frame < Frames; ++Frame)
		{
			GFrameCounter = ++SimulatedFrame;
			World->Tick(LEVELTICK_All, 1.0f / 60.0f);
		}
	}

	void DestroyPhysicsWorld(UWorld* World)
	{
		World->EndPlay(EEndPlayReason::Quit);
		World->DestroyWorld(false);
		GEngine->DestroyWorldContext(World);
	}
}

bool FGravityAnomalyRoomTest::RunTest(const FString& Parameters)
{
	UWorld* World = MakePhysicsWorld();
	uint64 SimulatedFrame = GFrameCounter;
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ARoom* Room = World->SpawnActor<ARoom>(ARoom::StaticClass(), FVector(0, 0, 500),
		FRotator::ZeroRotator, Params);
	UGravityAnomalyComponent* Anomaly = Room->GravityAnomaly;
	TestNotNull(TEXT("Room owns gravity anomaly"), Anomaly);
	Anomaly->MinEventInterval = 0.1f;
	Anomaly->MaxEventInterval = 0.1f;
	Anomaly->EventChance = 1.0f;
	Anomaly->SlowEventChance = 1.0f;
	Anomaly->MinSlowGravity = 8.5f;
	Anomaly->MaxSlowGravity = 8.5f;
	Anomaly->bEnableUnseenDrop = false;
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
		if (bTagged) Item->Tags.Add(TEXT("GravityAnomaly"));
		Item->FinishSpawning(FTransform(Location));
		return Item;
	};
	ABase_Item* Tagged = SpawnItem(FVector(0, 0, 500), true);
	ABase_Item* Untagged = SpawnItem(FVector(220, 0, 500), false);
	ABase_Item* Held = SpawnItem(FVector(-220, 0, 500), true);
	Held->bIsPickedUp = true;
	TestTrue(TEXT("Tagged loose item is eligible"), Tagged->CanEnterGravityAnomaly());
	TestTrue(TEXT("Tagged item overlaps room"), Room->RoomVolume->IsOverlappingActor(Tagged));
	TestFalse(TEXT("Held item is ineligible"), Held->CanEnterGravityAnomaly());
	TestFalse(TEXT("Ordinary room is inactive"), Anomaly->IsAnomalyActive());
	Room->SetCursed(true);
	for (int32 Frame = 0; Frame < 15 && !Anomaly->IsAnomalyActive(); ++Frame)
		TickPhysics(World, 1, SimulatedFrame);
	TestTrue(TEXT("Cursed room starts a fall for tagged item"), Anomaly->IsAnomalyActive());
	TestTrue(TEXT("Tagged item simulates physics"), Tagged->ItemMesh->IsSimulatingPhysics());
	TestTrue(TEXT("Native gravity remains enabled"), Tagged->ItemMesh->IsGravityEnabled());
	TestEqual(TEXT("Selected slow acceleration is stable"),
		Anomaly->GetActiveGravityForItem(Tagged), 8.5f);
	TestFalse(TEXT("Untagged item is unaffected"), Untagged->ItemMesh->IsSimulatingPhysics());
	TestFalse(TEXT("Held item is unaffected"), Held->ItemMesh->IsSimulatingPhysics());
	Tagged->ItemMesh->SetLinearDamping(0.0f);
	Tagged->ItemMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
	const float StartZ = Tagged->ItemMesh->GetComponentLocation().Z;
	TickPhysics(World, 12, SimulatedFrame);
	const float VerticalSpeed = Tagged->ItemMesh->GetPhysicsLinearVelocity().Z;
	const float MeasuredGravity = -VerticalSpeed / 20.0f; // 12 frames / 60 Hz, cm to m.
	AddInfo(FString::Printf(TEXT("Slow gravity: chosen=%.2f measured=%.2f m/s^2 speed=%.1f cm/s"),
		Anomaly->GetActiveGravityForItem(Tagged), MeasuredGravity, VerticalSpeed));
	TestTrue(TEXT("Item continuously descends without a pause"),
		Tagged->ItemMesh->GetComponentLocation().Z < StartZ - 1.0f && VerticalSpeed < -100.0f);
	TestTrue(TEXT("Measured acceleration follows the selected slow value"),
		FMath::Abs(MeasuredGravity - 8.5f) < 1.0f);
	TestTrue(TEXT("No sideways impulse is applied"),
		Tagged->ItemMesh->GetPhysicsLinearVelocity().Size2D() < 10.0f);
	TestTrue(TEXT("Gravity stays enabled throughout the anomaly"),
		Tagged->ItemMesh->IsGravityEnabled());
	Room->SetCursed(false);
	TestFalse(TEXT("Uncursing clears active fall"), Anomaly->IsAnomalyActive());
	TestTrue(TEXT("Cleanup retains native gravity"), Tagged->ItemMesh->IsGravityEnabled());
	TestTrue(TEXT("Cleanup preserves momentum"), Tagged->ItemMesh->GetPhysicsLinearVelocity().Z < -100.0f);
	TestFalse(TEXT("Inactive component does not tick"), Anomaly->IsComponentTickEnabled());

	// Actual authored BP_Item2 must still react immediately to a server drop.
	Anomaly->MinEventInterval = 100.0f;
	Anomaly->MaxEventInterval = 100.0f;
	Room->SetCursed(true);
	UClass* MugClass = LoadClass<ABase_Item>(nullptr,
		TEXT("/Game/_Alex/Pickable/BP_Item2.BP_Item2_C"));
	UClass* CharacterClass = LoadClass<AHronoCharacter>(nullptr,
		TEXT("/Game/_Alex/HE_CharacterHrono1.HE_CharacterHrono1_C"));
	if (TestNotNull(TEXT("BP_Item2 exists"), MugClass)
		&& TestNotNull(TEXT("Player Blueprint exists"), CharacterClass))
	{
		TestTrue(TEXT("BP_Item2 opts into gravity anomaly"),
			MugClass->GetDefaultObject<ABase_Item>()->ActorHasTag(TEXT("GravityAnomaly")));
		AHronoCharacter* Player = World->SpawnActor<AHronoCharacter>(CharacterClass,
			FVector(0, 0, 500), FRotator::ZeroRotator, Params);
		ABase_Item* Mug = World->SpawnActor<ABase_Item>(MugClass,
			FVector(0, 0, 500), FRotator::ZeroRotator, Params);
		if (TestNotNull(TEXT("Player"), Player) && TestNotNull(TEXT("Mug"), Mug))
		{
#if WITH_EDITOR
			FStaticMeshCompilingManager::Get().FinishCompilation({Mug->ItemMesh->GetStaticMesh()});
#endif
			Mug->ItemTimeline = EItemTimeline::Both;
			if (TestTrue(TEXT("Pickup succeeds"), Mug->TryPickUp(Player)))
			{
				Mug->Drop();
				TestTrue(TEXT("Drop starts anomaly immediately"), Anomaly->IsAnomalyActive());
				TestTrue(TEXT("Dropped mug keeps gravity"), Mug->ItemMesh->IsGravityEnabled());
				TestTrue(TEXT("Dropped mug has selected gravity"),
					Anomaly->GetActiveGravityForItem(Mug) >= 8.5f);
				Room->SetCursed(false);
				TestFalse(TEXT("Uncursing clears dropped mug"), Anomaly->IsAnomalyActive());
				if (TestTrue(TEXT("Mug can be picked up again"), Mug->TryPickUp(Player)))
				{
					Mug->Drop();
					TestFalse(TEXT("Ordinary room drop has no anomaly"), Anomaly->IsAnomalyActive());
				}
			}
		}
	}
	DestroyPhysicsWorld(World);
	return true;
}

bool FGravityAnomalyLandingTest::RunTest(const FString& Parameters)
{
	UWorld* World = MakePhysicsWorld();
	uint64 SimulatedFrame = GFrameCounter;
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ARoom* Room = World->SpawnActor<ARoom>(ARoom::StaticClass(), FVector(0, 0, 500),
		FRotator::ZeroRotator, Params);
	Room->RoomVolume->SetBoxExtent(FVector(500, 500, 500));
	UGravityAnomalyComponent* Anomaly = Room->GravityAnomaly;
	Anomaly->EventChance = 1.0f;
	Anomaly->MinEventInterval = 100.0f;
	Anomaly->MaxEventInterval = 100.0f;
	Anomaly->SlowEventChance = 0.0f;
	Anomaly->MinFastGravity = 10.5f;
	Anomaly->MaxFastGravity = 10.5f;
	Room->SetCursed(true);
	UClass* MugClass = LoadClass<ABase_Item>(nullptr,
		TEXT("/Game/_Alex/Pickable/BP_Item2.BP_Item2_C"));
	UClass* CharacterClass = LoadClass<AHronoCharacter>(nullptr,
		TEXT("/Game/_Alex/HE_CharacterHrono1.HE_CharacterHrono1_C"));
	AHronoCharacter* Player = CharacterClass ? World->SpawnActor<AHronoCharacter>(CharacterClass,
		FVector(0, 0, 550), FRotator::ZeroRotator, Params) : nullptr;
	ABase_Item* Mug = MugClass ? World->SpawnActor<ABase_Item>(MugClass,
		FVector(0, 0, 550), FRotator::ZeroRotator, Params) : nullptr;
	if (TestNotNull(TEXT("Landing player"), Player) && TestNotNull(TEXT("Landing BP_Item2"), Mug))
	{
#if WITH_EDITOR
		FStaticMeshCompilingManager::Get().FinishCompilation({Mug->ItemMesh->GetStaticMesh()});
#endif
		Mug->ItemTimeline = EItemTimeline::Both;
		if (TestTrue(TEXT("Pickup before landing"), Mug->TryPickUp(Player)))
		{
			Mug->Drop();
			TestTrue(TEXT("First throw starts anomaly"), Anomaly->IsAnomalyActive());
			TestEqual(TEXT("Fast fall uses 10.5 m/s²"),
				Anomaly->GetActiveGravityForItem(Mug), 10.5f);
			const FVector DropLocation = Mug->ItemMesh->GetComponentLocation();
			AStaticMeshActor* Floor = World->SpawnActor<AStaticMeshActor>(
				DropLocation - FVector(0, 0, 125), FRotator::ZeroRotator, Params);
			Floor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
			Floor->GetStaticMeshComponent()->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,
				TEXT("/Engine/BasicShapes/Cube.Cube")));
			Floor->GetStaticMeshComponent()->SetWorldScale3D(FVector(5, 5, 1));
			Floor->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			TickPhysics(World, 90, SimulatedFrame);
			TestFalse(TEXT("Landing ends the altered fall"), Anomaly->IsAnomalyActive());
			TestTrue(TEXT("Landed prop retains normal gravity"), Mug->ItemMesh->IsGravityEnabled());
			TestTrue(TEXT("Prop reached floor"),
				Mug->ItemMesh->GetComponentLocation().Z < DropLocation.Z - 35.0f);
			TestTrue(TEXT("Landing did not introduce sideways impulse"),
				Mug->ItemMesh->GetPhysicsLinearVelocity().Size2D() < 30.0f);
			Anomaly->SetEnabled(false);
			Anomaly->MinEventInterval = 0.1f;
			Anomaly->MaxEventInterval = 0.1f;
			Anomaly->SetEnabled(true);
			TickPhysics(World, 20, SimulatedFrame);
			TestFalse(TEXT("Timer cannot re-arm grounded prop"), Anomaly->IsAnomalyActive());
			Anomaly->SetEnabled(false);
			Anomaly->MinEventInterval = 100.0f;
			Anomaly->MaxEventInterval = 100.0f;
			Anomaly->SetEnabled(true);
			if (TestTrue(TEXT("Grounded mug can be picked up again"), Mug->TryPickUp(Player)))
			{
				Mug->Drop();
				TestTrue(TEXT("New throw gets a new gravity sample"), Anomaly->IsAnomalyActive());
				TestEqual(TEXT("Repeated fast fall remains within cap"),
					Anomaly->GetActiveGravityForItem(Mug), 10.5f);
				TickPhysics(World, 15, SimulatedFrame);
				TestTrue(TEXT("Repeated fall never pauses gravity"), Mug->ItemMesh->IsGravityEnabled());
				// The rare horror variant is a continuous slow fall until an observer
				// looks away, then a capped fast fall; no impulse or teleport.
				Floor->SetActorEnableCollision(false);
				Floor->Destroy();
				TickPhysics(World, 1, SimulatedFrame);
				Anomaly->SetEnabled(false);
				Anomaly->SlowEventChance = 1.0f;
				Anomaly->MinSlowGravity = 8.5f;
				Anomaly->MaxSlowGravity = 8.5f;
				Anomaly->bEnableUnseenDrop = true;
				Anomaly->UnseenDropChance = 1.0f;
				Anomaly->bDebug = true;
				Anomaly->SetEnabled(true);
				APlayerController* Controller = World->SpawnActor<APlayerController>();
				if (TestNotNull(TEXT("Horror observer controller"), Controller)
					&& TestTrue(TEXT("Mug can be picked up for unseen drop"), Mug->TryPickUp(Player)))
				{
					Controller->Possess(Player);
					Mug->Drop();
					TestEqual(TEXT("Unseen variant begins slow"),
						Anomaly->GetActiveGravityForItem(Mug), 8.5f);
					const FVector Target = Mug->ItemMesh->GetComponentLocation();
					Player->SetActorLocation(Target - FVector(350, 0, 0));
					const UCameraComponent* Camera = Player->GetFirstPersonCameraComponent();
					Controller->SetControlRotation((Target - Camera->GetComponentLocation()).Rotation());
					TickPhysics(World, 12, SimulatedFrame);
					AddInfo(FString::Printf(TEXT("Horror sight: eye=%s target=%s control=%s velocity=%s active=%.2f"),
						*Camera->GetComponentLocation().ToCompactString(), *Target.ToCompactString(),
						*Player->GetControlRotation().ToCompactString(),
						*Mug->ItemMesh->GetPhysicsLinearVelocity().ToCompactString(),
						Anomaly->GetActiveGravityForItem(Mug)));
					TestEqual(TEXT("Looking at slow fall keeps stable acceleration"),
						Anomaly->GetActiveGravityForItem(Mug), 8.5f);
					FRotator Away = Controller->GetControlRotation();
					Away.Yaw += 180.0f;
					Controller->SetControlRotation(Away);
					TickPhysics(World, 3, SimulatedFrame);
					TestEqual(TEXT("Looking away switches only to capped 10.5 m/s²"),
						Anomaly->GetActiveGravityForItem(Mug), 10.5f);
					TestTrue(TEXT("Unseen drop still uses native gravity"),
						Mug->ItemMesh->IsGravityEnabled());
				}
			}
		}
	}
	DestroyPhysicsWorld(World);
	return true;
}

#endif
