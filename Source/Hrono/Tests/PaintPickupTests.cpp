#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Items/PaintItem.h"
#include "Enviroment/Room.h"
#include "HronoCharacter.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/SceneCapture2D.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "NiagaraComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPaintCannotBeHeldTest,
	"Hrono.Items.PaintCannotBeHeld",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPaintCannotBeHeldTest::RunTest(const FString& Parameters)
{
	for (const TCHAR* Path : {
		TEXT("/Game/_Alex/Paints/BP_PaintItem.BP_PaintItem_C"),
		TEXT("/Game/_Alex/Paints/BP_PaintItem1.BP_PaintItem1_C"),
		TEXT("/Game/_Alex/Paints/BP_PaintItem2.BP_PaintItem2_C"),
		TEXT("/Game/_Alex/Paints/BP_PaintItem3.BP_PaintItem3_C"),
		TEXT("/Game/_Alex/Paints/BP_PaintItem4.BP_PaintItem4_C"),
		TEXT("/Game/_Alex/Paints/BP_PaintItem5.BP_PaintItem5_C")})
	{
		UClass* PaintingClass = LoadClass<APaintItem>(nullptr, Path);
		if (TestNotNull(FString::Printf(TEXT("painting class %s"), Path), PaintingClass))
		{
			TestFalse(FString::Printf(TEXT("painting CDO cannot be carried %s"), Path),
				PaintingClass->GetDefaultObject<APaintItem>()->CanBePickedUp());
		}
	}

	const auto Settings = UWorld::InitializationValues().AllowAudioPlayback(false)
		.CreatePhysicsScene(true).CreateNavigation(false).CreateAISystem(false)
		.ShouldSimulatePhysics(true).SetTransactional(false);
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr, true,
		ERHIFeatureLevel::Num, &Settings);
	GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	World->BeginPlay();
	World->GetWorldSettings()->NotifyBeginPlay();

	UClass* CharacterClass = LoadClass<AHronoCharacter>(nullptr,
		TEXT("/Game/_Alex/HE_CharacterHrono1.HE_CharacterHrono1_C"));
	AHronoCharacter* Character = CharacterClass
		? World->SpawnActor<AHronoCharacter>(CharacterClass) : nullptr;
	ASceneCapture2D* MirrorCaptureActor = World->SpawnActor<ASceneCapture2D>();
	ABase_Item* MonocleLikeItem = World->SpawnActor<ABase_Item>();
	USceneCaptureComponent2D* MonocleCapture = IsValid(MonocleLikeItem)
		? NewObject<USceneCaptureComponent2D>(MonocleLikeItem) : nullptr;
	if (IsValid(MonocleLikeItem) && IsValid(MonocleCapture))
	{
		MonocleLikeItem->bCanRepelMannequin = true;
		MonocleLikeItem->bUseCenteredInteractionPoint = true;
		MonocleLikeItem->AddInstanceComponent(MonocleCapture);
		MonocleCapture->SetupAttachment(MonocleLikeItem->GetRootComponent());
		MonocleCapture->RegisterComponent();
	}
	APaintItem* Painting = World->SpawnActor<APaintItem>();
	if (TestNotNull(TEXT("server character"), Character)
		&& TestNotNull(TEXT("server painting"), Painting))
	{
		const FTransform OriginalTransform = Painting->GetActorTransform();
		const ECollisionEnabled::Type OriginalCollision = Painting->ItemMesh->GetCollisionEnabled();
		Painting->SetPaintAnomalyType(EPaintAnomalyType::Eyes);
		TestTrue(TEXT("Eyes are the only visible clue"),
			Painting->LeftEyeMesh->IsVisible() && Painting->RightEyeMesh->IsVisible()
			&& !Painting->TentacleEffect->IsVisible()
			&& !Painting->CubeAnomalyMesh->IsVisible());
		Painting->SetPaintAnomalyType(EPaintAnomalyType::Tentacles);
		TestTrue(TEXT("Tentacles replace eyes"),
			!Painting->LeftEyeMesh->IsVisible() && !Painting->RightEyeMesh->IsVisible()
			&& Painting->TentacleEffect->IsVisible()
			&& !Painting->CubeAnomalyMesh->IsVisible());
		Painting->SetPaintAnomalyType(EPaintAnomalyType::TextureCube);
		TestTrue(TEXT("Cube canvas replaces both old clues"),
			!Painting->LeftEyeMesh->IsVisible() && !Painting->RightEyeMesh->IsVisible()
			&& !Painting->TentacleEffect->IsVisible()
			&& Painting->CubeAnomalyMesh->IsVisible());
		TestTrue(TEXT("Cube canvas is scene-capture-only"),
			Painting->CubeAnomalyMesh->bVisibleInSceneCaptureOnly != 0);
		TestEqual(TEXT("Cube canvas never blocks sight or pickup"),
			Painting->CubeAnomalyMesh->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
		if (TestNotNull(TEXT("Other scene capture"), MirrorCaptureActor)
			&& TestNotNull(TEXT("Monocle scene capture"), MonocleCapture))
		{
			TestTrue(TEXT("Mirror capture excludes the cube canvas"),
				MirrorCaptureActor->GetCaptureComponent2D()->HiddenComponents.Contains(
					TWeakObjectPtr<UPrimitiveComponent>(Painting->CubeAnomalyMesh)));
			TestFalse(TEXT("Monocle capture keeps the cube canvas"),
				MonocleCapture->HiddenComponents.Contains(
					TWeakObjectPtr<UPrimitiveComponent>(Painting->CubeAnomalyMesh)));
		}
		Painting->SetPaintAnomalyType(EPaintAnomalyType::None);
		TestFalse(TEXT("None hides cube canvas"), Painting->CubeAnomalyMesh->IsVisible());
		Painting->SetPaintAnomalyType(EPaintAnomalyType::Eyes);
		TestFalse(TEXT("painting is not a pickup target"), Painting->CanBePickedUp());
		TestFalse(TEXT("painting has no pickup highlight"), Painting->CanHighlightFor(Character));
		TestFalse(TEXT("direct item acquisition rejected"), Painting->TryPickUp(Character));
		struct { ABase_Item* Item; } PickupArgs{Painting};
		Character->ProcessEvent(Character->FindFunctionChecked(TEXT("ServerPickupItem")), &PickupArgs);
		TestNull(TEXT("hand remains empty"), Character->GetHeldItem());
		TestNull(TEXT("painting has no owning character"), Painting->OwningCharacter);
		TestFalse(TEXT("painting held flag remains clear"), Painting->bIsPickedUp);
		TestTrue(TEXT("painting stays in the world"),
			Painting->GetActorTransform().Equals(OriginalTransform));
		TestEqual(TEXT("painting collision remains authored"),
			Painting->ItemMesh->GetCollisionEnabled(), OriginalCollision);
		TestEqual(TEXT("painting anomaly remains active"),
			Painting->GetPaintAnomalyType(), EPaintAnomalyType::Eyes);
	}

	World->EndPlay(EEndPlayReason::Quit);
	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPaintAnomalySelectionTest,
	"Hrono.Items.PaintAnomalySelection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPaintAnomalySelectionTest::RunTest(const FString& Parameters)
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

	ARoom* Room = World->SpawnActor<ARoom>();
	APaintItem* Past = World->SpawnActor<APaintItem>();
	APaintItem* Future = World->SpawnActor<APaintItem>();
	APaintItem* PastAlternate = World->SpawnActor<APaintItem>();
	APaintItem* FutureAlternate = World->SpawnActor<APaintItem>();
	if (TestNotNull(TEXT("Test room"), Room)
		&& TestNotNull(TEXT("Past painting"), Past)
		&& TestNotNull(TEXT("Future painting"), Future)
		&& TestNotNull(TEXT("Alternate Past painting"), PastAlternate)
		&& TestNotNull(TEXT("Alternate Future painting"), FutureAlternate))
	{
		Past->ItemTimeline = EItemTimeline::Past;
		Future->ItemTimeline = EItemTimeline::Future;
		PastAlternate->ItemTimeline = EItemTimeline::Past;
		FutureAlternate->ItemTimeline = EItemTimeline::Future;
		FRoomPaintingPair FirstPair;
		FirstPair.Past = Past;
		FirstPair.Future = Future;
		FRoomPaintingPair SecondPair;
		SecondPair.Past = PastAlternate;
		SecondPair.Future = FutureAlternate;
		Room->PaintingPairs = {FirstPair, SecondPair};
		Room->SetCursed(true);
		TSet<EPaintAnomalyType> ObservedTypes;
		TSet<int32> ObservedPairs;
		for (int32 Seed = 1; Seed <= 24; ++Seed)
		{
			Room->ConfigurePaintingEvidence(0, Seed);
			TestEqual(TEXT("Cursed room selects one complete pair"),
				Room->GetSelectedCursedPaintings().Num(), 2);
			const TArray<AActor*> Selected = Room->GetSelectedCursedPaintings();
			if (Selected.Num() != 2)
			{
				continue;
			}
			const int32 PairIndex = Selected[0] == Past ? 0 : 1;
			ObservedPairs.Add(PairIndex);
			TestTrue(TEXT("Selection cannot mix paintings from different pairs"),
				(PairIndex == 0 && Selected[1] == Future)
				|| (PairIndex == 1 && Selected[0] == PastAlternate
					&& Selected[1] == FutureAlternate));
			APaintItem* SelectedPast = CastChecked<APaintItem>(Selected[0]);
			APaintItem* SelectedFuture = CastChecked<APaintItem>(Selected[1]);
			const EPaintAnomalyType PastType = SelectedPast->GetPaintAnomalyType();
			const EPaintAnomalyType FutureType = SelectedFuture->GetPaintAnomalyType();
			TestTrue(TEXT("Past painting receives exactly one clue"),
				PastType == EPaintAnomalyType::Eyes
				|| PastType == EPaintAnomalyType::Tentacles
				|| PastType == EPaintAnomalyType::TextureCube);
			TestEqual(TEXT("Both halves of a pair have the same clue"), FutureType, PastType);
			ObservedTypes.Add(PastType);
			Room->ConfigurePaintingEvidence(0, Seed);
			TestTrue(TEXT("Same seed keeps pair stable"),
				Room->GetSelectedCursedPaintings() == Selected);
			TestEqual(TEXT("Same seed keeps clue stable"),
				SelectedPast->GetPaintAnomalyType(), PastType);
		}
		TestEqual(TEXT("Seeded selection covers all three clue types"),
			ObservedTypes.Num(), 3);
		TestEqual(TEXT("Seeded selection covers both pairs"), ObservedPairs.Num(), 2);
		Room->SetPaintingAnomalyAvailability(false, false);
		Room->ConfigurePaintingEvidence(0, 7);
		TestEqual(TEXT("Only cubemap remains when eyes and tentacles are disabled"),
			CastChecked<APaintItem>(Room->GetSelectedCursedPaintings()[0])->GetPaintAnomalyType(),
			EPaintAnomalyType::TextureCube);
		Room->SetPaintingAnomalyAvailability(true, false);
		for (int32 Seed = 1; Seed <= 8; ++Seed)
		{
			Room->ConfigurePaintingEvidence(0, Seed);
			TestTrue(TEXT("Tentacles are excluded"),
				Past->GetPaintAnomalyType() != EPaintAnomalyType::Tentacles
				&& Future->GetPaintAnomalyType() != EPaintAnomalyType::Tentacles
				&& PastAlternate->GetPaintAnomalyType() != EPaintAnomalyType::Tentacles
				&& FutureAlternate->GetPaintAnomalyType() != EPaintAnomalyType::Tentacles);
		}
		Room->SetPaintingAnomalyAvailability(true, true);
		Room->SetCursed(false);
		for (int32 Seed = 1; Seed <= 8; ++Seed)
		{
			Room->ConfigurePaintingEvidence(1, Seed);
			TestEqual(TEXT("Ordinary room keeps one false-positive pair"),
				Room->GetSelectedCursedPaintings().Num(), 2);
			TestTrue(TEXT("Ordinary room never gives the cube clue"),
				Past->GetPaintAnomalyType() != EPaintAnomalyType::TextureCube
				&& Future->GetPaintAnomalyType() != EPaintAnomalyType::TextureCube
				&& PastAlternate->GetPaintAnomalyType() != EPaintAnomalyType::TextureCube
				&& FutureAlternate->GetPaintAnomalyType() != EPaintAnomalyType::TextureCube);
		}
		Room->SetPaintingAnomalyAvailability(false, false);
		Room->ConfigurePaintingEvidence(1, 5);
		TestTrue(TEXT("Ordinary room has no disallowed false-positive clue"),
			Room->GetSelectedCursedPaintings().IsEmpty());
		Room->SetPaintingAnomalyAvailability(true, true);
		Room->PaintingPairs.Reset();
		Room->Paintings = {Past, Future};
		Room->SetCursed(true);
		Room->ConfigurePaintingEvidence(0, 11);
		TestEqual(TEXT("Unmigrated flat painting list still selects both timelines"),
			Room->GetSelectedCursedPaintings().Num(), 2);
		TestEqual(TEXT("Legacy selected paintings also share one clue"),
			Past->GetPaintAnomalyType(), Future->GetPaintAnomalyType());
	}

	World->EndPlay(EEndPlayReason::Quit);
	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return true;
}

#endif
