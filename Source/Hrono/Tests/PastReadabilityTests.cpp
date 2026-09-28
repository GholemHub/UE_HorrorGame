#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Enviroment/OuijaBoard.h"
#include "Items/Clock.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPastReadableActorsTest,
	"Hrono.Presentation.PastReadableActors",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPastReadableActorsTest::RunTest(const FString& Parameters)
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

	auto FindStaticMesh = [](AActor* Actor, FName Name)
	{
		TInlineComponentArray<UStaticMeshComponent*> Meshes(Actor);
		for (UStaticMeshComponent* Mesh : Meshes)
		{
			if (Mesh->GetFName() == Name)
			{
				return Mesh;
			}
		}
		return static_cast<UStaticMeshComponent*>(nullptr);
	};

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AClock* Clock = World->SpawnActorDeferred<AClock>(AClock::StaticClass(),
		FTransform::Identity, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	for (const FName HandName : {FName(TEXT("Hour")), FName(TEXT("Minute")), FName(TEXT("Second"))})
	{
		UStaticMeshComponent* Hand = NewObject<UStaticMeshComponent>(Clock, HandName);
		Clock->AddInstanceComponent(Hand);
		Hand->SetMobility(EComponentMobility::Movable);
		Hand->SetupAttachment(Clock->GetRootComponent());
		Hand->RegisterComponent();
	}
	Clock->FinishSpawning(FTransform::Identity);
	UStaticMeshComponent* ClockSource = Clock ? Clock->GetItemMesh() : nullptr;
	const FVector ClockActorScale = Clock ? Clock->GetActorScale3D() : FVector::ZeroVector;
	const FVector ClockMeshScale = ClockSource ? ClockSource->GetRelativeScale3D() : FVector::ZeroVector;
	const ECollisionEnabled::Type ClockCollision = ClockSource
		? ClockSource->GetCollisionEnabled() : ECollisionEnabled::NoCollision;
	if (TestNotNull(TEXT("native clock"), Clock)
		&& TestNotNull(TEXT("clock source mesh"), ClockSource))
	{
		Clock->UpdateVisibilityForLocalPlayer(EItemTimeline::Past);
		UStaticMeshComponent* Mirror = FindStaticMesh(Clock, TEXT("PastMirror_ItemMesh"));
		TestNotNull(TEXT("hour hand copy"), FindStaticMesh(Clock, TEXT("PastMirror_Hour")));
		TestNotNull(TEXT("minute hand copy"), FindStaticMesh(Clock, TEXT("PastMirror_Minute")));
		TestNotNull(TEXT("second hand copy"), FindStaticMesh(Clock, TEXT("PastMirror_Second")));
		if (TestNotNull(TEXT("clock render copy"), Mirror))
		{
			TestTrue(TEXT("Past clock copy visible"), Mirror->IsVisible());
			TestEqual(TEXT("Past clock copy has no collision"), Mirror->GetCollisionEnabled(),
				ECollisionEnabled::NoCollision);
			TestEqual(TEXT("clock copy uses X reflection"),
				Mirror->GetAttachParent()->GetRelativeScale3D().X, -1.0);
		}
		FClockTimeOfDay NewTime;
		NewTime.Hour = 3;
		NewTime.Minute = 30;
		NewTime.Second = 15;
		Clock->SetCurrentTime(NewTime);
		Clock->Tick(0.016f);
		UStaticMeshComponent* Hour = FindStaticMesh(Clock, TEXT("Hour"));
		UStaticMeshComponent* MirrorHour = FindStaticMesh(Clock, TEXT("PastMirror_Hour"));
		if (TestNotNull(TEXT("source hour hand"), Hour)
			&& TestNotNull(TEXT("mirrored hour hand"), MirrorHour))
		{
			TestTrue(TEXT("mirrored hand tracks authoritative time"),
				MirrorHour->GetRelativeTransform().Equals(Hour->GetRelativeTransform()));
		}
		TestFalse(TEXT("Past clock source hidden"), ClockSource->IsVisible());
		Clock->UpdateVisibilityForLocalPlayer(EItemTimeline::Future);
		TestTrue(TEXT("Future clock source restored"), ClockSource->IsVisible());
		if (Mirror)
		{
			TestFalse(TEXT("Future clock copy hidden"), Mirror->IsVisible());
		}
		TestEqual(TEXT("clock actor scale unchanged"), Clock->GetActorScale3D(), ClockActorScale);
		TestEqual(TEXT("clock source scale unchanged"), ClockSource->GetRelativeScale3D(), ClockMeshScale);
		TestEqual(TEXT("clock collision unchanged"), ClockSource->GetCollisionEnabled(), ClockCollision);
	}

	AOuijaBoard* Board = World->SpawnActor<AOuijaBoard>(AOuijaBoard::StaticClass(),
		FTransform::Identity, SpawnParams);
	UStaticMeshComponent* BoardSource = Board ? FindStaticMesh(Board, TEXT("BoardMesh")) : nullptr;
	UStaticMeshComponent* ArrowSource = Board ? FindStaticMesh(Board, TEXT("ArrowMesh")) : nullptr;
	UBoxComponent* LetterA = nullptr;
	if (Board)
	{
		TInlineComponentArray<UBoxComponent*> Boxes(Board);
		for (UBoxComponent* Box : Boxes)
		{
			if (Box->GetFName() == TEXT("Letter_A"))
			{
				LetterA = Box;
				break;
			}
		}
	}
	const FTransform LetterATransform = LetterA ? LetterA->GetRelativeTransform() : FTransform::Identity;
	const ECollisionEnabled::Type BoardCollision = BoardSource
		? BoardSource->GetCollisionEnabled() : ECollisionEnabled::NoCollision;
	const FVector BoardScale = Board ? Board->GetActorScale3D() : FVector::ZeroVector;
	if (TestNotNull(TEXT("native board"), Board)
		&& TestNotNull(TEXT("board source mesh"), BoardSource)
		&& TestNotNull(TEXT("arrow source mesh"), ArrowSource))
	{
		Board->RefreshPastReadability(EItemTimeline::Past);
		UStaticMeshComponent* MirrorBoard = FindStaticMesh(Board, TEXT("PastMirrorBoardMesh"));
		UStaticMeshComponent* MirrorArrow = FindStaticMesh(Board, TEXT("PastMirrorArrowMesh"));
		if (TestNotNull(TEXT("board render copy"), MirrorBoard))
		{
			TestTrue(TEXT("Past board copy visible"), MirrorBoard->IsVisible());
			TestEqual(TEXT("board copy has no collision"), MirrorBoard->GetCollisionEnabled(),
				ECollisionEnabled::NoCollision);
			TestEqual(TEXT("board copy uses X reflection"),
				MirrorBoard->GetAttachParent()->GetRelativeScale3D().X, -1.0);
		}
		if (TestNotNull(TEXT("arrow render copy"), MirrorArrow))
		{
			TestTrue(TEXT("Past arrow copy visible"), MirrorArrow->IsVisible());
		}
		TestFalse(TEXT("Past board source hidden"), BoardSource->IsVisible());
		TestFalse(TEXT("Past arrow source hidden"), ArrowSource->IsVisible());
		Board->RefreshPastReadability(EItemTimeline::Future);
		TestTrue(TEXT("Future board source restored"), BoardSource->IsVisible());
		TestTrue(TEXT("Future arrow source restored"), ArrowSource->IsVisible());
		if (MirrorBoard)
		{
			TestFalse(TEXT("Future board copy hidden"), MirrorBoard->IsVisible());
		}
		TestEqual(TEXT("board actor scale unchanged"), Board->GetActorScale3D(), BoardScale);
		TestEqual(TEXT("board collision unchanged"), BoardSource->GetCollisionEnabled(), BoardCollision);
		if (TestNotNull(TEXT("authoritative Letter_A trigger"), LetterA))
		{
			TestTrue(TEXT("letter trigger transform unchanged"),
				LetterA->GetRelativeTransform().Equals(LetterATransform));
		}
	}

	World->EndPlay(EEndPlayReason::Quit);
	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return true;
}

#endif
