#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Items/PaintItem.h"
#include "HronoCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"

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
	APaintItem* Painting = World->SpawnActor<APaintItem>();
	if (TestNotNull(TEXT("server character"), Character)
		&& TestNotNull(TEXT("server painting"), Painting))
	{
		const FTransform OriginalTransform = Painting->GetActorTransform();
		const ECollisionEnabled::Type OriginalCollision = Painting->ItemMesh->GetCollisionEnabled();
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

#endif
