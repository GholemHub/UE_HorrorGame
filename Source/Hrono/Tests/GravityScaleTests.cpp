#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
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
#include "Items/GravityScaleItem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGravityScaleTest, "Hrono.Items.GravityScale",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGravityScaleTest::RunTest(const FString& Parameters)
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
	uint64 SimulatedFrame = GFrameCounter;
	auto Tick = [&](int32 Frames)
	{
		TGuardValue<uint64> RestoreFrameCounter(GFrameCounter, GFrameCounter);
		for (int32 Frame = 0; Frame < Frames; ++Frame)
		{
			GFrameCounter = ++SimulatedFrame;
			World->Tick(LEVELTICK_All, 1.0f / 60.0f);
		}
	};
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ARoom* Room = World->SpawnActor<ARoom>(ARoom::StaticClass(),
		FVector(0, 0, 100), FRotator::ZeroRotator, Params);
	Room->RoomVolume->SetBoxExtent(FVector(300, 300, 300));
	AGravityScaleItem* Scale = World->SpawnActor<AGravityScaleItem>(
		AGravityScaleItem::StaticClass(), FVector(0, 0, 100), FRotator::ZeroRotator, Params);
	ABase_Item* Item = World->SpawnActorDeferred<ABase_Item>(ABase_Item::StaticClass(),
		FTransform(FVector(0, 0, 135)), nullptr, nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	Item->ItemMesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,
		TEXT("/Engine/BasicShapes/Cube.Cube")));
	Item->ItemMesh->SetRelativeScale3D(FVector(0.1f));
	Item->FinishSpawning(FTransform(FVector(0, 0, 135)));
	Item->EnableDroppedPhysics();
	Tick(150);
	const FGravityScaleReading Normal = Scale->GetReading();
	TestEqual(TEXT("Only the resting Base_Item is weighed"), Normal.Item.Get(), Item);
	TestTrue(TEXT("Chaos supplies a positive mesh mass"), Normal.MassKg > 0.0f);
	TestTrue(TEXT("Normal calibrated display matches mass"),
		FMath::IsNearlyEqual(Normal.ApparentKg, Normal.MassKg, 0.001f));
	TestTrue(TEXT("Normal weight uses m times 9.81"),
		FMath::IsNearlyEqual(Normal.WeightNewtons, Normal.MassKg * 9.81f, 0.001f));
	Room->SetCursed(true);
	Tick(15);
	const FGravityScaleReading Cursed = Scale->GetReading();
	TestTrue(TEXT("Cursed room switches the scale's gravity reference"), Cursed.bCursedRoom);
	TestEqual(TEXT("Default cursed gravity is 10.5 m/s²"), Cursed.GravityMS2, 10.5f);
	TestTrue(TEXT("Cursed room displays increased apparent kilograms"),
		Cursed.ApparentKg > Normal.ApparentKg);
	Item->Destroy();
	Tick(15);
	TestNull(TEXT("Removing the item clears the reading"), Scale->GetReading().Item.Get());
	AStaticMeshActor* NonItem = World->SpawnActor<AStaticMeshActor>(
		FVector(0, 0, 135), FRotator::ZeroRotator, Params);
	NonItem->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
	NonItem->GetStaticMeshComponent()->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,
		TEXT("/Engine/BasicShapes/Cube.Cube")));
	NonItem->GetStaticMeshComponent()->SetWorldScale3D(FVector(0.1f));
	NonItem->GetStaticMeshComponent()->SetSimulatePhysics(true);
	Tick(90);
	TestNull(TEXT("A non-Base_Item cannot be weighed"), Scale->GetReading().Item.Get());
	NonItem->Destroy();
	UClass* CharacterClass = LoadClass<AHronoCharacter>(nullptr,
		TEXT("/Game/_Alex/HE_CharacterHrono1.HE_CharacterHrono1_C"));
	AHronoCharacter* Player = CharacterClass ? World->SpawnActor<AHronoCharacter>(CharacterClass,
		FVector(0, 0, 150), FRotator::ZeroRotator, Params) : nullptr;
	if (TestNotNull(TEXT("Character for scale pickup"), Player)
		&& TestTrue(TEXT("The scale can be picked up as a normal item"), Scale->TryPickUp(Player)))
	{
		TestTrue(TEXT("Held scale has an owner"), Scale->OwningCharacter == Player);
		Scale->Drop();
		TestNull(TEXT("Dropped scale releases ownership"), Scale->OwningCharacter);
		TestTrue(TEXT("Dropped scale resumes mesh physics"), Scale->GetItemMesh()->IsSimulatingPhysics());
	}
	World->EndPlay(EEndPlayReason::Quit);
	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return true;
}

#endif
