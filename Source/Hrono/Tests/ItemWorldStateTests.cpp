#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Items/Base_Item.h"
#include "Items/Rune_Item.h"
#include "Items/Drag_Item.h"
#include "Items/TimelineTransferItem.h"
#include "Items/HidingWardrobe.h"
#include "Items/ThreeDrawerCabinet.h"
#include "Components/HeldItemInertiaComponent.h"
#include "Components/RitualCandleComponent.h"
#include "Ritual/RitualCandleActor.h"
#include "Ritual/TableRitualManager.h"
#include "Enviroment/OuijaBoard.h"
#include "Items/Chair.h"
#include "Items/RitualBottle.h"
#include "Ritual/TableRitualGate.h"
#include "Items/RunePentagram.h"
#include "Camera/CameraComponent.h"
#include "HronoCollisionChannels.h"
#include <limits>
#include "Items/RitualGoatSkull.h"
#include "HronoCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Tests/WardrobeSafetyTestObserver.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "TimerManager.h"
#include "UObject/UnrealType.h"
#include <algorithm>

namespace ItemWorldTests
{
// Transient runtime world: no map or Blueprint assets are saved by these tests.
struct FWorldScope
{
	UWorld* World;
	explicit FWorldScope(bool bStartPlay = true)
	{
		const auto Settings = UWorld::InitializationValues().AllowAudioPlayback(false)
			.CreatePhysicsScene(true).CreateNavigation(false).CreateAISystem(false)
			.ShouldSimulatePhysics(true).SetTransactional(false);
		World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr, true,
			ERHIFeatureLevel::Num, &Settings);
		GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
		World->InitializeActorsForPlay(FURL());
		if (bStartPlay)
		{
			World->BeginPlay();
			// This empty world has no GameMode to dispatch BeginPlay to actors.
			World->GetWorldSettings()->NotifyBeginPlay();
		}
	}
	~FWorldScope()
	{
		if (World->HasBegunPlay())
		{
			World->EndPlay(EEndPlayReason::Quit);
		}
		World->DestroyWorld(false);
		GEngine->DestroyWorldContext(World);
	}
};

template<class T = ABase_Item>
T* Spawn(UWorld* World, const FTransform& Offset = FTransform::Identity)
{
	const FTransform Pose(FVector(100.0, 200.0, 500.0));
	T* Item = World->SpawnActorDeferred<T>(T::StaticClass(), Pose, nullptr, nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	Item->ItemMesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")));
	if (Item->ItemMesh != Item->GetRootComponent())
	{
		Item->ItemMesh->SetRelativeTransform(Offset);
	}
	Item->FinishSpawning(Pose);
	return Item;
}

void SetFlag(ABase_Item* Item, const TCHAR* Name, bool Value)
{
	FBoolProperty* Property = FindFProperty<FBoolProperty>(ABase_Item::StaticClass(), Name);
	check(Property);
	Property->SetPropertyValue_InContainer(Item, Value);
}

bool GetFlag(ABase_Item* Item, const TCHAR* Name)
{
	FBoolProperty* Property = FindFProperty<FBoolProperty>(ABase_Item::StaticClass(), Name);
	check(Property);
	return Property->GetPropertyValue_InContainer(Item);
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMirrorTransferProximityTest,
	"Hrono.Items.MirrorTransferProximity", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMirrorTransferProximityTest::RunTest(const FString& Parameters)
{
	using namespace ItemWorldTests;
	FWorldScope Scope(false);
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	UClass* CharacterClass = LoadClass<AHronoCharacter>(nullptr,
		TEXT("/Game/_Alex/HE_CharacterHrono1.HE_CharacterHrono1_C"));
	if (!TestNotNull(TEXT("Character Blueprint class"), CharacterClass))
	{
		return false;
	}
	ATimelineTransferItem* Surface = Scope.World->SpawnActor<ATimelineTransferItem>(
		ATimelineTransferItem::StaticClass(), FVector(1000.0f, 200.0f, 140.0f),
		FRotator(0.0f, 90.0f, 0.0f), Params);
	AHronoCharacter* Character = Scope.World->SpawnActor<AHronoCharacter>(
		CharacterClass, FVector::ZeroVector, FRotator::ZeroRotator, Params);
	if (!TestNotNull(TEXT("Transfer surface"), Surface)
		|| !TestNotNull(TEXT("Source character"), Character))
	{
		return false;
	}

	Surface->SetActorScale3D(FVector(6.0f, 0.7f, 1.0f));
	Surface->MaxCharacterDistanceToMirror = 150.0f;
	const UBoxComponent* Box = Surface->TransferBox;
	const FVector Center = Box->GetComponentLocation();
	const FVector Normal = Box->GetForwardVector();
	const FVector Right = Box->GetRightVector();
	const FVector Up = Box->GetUpVector();

	Character->SetActorLocation(Center + Normal * 100.0f);
	TestTrue(TEXT("Character one meter in front of rotated mirror is near"),
		Surface->IsCharacterNearMirror(*Character));
	Character->SetActorLocation(Center + Normal * 250.0f);
	TestFalse(TEXT("Character far from mirror plane is rejected"),
		Surface->IsCharacterNearMirror(*Character));
	Character->SetActorLocation(Center + Normal * 100.0f
		+ Right * (Box->GetScaledBoxExtent().Y + 300.0f));
	TestFalse(TEXT("Character far beyond mirror edge is rejected"),
		Surface->IsCharacterNearMirror(*Character));
	Character->SetActorLocation(Center + Normal * 100.0f
		+ Up * (Box->GetScaledBoxExtent().Z + 300.0f));
	TestFalse(TEXT("Character on another floor is rejected"),
		Surface->IsCharacterNearMirror(*Character));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRitualCandleStateTest,
	"Hrono.Items.RitualCandleState", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRitualCandleStateTest::RunTest(const FString& Parameters)
{
	using namespace ItemWorldTests;
	FWorldScope Scope;
	UClass* CandleClass = LoadClass<ARitualCandleActor>(nullptr,
		TEXT("/Game/_Alex/Pickable/BP_Item_Candle.BP_Item_Candle_C"));
	if (!TestNotNull(TEXT("Ritual candle Blueprint"), CandleClass))
	{
		return false;
	}
	ARitualCandleActor* Candle = Scope.World->SpawnActor<ARitualCandleActor>(CandleClass);
	URitualCandleComponent* State = IsValid(Candle) ? Candle->RitualCandle : nullptr;
	if (!TestNotNull(TEXT("Replicated ritual candle state component"), State))
	{
		return false;
	}
	TestTrue(TEXT("Candle actor replicates"), Candle->GetIsReplicated());
	TestTrue(TEXT("Candle component replicates"), State->GetIsReplicated());
	TArray<URitualCandleComponent*> StateComponents;
	Candle->GetComponents<URitualCandleComponent>(StateComponents);
	TestEqual(TEXT("Exactly one native ritual candle component"), StateComponents.Num(), 1);
	TestTrue(TEXT("Native actor owns the replicated component"),
		StateComponents.Num() == 1 && StateComponents[0] == State);
	TestEqual(TEXT("Candle starts unlit"), State->GetLitMask(), uint8(0));
	if (!TestEqual(TEXT("Three configured candle flames"), State->FlameComponentNames.Num(), 3))
	{
		return false;
	}

	TArray<UParticleSystemComponent*> Flames;
	Candle->GetComponents<UParticleSystemComponent>(Flames);
	TestEqual(TEXT("Three authored flame components"), Flames.Num(), 3);
	for (UParticleSystemComponent* Flame : Flames)
	{
		TestFalse(TEXT("Flame starts hidden"), Flame->IsVisible());
	}
	auto FlameAt = [&Flames, State](int32 Index) -> UParticleSystemComponent*
	{
		for (UParticleSystemComponent* Flame : Flames)
		{
			if (Flame->GetFName() == State->FlameComponentNames[Index])
			{
				return Flame;
			}
		}
		return nullptr;
	};

	Candle->StartRitualLighting();
	{
		TGuardValue<uint64> RestoreFrameCounter(GFrameCounter, GFrameCounter);
		++GFrameCounter;
		Scope.World->Tick(LEVELTICK_All, 0.01f);
		for (uint8 Step = 0; Step < 3; ++Step)
		{
			// Advance real game frames; UWorld clamps a single large DeltaSeconds.
			for (int32 Frame = 0; Frame < 11; ++Frame)
			{
				++GFrameCounter;
				Scope.World->Tick(LEVELTICK_All, 0.1f);
			}
			TestEqual(TEXT("One more flame lights per interval"), State->GetLitMask(),
				static_cast<uint8>((1u << (Step + 1)) - 1u));
			if (UParticleSystemComponent* Flame = FlameAt(Step))
			{
				TestTrue(TEXT("New flame is visible"), Flame->IsVisible());
				TestTrue(TEXT("Child candle light is visible"),
					Flame->GetAttachChildren().Num() > 0 && Flame->GetAttachChildren()[0]->IsVisible());
			}
			else
			{
				AddError(TEXT("Configured flame component is missing"));
			}
		}
	}
	Candle->ReportRitualMistake();
	TestEqual(TEXT("First mistake extinguishes first flame"), State->GetLitMask(), uint8(6));
	TestEqual(TEXT("First candle body hides as in the authored Blueprint"),
		State->GetHiddenBodyMask(), uint8(1));
	if (UParticleSystemComponent* FirstFlame = FlameAt(0))
	{
		TestFalse(TEXT("First candle body is hidden"), FirstFlame->GetAttachParent()->IsVisible());
	}
	Candle->ReportRitualMistake();
	TestEqual(TEXT("Second mistake extinguishes second flame"), State->GetLitMask(), uint8(4));
	TestEqual(TEXT("Second candle body hides"), State->GetHiddenBodyMask(), uint8(3));
	Candle->ReportRitualMistake();
	TestEqual(TEXT("Third mistake extinguishes all flames"), State->GetLitMask(), uint8(0));
	TestEqual(TEXT("All candle bodies hide"), State->GetHiddenBodyMask(), uint8(7));
	Candle->ReportRitualMistake();
	TestEqual(TEXT("Extra mistake cannot corrupt the completed state"),
		State->GetHiddenBodyMask(), uint8(7));
	Candle->StartRitualLighting();
	TestEqual(TEXT("Restart clears old flame state"), State->GetLitMask(), uint8(0));
	TestEqual(TEXT("Restart restores candle bodies"), State->GetHiddenBodyMask(), uint8(0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTableRitualStateTest,
	"Hrono.Items.TableRitualState", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTableRitualStateTest::RunTest(const FString& Parameters)
{
	using namespace ItemWorldTests;
	FWorldScope Scope;
	UClass* ManagerClass = LoadClass<ATableRitualManager>(nullptr,
		TEXT("/Game/_Alex/Room/BP_TableRitualManager.BP_TableRitualManager_C"));
	if (!TestNotNull(TEXT("Native table manager Blueprint"), ManagerClass)) return false;

	AOuijaBoard* Board = Scope.World->SpawnActor<AOuijaBoard>();
	AChair* Sliding = Spawn<AChair>(Scope.World);
	if (!TestNotNull(TEXT("Ouija board"), Board)
		|| !TestNotNull(TEXT("Sliding chair test actor"), Sliding)) return false;

	const FTransform Pose(FVector(0.0f, 0.0f, 200.0f));
	ATableRitualManager* Manager = Scope.World->SpawnActorDeferred<ATableRitualManager>(
		ManagerClass, Pose);
	if (!TestNotNull(TEXT("Table manager"), Manager)) return false;
	Manager->OuijaBoard = Board;
	Manager->SlidingChair = Sliding;
	Manager->FinishSpawning(Pose);
	TestTrue(TEXT("Manager replicates its snapshot"), Manager->GetIsReplicated());
	TestTrue(TEXT("Manager stays relevant for late join"), Manager->bAlwaysRelevant);
	TestTrue(TEXT("Board starts hidden before a victim is chosen"), Board->IsHidden());
	TestFalse(TEXT("Cannot start without two seated players and dependencies"),
		Manager->TryStartTableRitual());
	TestEqual(TEXT("Rejected start leaves attempts unchanged"),
		Manager->RitualState.AttemptsRemaining, 3);
	TestEqual(TEXT("Rejected start leaves phase idle"),
		Manager->RitualState.Phase, ETableRitualPhase::Idle);
	TestFalse(TEXT("Only the chosen victim can finish an attempt"),
		Manager->CompleteVictimReturn(nullptr));

	Manager->RitualState.bBoardVisible = true;
	Manager->RitualState.BoardYaw = 180.0f;
	Manager->RitualState.ChairSlideDuration = 1.0f;
	Manager->RitualState.ChairTarget = FVector(250.0f, -80.0f, 500.0f);
	Manager->RitualState.Phase = ETableRitualPhase::Exhausted;
	UFunction* RepNotify = Manager->FindFunction(TEXT("OnRep_RitualState"));
	if (!TestNotNull(TEXT("Table snapshot rep-notify"), RepNotify)) return false;
	Manager->ProcessEvent(RepNotify, nullptr);
	TestFalse(TEXT("Late snapshot reveals the board"), Board->IsHidden());
	TestTrue(TEXT("Late snapshot restores chosen board orientation"),
		FMath::IsNearlyEqual(Board->GetActorRotation().Yaw, 180.0f));
	TestTrue(TEXT("Late snapshot restores final sliding chair pose"),
		Sliding->GetActorLocation().Equals(Manager->RitualState.ChairTarget));

	Sliding->SetRitualStarted(true);
	TestTrue(TEXT("A ritual chair becomes replicated"), Sliding->GetIsReplicated());
	TestTrue(TEXT("Its ritual state is server-owned"), Sliding->IsRitualStarted);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTableRitualFlowTest,
	"Hrono.Items.TableRitualFlow", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTableRitualFlowTest::RunTest(const FString& Parameters)
{
	using namespace ItemWorldTests;
	FWorldScope Scope;
	AChair* ChairA = Spawn<AChair>(Scope.World);
	AChair* ChairB = Spawn<AChair>(Scope.World);
	AChair* Sliding = Spawn<AChair>(Scope.World);
	AChair* RitualPoint = Spawn<AChair>(Scope.World);
	AOuijaBoard* Board = Scope.World->SpawnActor<AOuijaBoard>();
	ARitualBottle* Bottle = Scope.World->SpawnActor<ARitualBottle>();
	ARitualCandleActor* Candle = Scope.World->SpawnActor<ARitualCandleActor>();
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	UClass* CharacterClass = LoadClass<AHronoCharacter>(nullptr,
		TEXT("/Game/_Alex/HE_CharacterHrono1.HE_CharacterHrono1_C"));
	if (!TestNotNull(TEXT("Playable character class"), CharacterClass)) return false;
	AHronoCharacter* PlayerA = Scope.World->SpawnActor<AHronoCharacter>(
		CharacterClass, FVector(0.0f, 0.0f, 200.0f), FRotator::ZeroRotator, Params);
	AHronoCharacter* PlayerB = Scope.World->SpawnActor<AHronoCharacter>(
		CharacterClass, FVector(200.0f, 0.0f, 200.0f), FRotator::ZeroRotator, Params);
	APlayerController* ControllerA = Scope.World->SpawnActor<APlayerController>(
		APlayerController::StaticClass(), FVector(400.0f, 0.0f, 200.0f), FRotator::ZeroRotator, Params);
	APlayerController* ControllerB = Scope.World->SpawnActor<APlayerController>(
		APlayerController::StaticClass(), FVector(600.0f, 0.0f, 200.0f), FRotator::ZeroRotator, Params);
	if (!TestNotNull(TEXT("Table chair A"), ChairA)
		|| !TestNotNull(TEXT("Table chair B"), ChairB)
		|| !TestNotNull(TEXT("Bottle"), Bottle)
		|| !TestNotNull(TEXT("Candle"), Candle)
		|| !TestNotNull(TEXT("Two characters"), PlayerA)
		|| !TestNotNull(TEXT("Second character"), PlayerB)
		|| !TestNotNull(TEXT("Two controllers"), ControllerA)
		|| !TestNotNull(TEXT("Second controller"), ControllerB)) return false;
	ControllerA->Possess(PlayerA);
	ControllerB->Possess(PlayerB);
	Bottle->SpinDuration = 0.2f;
	const FTransform Pose(FVector(0.0f, 0.0f, 200.0f));
	ATableRitualManager* Manager = Scope.World->SpawnActorDeferred<ATableRitualManager>(
		ATableRitualManager::StaticClass(), Pose);
	if (!TestNotNull(TEXT("Server table manager"), Manager)) return false;
	Manager->TableChairA = ChairA;
	Manager->TableChairB = ChairB;
	Manager->SlidingChair = Sliding;
	Manager->RitualBottle = Bottle;
	Manager->RitualCandle = Candle;
	Manager->OuijaBoard = Board;
	Manager->VictimRitualPoint = RitualPoint;
	Manager->ChairSlideDuration = 0.1f;
	Manager->FinishSpawning(Pose);

	UClass* ImageClass = LoadClass<ABase_Item>(nullptr,
		TEXT("/Game/_Alex/Paints/BP_CursedImage_Item.BP_CursedImage_Item_C"));
	if (!TestNotNull(TEXT("Cursed image unlock asset"), ImageClass)) return false;
	ABase_Item* Image = Scope.World->SpawnActor<ABase_Item>(ImageClass);
	if (!TestNotNull(TEXT("Cursed image"), Image)) return false;
	Image->OwningCharacter = PlayerA;
	Image->bIsPickedUp = true;
	TableRitualGate::NotifySuccessfulPickup(*Image, *PlayerA);
	TestTrue(TEXT("Pickup unlocks configured table chairs"),
		ChairA->IsRitualGuidanceUnlocked() && ChairB->IsRitualGuidanceUnlocked());
	TestFalse(TEXT("One player cannot start the table ritual"), Manager->TryStartTableRitual());
	if (!TestTrue(TEXT("First player sits"), PlayerA->ForceSitOnChair(ChairA))) return false;
	TestEqual(TEXT("Still idle with only one sitter"),
		Manager->RitualState.Phase, ETableRitualPhase::Idle);
	if (!TestTrue(TEXT("Second player sits"), PlayerB->ForceSitOnChair(ChairB))) return false;
	TestEqual(TEXT("Server starts exactly one ritual sequence"),
		Manager->RitualState.Phase, ETableRitualPhase::Preparing);
	TestFalse(TEXT("Duplicate start is rejected"), Manager->TryStartTableRitual());

	TGuardValue<uint64> RestoreFrameCounter(GFrameCounter, GFrameCounter);
	auto TickUntilVictim = [&]()
	{
		for (int32 Frame = 0; Frame < 220
			&& Manager->RitualState.Phase != ETableRitualPhase::VictimChosen; ++Frame)
		{
			++GFrameCounter;
			Scope.World->Tick(LEVELTICK_All, 0.1f);
		}
		return Manager->RitualState.Phase == ETableRitualPhase::VictimChosen;
	};
	for (int32 Remaining = 2; Remaining >= 0; --Remaining)
	{
		if (!TestTrue(TEXT("Bottle reaches a selected victim"), TickUntilVictim())) return false;
		AHronoCharacter* Victim = Manager->RitualState.Victim;
		TestTrue(TEXT("Selected victim belongs to a configured chair"),
			Victim == PlayerA || Victim == PlayerB);
		TestTrue(TEXT("Chosen player moves to the ritual point"), Victim->bIsAtRitualPoint);
		TestFalse(TEXT("Ouija board is visible after selection"), Board->IsHidden());
		if (!TestTrue(TEXT("Selected victim can return once"),
			Manager->CompleteVictimReturn(Victim))) return false;
		TestEqual(TEXT("One attempt consumed on server"),
			Manager->RitualState.AttemptsRemaining, Remaining);
		TestFalse(TEXT("Duplicate return cannot consume another attempt"),
			Manager->CompleteVictimReturn(Victim));
	}
	TestEqual(TEXT("Third failure ends the ritual"),
		Manager->RitualState.Phase, ETableRitualPhase::Exhausted);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FItemDroppedMovementTest,
	"Hrono.Items.DroppedMovement", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FItemDroppedMovementTest::RunTest(const FString& Parameters)
{
	using namespace ItemWorldTests;
	FWorldScope Scope;
	const FTransform Offset(FRotator(15.0, 35.0, 5.0), FVector(12.0, -7.0, 24.0), FVector(0.5));
	ABase_Item* Server = Spawn(Scope.World, Offset);
	ABase_Item* Client = Spawn(Scope.World, Offset);
	Client->SetRole(ROLE_SimulatedProxy);
	Server->EnableDroppedPhysics();
	TestTrue(TEXT("Runtime test actor has begun play"), Server->HasActorBegunPlay());
	TestTrue(TEXT("Runtime test actor tick is registered"), Server->PrimaryActorTick.IsTickFunctionRegistered());
	SetFlag(Client, TEXT("bDroppedPhysicsEnabled"), true);
	Client->OnRep_DroppedPhysicsEnabled();
	TestTrue(TEXT("Server simulates detached child mesh"), Server->ItemMesh->IsSimulatingPhysics());
	TestNull(TEXT("Server physics detached mesh"), Server->ItemMesh->GetAttachParent());
	TestFalse(TEXT("Client does not independently simulate child mesh"), Client->ItemMesh->IsSimulatingPhysics());
	TestTrue(TEXT("Client mesh follows actor root"), Client->ItemMesh->GetAttachParent() == Client->GetRootComponent());
	TestTrue(TEXT("Dropped authority tracks physics"), Server->IsActorTickEnabled());
	TestEqual(TEXT("Tracking runs after physics"), Server->PrimaryActorTick.TickGroup.GetValue(), TG_PostPhysics);
	TestFalse(TEXT("Proxy needs no tracking tick"), Client->IsActorTickEnabled());

	const FVector BeforeFall = Server->ItemMesh->GetComponentLocation();
	{
		// The tick scheduler deduplicates actor ticks by GFrameCounter. Give each
		// synthetic world frame its own counter, then restore the editor's frame.
		TGuardValue<uint64> RestoreFrameCounter(GFrameCounter, GFrameCounter);
		for (int32 Frame = 0; Frame < 30; ++Frame)
		{
			++GFrameCounter;
			Scope.World->Tick(LEVELTICK_All, 1.0f / 60.0f);
		}
	}
	TestTrue(TEXT("Real Chaos body falls"), Server->ItemMesh->GetComponentLocation().Z < BeforeFall.Z - 10.0);
	TestTrue(TEXT("Post-physics tick keeps actor aligned"),
		(Offset * Server->GetActorTransform()).Equals(Server->ItemMesh->GetComponentTransform(), 0.01));

	for (int32 Sample = 0; Sample < 3; ++Sample)
	{
		// Exercise translation, rotation and non-zero authored mesh offsets.
		const FTransform BodyPose(FRotator(20.0 * Sample, 70.0, -15.0),
			FVector(600.0 + Sample * 120.0, -70.0, 300.0 - Sample * 80.0), FVector(0.5));
		Server->ItemMesh->SetWorldTransform(BodyPose, false, nullptr, ETeleportType::TeleportPhysics);
		Server->GatherCurrentMovement();
		Client->SetReplicatedMovement(Server->GetReplicatedMovement());
		Client->OnRep_ReplicatedMovement();
		TestTrue(TEXT("Replicated root reconstructs server mesh pose"),
			Client->ItemMesh->GetComponentTransform().Equals(BodyPose, 0.01));
		TestTrue(TEXT("Root correction does not move server body"),
			Server->ItemMesh->GetComponentTransform().Equals(BodyPose, 0.01));
	}

	Server->ItemMesh->PutAllRigidBodiesToSleep();
	Server->GatherCurrentMovement();
	TestTrue(TEXT("Sleeping body preserves last actor pose"),
		(Offset * Server->GetActorTransform()).Equals(Server->ItemMesh->GetComponentTransform(), 0.01));
	const FTransform BeforeFloat = Server->ItemMesh->GetComponentTransform();
	Server->EnableFloatingPickup();
	TestFalse(TEXT("Floating stops physics"), Server->ItemMesh->IsSimulatingPhysics());
	TestFalse(TEXT("Floating removes native tracking tick"), Server->IsActorTickEnabled());
	TestTrue(TEXT("Floating preserves visible position"), Server->ItemMesh->GetComponentTransform().Equals(BeforeFloat, 0.01));

	ARitualGoatSkull* Skull = Spawn<ARitualGoatSkull>(Scope.World);
	Skull->EnableDroppedPhysics();
	TestTrue(TEXT("Skull retains physical root"), Skull->ItemMesh == Skull->GetRootComponent());
	TestTrue(TEXT("Skull retains root simulation"), Skull->ItemMesh->IsSimulatingPhysics());
	TestFalse(TEXT("Skull does not use child-mesh tracking"), Skull->IsActorTickEnabled());
	Skull->SetRole(ROLE_SimulatedProxy);
	Skull->OnRep_DroppedPhysicsEnabled();
	TestTrue(TEXT("Root-body proxies retain Unreal physics replication"), Skull->ItemMesh->IsSimulatingPhysics());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRunePlacementStateTest,
	"Hrono.Items.RunePlacementState", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRunePlacementStateTest::RunTest(const FString& Parameters)
{
	using namespace ItemWorldTests;
	FWorldScope Scope;
	ABase_Item* Pentagram = Spawn(Scope.World);
	ARune_Item* Server = Spawn<ARune_Item>(Scope.World);
	Server->RuneId = TEXT("Rune.Test");
	Server->EnableDroppedPhysics();
	const FTransform SlotPose(FRotator(0.0, 70.0, 0.0), FVector(900.0, -250.0, 150.0));
	Server->PlaceRuneInPentagram(Pentagram, TEXT("Slot.Test"), Server->RuneId, SlotPose, 3);
	TestTrue(TEXT("World rune placed successfully"), Server->bPlacedInPentagram);
	TestFalse(TEXT("Placement clears dropped flag on server"), GetFlag(Server, TEXT("bDroppedPhysicsEnabled")));
	TestFalse(TEXT("Placement clears floating flag on server"), GetFlag(Server, TEXT("bFloatingPickupEnabled")));
	Server->EnableDroppedPhysics();
	Server->EnableFloatingPickup();
	TestFalse(TEXT("World helpers cannot unlock placed rune"), Server->ItemMesh->IsSimulatingPhysics());
	TestFalse(TEXT("Placed rune has no collision"), Server->GetActorEnableCollision());
	TestFalse(TEXT("Placed rune needs no tracking tick"), Server->IsActorTickEnabled());

	ARune_Item* Client = Spawn<ARune_Item>(Scope.World);
	Client->SetRole(ROLE_SimulatedProxy);
	int32 Order[] = {0, 1, 2, 3};
	int32 Permutations = 0;
	do
	{
		Client->bPlacedInPentagram = true;
		Client->PlacedPentagram = Pentagram;
		Client->SetActorTransform(SlotPose);
		Client->AttachToActor(Pentagram, FAttachmentTransformRules::KeepWorldTransform);
		// Deliberately retain conflicting old flags: Placed must always win.
		SetFlag(Client, TEXT("bDroppedPhysicsEnabled"), true);
		SetFlag(Client, TEXT("bFloatingPickupEnabled"), true);
		for (int32 Callback : Order)
		{
			switch (Callback)
			{
			case 0: Client->OnRep_DroppedPhysicsEnabled(); break;
			case 1: Client->OnRep_FloatingPickupEnabled(); break;
			case 2: Client->OnRep_OwningCharacter(nullptr); break;
			case 3: Client->ProcessEvent(Client->FindFunctionChecked(TEXT("OnRep_PlacementState")), nullptr); break;
			}
			TestFalse(TEXT("Every notification keeps placed physics off"), Client->ItemMesh->IsSimulatingPhysics());
			TestEqual(TEXT("Every notification keeps mesh collision off"), Client->ItemMesh->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
			TestTrue(TEXT("Every notification preserves slot parent"), Client->GetAttachParentActor() == Pentagram);
			TestTrue(TEXT("Every notification preserves slot pose"), Client->GetActorTransform().Equals(SlotPose, 0.01));
		}
		++Permutations;
	} while (std::next_permutation(std::begin(Order), std::end(Order)));
	TestEqual(TEXT("All callback orders covered"), Permutations, 24);
	FRepMovement OldDrop;
	OldDrop.Location = FVector(-500.0, 500.0, 800.0);
	OldDrop.bRepPhysics = true;
	Client->SetReplicatedMovement(OldDrop);
	Client->OnRep_ReplicatedMovement();
	TestFalse(TEXT("Stale movement cannot enable placed physics"), Client->ItemMesh->IsSimulatingPhysics());
	TestTrue(TEXT("Stale movement cannot move placed rune"), Client->GetActorTransform().Equals(SlotPose, 0.01));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FItemPickupAssetsTest,
	"Hrono.Items.PickupAssetsAndPlacement", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FItemPickupAssetsTest::RunTest(const FString& Parameters)
{
	using namespace ItemWorldTests;
	// Run PostInitializeComponents, but do not start character Blueprint gameplay
	// that expects a real local controller, HUD and populated gameplay map.
	FWorldScope Scope(false);
	UClass* CharacterClass = LoadClass<AHronoCharacter>(nullptr,
		TEXT("/Game/_Alex/HE_CharacterHrono1.HE_CharacterHrono1_C"));
	if (!TestNotNull(TEXT("Gameplay character class"), CharacterClass)) return false;
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AHronoCharacter* Player = Scope.World->SpawnActor<AHronoCharacter>(CharacterClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	AHronoCharacter* Recipient = Scope.World->SpawnActor<AHronoCharacter>(CharacterClass, FVector(300.0, 0.0, 0.0), FRotator::ZeroRotator, SpawnParams);
	if (!TestNotNull(TEXT("Source player"), Player) || !TestNotNull(TEXT("Recipient player"), Recipient)) return false;
	auto Pickup = [](AHronoCharacter* Character, ABase_Item* Item)
	{
		// Arrange a reachable, unobstructed pickup. This pre-BeginPlay fixture must
		// explicitly initialize the native collision state normally applied at BeginPlay.
		if (!Item->bIsPickedUp && !(Cast<ARune_Item>(Item) && Cast<ARune_Item>(Item)->bPlacedInPentagram))
		{
			Item->EnableFloatingPickup();
			const FVector Eye = Character->FindComponentByClass<UCameraComponent>()->GetComponentLocation();
			Item->SetActorLocation(Eye + FVector(0,150,0) - Item->ItemMesh->GetRelativeLocation());
		}
		struct { ABase_Item* Item; } Args{Item};
		Character->ProcessEvent(Character->FindFunctionChecked(TEXT("ServerPickupItem")), &Args);
	};
	auto Drop = [](AHronoCharacter* Character)
	{
		Character->ProcessEvent(Character->FindFunctionChecked(TEXT("ServerDropCurrentItem")), nullptr);
	};

	for (const TCHAR* Name : {TEXT("BP_Dozimetr"), TEXT("BP_Monocle"), TEXT("BP_Key_Item"), TEXT("BP_AxeItem"), TEXT("BP_Rune_Item_1")})
	{
		const FString Path = FString::Printf(TEXT("/Game/_Alex/Pickable/%s.%s_C"), Name, Name);
		UClass* ItemClass = LoadClass<ABase_Item>(nullptr, *Path);
		if (!TestNotNull(Path, ItemClass)) return false;
		ABase_Item* Item = Scope.World->SpawnActor<ABase_Item>(ItemClass, FVector(150.0, 0.0, 200.0), FRotator::ZeroRotator, SpawnParams);
		if (!TestNotNull(TEXT("Blueprint item"), Item)) return false;
		if (FCString::Strcmp(Name, TEXT("BP_Monocle")) == 0)
		{
			USceneCaptureComponent2D* Capture = Item->FindComponentByClass<USceneCaptureComponent2D>();
			if (!TestNotNull(TEXT("Monocle SceneCapture"), Capture)) return false;
			TestFalse(TEXT("World monocle does not capture every frame"), Capture->bCaptureEveryFrame);
			TestFalse(TEXT("World monocle does not capture on movement"), Capture->bCaptureOnMovement);
			TestFalse(TEXT("World monocle capture is inactive"), Capture->IsActive());
			TestTrue(TEXT("World monocle lens uses dormant mesh material"),
				Item->ItemMesh->GetMaterial(2) == Item->ItemMesh->GetStaticMesh()->GetMaterial(2));
		}
		Item->ItemTimeline = EItemTimeline::Both;
		const FTransform AuthoredPose = Item->ItemMesh->GetRelativeTransform();
		USceneComponent* AuthoredRoot = Item->GetRootComponent();
		for (int32 Cycle = 0; Cycle < 3; ++Cycle)
		{
			const FString Label = FString::Printf(TEXT("%s cycle %d: "), Name, Cycle + 1);
			Pickup(Player, Item);
			TestTrue(Label + TEXT("pickup"), Player->GetHeldItem() == Item);
			if (FCString::Strcmp(Name, TEXT("BP_Monocle")) == 0)
			{
				TestTrue(Label + TEXT("monocle opts into centre point"), Item->bUseCenteredInteractionPoint);
				TestTrue(Label + TEXT("point is attached directly to camera"),
					Player->MonocleInteractionPoint->GetAttachParent() == Player->GetFirstPersonCameraComponent());
				const FVector PointOffset = Player->MonocleInteractionPoint->GetRelativeLocation();
				TestTrue(Label + TEXT("point lies on camera forward axis"),
					PointOffset.X > 0.0f && FMath::IsNearlyZero(PointOffset.Y) && FMath::IsNearlyZero(PointOffset.Z));
				TestTrue(Label + TEXT("Past monocle uses centre point"),
					Item->GetRootComponent()->GetAttachParent() == Player->MonocleInteractionPoint.Get());
				TestTrue(Label + TEXT("monocle has no lateral or vertical held offset"),
					Item->GetRootComponent()->GetRelativeLocation().IsNearlyZero());
				TestFalse(Label + TEXT("monocle inertia cannot displace the centre"),
					Item->HeldItemInertia->IsComponentTickEnabled());
				Player->CharacterTimeline = EItemTimeline::Future;
				TestTrue(Label + TEXT("Future reattachment succeeds"), Item->RefreshHeldAttachmentPoint());
				TestTrue(Label + TEXT("Future uses same centre point"),
					Item->GetRootComponent()->GetAttachParent() == Player->MonocleInteractionPoint.Get()
					&& Item->GetRootComponent()->GetRelativeLocation().IsNearlyZero());
				Player->CharacterTimeline = EItemTimeline::Past;
				Item->RefreshHeldAttachmentPoint();
				Recipient->CharacterTimeline = EItemTimeline::Future;
			}
			else
			{
				TestTrue(Label + TEXT("ordinary item keeps timeline hand point"),
					Item->GetRootComponent()->GetAttachParent() == Player->GetActiveInteractionPoint());
			}
			TestTrue(Label + TEXT("authored held mesh pose"), Item->ItemMesh->GetRelativeTransform().Equals(AuthoredPose, 0.01));
			TestFalse(Label + TEXT("held physics off"), Item->ItemMesh->IsSimulatingPhysics());
			TestTrue(Label + TEXT("explicit transfer"), Player->TransferHeldItemTo(Recipient, Item));
			if (FCString::Strcmp(Name, TEXT("BP_Monocle")) == 0)
			{
				TestTrue(Label + TEXT("Future recipient sees centred monocle"),
					Item->GetRootComponent()->GetAttachParent() == Recipient->MonocleInteractionPoint.Get()
					&& Item->GetRootComponent()->GetRelativeLocation().IsNearlyZero());
			}
			TestTrue(Label + TEXT("transfer preserves mesh pose"), Item->ItemMesh->GetRelativeTransform().Equals(AuthoredPose, 0.01));
			if (FCString::Strcmp(Name, TEXT("BP_Monocle")) == 0)
			{
				// Model a capture reactivated after pickup, as can happen in a
				// Blueprint callback. Drop must suppress it and its live lens surface.
				USceneCaptureComponent2D* Capture = Item->FindComponentByClass<USceneCaptureComponent2D>();
				UMaterialInterface* LiveLens = LoadObject<UMaterialInterface>(nullptr,
					TEXT("/Game/_Alex/Materials/Monocle_Material.Monocle_Material"));
				if (!TestNotNull(Label + TEXT("live lens material"), LiveLens)) return false;
				Item->ItemMesh->SetMaterial(2, LiveLens);
				Capture->bCaptureEveryFrame = true;
				Capture->bCaptureOnMovement = true;
				Capture->Activate(true);
			}
			Drop(Recipient);
			Recipient->CharacterTimeline = EItemTimeline::Past;
			TestNull(Label + TEXT("drop releases hand"), Recipient->GetHeldItem());
			// This fixture deliberately stops before BeginPlay. Verify the requested
			// body state; DroppedMovement tests active Chaos in a started game world.
			TestTrue(Label + TEXT("drop requests physics"), Item->ItemMesh->BodyInstance.bSimulatePhysics);
			TestTrue(Label + TEXT("dropped flag"), GetFlag(Item, TEXT("bDroppedPhysicsEnabled")));
			TestTrue(Label + TEXT("authored root preserved"), Item->GetRootComponent() == AuthoredRoot);
			if (FCString::Strcmp(Name, TEXT("BP_Monocle")) == 0)
			{
				const USceneCaptureComponent2D* Capture = Item->FindComponentByClass<USceneCaptureComponent2D>();
				TestFalse(Label + TEXT("dropped capture every-frame disabled"), Capture->bCaptureEveryFrame);
				TestFalse(Label + TEXT("dropped capture movement disabled"), Capture->bCaptureOnMovement);
				TestFalse(Label + TEXT("dropped capture tick disabled"), Capture->IsComponentTickEnabled());
				TestFalse(Label + TEXT("dropped capture inactive"), Capture->IsActive());
				TestTrue(Label + TEXT("dropped lens no longer samples shared render target"),
					Item->ItemMesh->GetMaterial(2) == Item->ItemMesh->GetStaticMesh()->GetMaterial(2));
			}
		}
		Item->Destroy();
	}

	ARune_Item* Rune = Spawn<ARune_Item>(Scope.World);
	ABase_Item* Pentagram = Spawn(Scope.World);
	Rune->RuneId = TEXT("Rune.Test");
	const FTransform Slot(FVector(700.0, 100.0, 250.0));
	Pickup(Player, Rune);
	TestTrue(TEXT("Rune held before placement"), Player->GetHeldItem() == Rune);
	Rune->PlaceRuneInPentagram(Pentagram, TEXT("Slot.Test"), TEXT("Rune.Wrong"), Slot, 3);
	TestTrue(TEXT("Wrong rune keeps hand occupied"), Player->GetHeldItem() == Rune);
	TestFalse(TEXT("Wrong rune is not placed"), Rune->bPlacedInPentagram);
	Rune->PlaceRuneInPentagram(Pentagram, TEXT("Slot.Test"), Rune->RuneId, Slot, 3);
	TestNull(TEXT("Placement releases hand"), Player->GetHeldItem());
	TestNull(TEXT("Placement clears replicated owner"), Rune->OwningCharacter);
	TestNull(TEXT("Placement clears network owner"), Rune->GetOwner());
	TestFalse(TEXT("Placement clears held flag"), Rune->bIsPickedUp);
	TestTrue(TEXT("Placement locks rune"), Rune->bPlacedInPentagram);
	TestFalse(TEXT("Held placement clears dropped flag"), GetFlag(Rune, TEXT("bDroppedPhysicsEnabled")));
	TestFalse(TEXT("Held placement clears floating flag"), GetFlag(Rune, TEXT("bFloatingPickupEnabled")));
	TestTrue(TEXT("Placement attaches to pentagram"), Rune->GetAttachParentActor() == Pentagram);
	TestFalse(TEXT("Held placement disables physics"), Rune->ItemMesh->IsSimulatingPhysics());
	TestTrue(TEXT("Held placement preserves slot pose"), Rune->GetActorTransform().Equals(Slot, 0.01));
	Pickup(Recipient, Rune);
	TestNull(TEXT("Placed rune cannot be picked up"), Recipient->GetHeldItem());
	Rune->Drop();
	Rune->EnableDroppedPhysics();
	TestTrue(TEXT("Drop helpers cannot detach placed rune"), Rune->GetAttachParentActor() == Pentagram);
	TestFalse(TEXT("Drop helpers cannot simulate placed rune"), Rune->ItemMesh->IsSimulatingPhysics());
	ABase_Item* Spare = Spawn(Scope.World);
	Pickup(Player, Spare);
	TestTrue(TEXT("Hand remains usable after placement"), Player->GetHeldItem() == Spare);
	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FServerInteractionRulesTest,
	"Hrono.Items.ServerInteractionRules", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FServerInteractionRulesTest::RunTest(const FString& Parameters)
{
	using namespace ItemWorldTests;
	FWorldScope Scope(false);
	UClass* CharacterClass = LoadClass<AHronoCharacter>(nullptr,
		TEXT("/Game/_Alex/HE_CharacterHrono1.HE_CharacterHrono1_C"));
	if (!TestNotNull(TEXT("Character class"), CharacterClass)) return false;
	if (const UFunction* Death = CharacterClass->FindFunctionByName(TEXT("OnDeath")))
		AddInfo(FString::Printf(TEXT("Blueprint OnDeath flags: 0x%08x"), static_cast<uint32>(Death->FunctionFlags)));
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AHronoCharacter* Player = Scope.World->SpawnActor<AHronoCharacter>(CharacterClass, FVector::ZeroVector, FRotator::ZeroRotator, Params);
	if (!TestNotNull(TEXT("Player"), Player)) return false;
	Player->CharacterTimeline = EItemTimeline::Past;
	const FVector Eye = Player->FindComponentByClass<UCameraComponent>()->GetComponentLocation();
	auto Prepare = [&](ABase_Item* Item)
	{
		Item->ItemTimeline = EItemTimeline::Both;
		Item->SetActorLocation(Eye + FVector(200,0,0));
		Item->ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Item->ItemMesh->SetCollisionResponseToAllChannels(ECR_Block);
	};
	ABase_Item* Item = Spawn(Scope.World);
	Prepare(Item);
	TestTrue(TEXT("Nearby visible item permitted"), Player->CanInteractWithActorOnServer(Item));
	Item->SetActorLocation(Eye + FVector(2000,0,0));
	TestFalse(TEXT("Distant item rejected"), Player->CanInteractWithActorOnServer(Item));
	Prepare(Item);
	Item->ItemTimeline = EItemTimeline::Future;
	TestFalse(TEXT("Other timeline rejected"), Player->CanInteractWithActorOnServer(Item));
	Item->ItemTimeline = EItemTimeline::Both;
	ABase_Item* Wall = Spawn(Scope.World);
	Prepare(Wall);
	Wall->SetActorLocation(Eye + FVector(100,0,0));
	TestFalse(TEXT("Occluded item rejected"), Player->CanInteractWithActorOnServer(Item));
	Wall->Destroy();
	Item->Destroy();

	ADrag_Item* Door = Spawn<ADrag_Item>(Scope.World);
	Prepare(Door);
	Door->bUseAutomaticOpenClose = false;
	Door->ItemMesh->SetRelativeRotation(FRotator::ZeroRotator);
	Player->Server_SetDoorPanelRotation(Door, Door->ItemMesh->GetFName(), FRotator(50,-999,30));
	TestTrue(TEXT("Door request clamps yaw and preserves pitch/roll"), Door->ItemMesh->GetRelativeRotation().Equals(FRotator(0,-90,0), 0.01));
	Door->bNeedKeyActor = true;
	Player->Server_SetDoorRotation(Door, FRotator::ZeroRotator);
	TestTrue(TEXT("Legacy door RPC obeys key lock"), FMath::IsNearlyEqual(Door->ItemMesh->GetRelativeRotation().Yaw, -90.0));
	Door->bNeedKeyActor = false;
	Door->RegisterDoorTriggerLock(true);
	Player->Server_SetDoorRotation(Door, FRotator::ZeroRotator);
	TestTrue(TEXT("Trigger lock blocks movement"), FMath::IsNearlyEqual(Door->ItemMesh->GetRelativeRotation().Yaw, -90.0));
	Door->RegisterDoorTriggerLock(false);
	Player->Server_SetDoorPanelRotation(Door, TEXT("FrameMesh"), FRotator::ZeroRotator);
	TestTrue(TEXT("Unconfigured panel rejected"), FMath::IsNearlyEqual(Door->ItemMesh->GetRelativeRotation().Yaw, -90.0));
	Player->Server_SetDoorRotation(Door, FRotator(0,std::numeric_limits<double>::quiet_NaN(),0));
	TestFalse(TEXT("NaN cannot enter door state"), Door->DoorRotation.ContainsNaN());
	Door->DragComponent->bUseCustomDoorAngleLimits = true;
	Door->DragComponent->MinimumDoorYaw = -30;
	Door->DragComponent->MaximumDoorYaw = 25;
	Player->Server_SetDoorRotation(Door, FRotator(0,70,0));
	TestTrue(TEXT("Authored angle limits respected"), FMath::IsNearlyEqual(Door->ItemMesh->GetRelativeRotation().Yaw,25.0));
	Door->DragComponent->bIsShelf = true;
	Door->DragComponent->ShelfClosedLocation = FVector::ZeroVector;
	Door->DragComponent->ShelfSlideAxis = FVector(1,0,0);
	Door->DragComponent->ShelfMaxDistance = 40;
	Player->Server_SetShelfPosition(Door, FVector(999,999,999));
	TestTrue(TEXT("Legacy shelf clamps axis and travel"), Door->ItemMesh->GetRelativeLocation().Equals(FVector(40,0,0),0.01));
	Door->bNeedKeyActor = true;
	Player->Server_SetShelfPanelPosition(Door, Door->ItemMesh->GetFName(), FVector::ZeroVector);
	TestTrue(TEXT("Shelf key lock blocks movement"), Door->ItemMesh->GetRelativeLocation().Equals(FVector(40,0,0),0.01));
	Door->Destroy();

	AHidingWardrobe* Wardrobe = Spawn<AHidingWardrobe>(Scope.World);
	Prepare(Wardrobe);
	Wardrobe->bUseAutomaticOpenClose = false;
	Wardrobe->RightDoorMesh->SetStaticMesh(Wardrobe->ItemMesh->GetStaticMesh());
	Wardrobe->RightDoorMesh->SetCollisionResponseToAllChannels(ECR_Block);
	Player->Server_SetDoorPanelRotation(Wardrobe, Wardrobe->RightDoorPivot->GetFName(), FRotator(15,-999,25));
	TestTrue(TEXT("Wardrobe right pivot uses its own angle limits"), Wardrobe->RightDoorPivot->GetRelativeRotation().Equals(FRotator(0,-110,0),0.01));
	TestTrue(TEXT("Wardrobe other panel stays unchanged"), Wardrobe->LeftDoorPivot->GetRelativeRotation().IsNearlyZero());
	Wardrobe->Destroy();

	AThreeDrawerCabinet* Cabinet = Spawn<AThreeDrawerCabinet>(Scope.World);
	Prepare(Cabinet);
	Cabinet->bUseAutomaticOpenClose = false;
	Cabinet->MiddleDrawerMesh->SetStaticMesh(Cabinet->ItemMesh->GetStaticMesh());
	Cabinet->MiddleDrawerMesh->SetCollisionResponseToAllChannels(ECR_Block);
	Cabinet->MiddleDrawerDragComponent->ShelfClosedLocation = Cabinet->MiddleDrawerMesh->GetRelativeLocation();
	const FVector MiddleClosed = Cabinet->MiddleDrawerDragComponent->ShelfClosedLocation;
	const FVector BottomBefore = Cabinet->BottomDrawerMesh->GetRelativeLocation();
	const FVector TopBefore = Cabinet->TopDrawerMesh->GetRelativeLocation();
	Cabinet->MiddleDrawerDragComponent->ShelfSlideAxis = FVector(1,0,0);
	Cabinet->MiddleDrawerDragComponent->ShelfMaxDistance = 35;
	Player->Server_SetShelfPanelPosition(Cabinet, Cabinet->MiddleDrawerMesh->GetFName(), MiddleClosed + FVector(999,500,500));
	TestTrue(TEXT("Named middle drawer uses authored travel"), Cabinet->MiddleDrawerMesh->GetRelativeLocation().Equals(MiddleClosed+FVector(35,0,0),0.01));
	TestTrue(TEXT("Other drawers unchanged"), Cabinet->BottomDrawerMesh->GetRelativeLocation().Equals(BottomBefore) && Cabinet->TopDrawerMesh->GetRelativeLocation().Equals(TopBefore));
	Cabinet->Destroy();

	ADrag_Item* LockedDoor = Spawn<ADrag_Item>(Scope.World);
	Prepare(LockedDoor);
	LockedDoor->SetActorLocation(Eye + FVector(0,200,0));
	LockedDoor->bNeedKeyActor = true;
	ABase_Item* Key = Spawn(Scope.World);
	Prepare(Key);
	Key->ItemTags.AddTag(LockedDoor->GetRequiredKeyTag());
	struct { ABase_Item* Item; } KeyPickup{Key};
	Player->ProcessEvent(Player->FindFunctionChecked(TEXT("ServerPickupItem")), &KeyPickup);
	if (!TestTrue(TEXT("Matching key held"), Player->GetHeldItem() == Key)) return false;
	struct { ADrag_Item* Item; } UnlockArgs{LockedDoor};
	auto Unlock = [&]() { Player->ProcessEvent(Player->FindFunctionChecked(TEXT("ServerUnlockWithHeldKey")), &UnlockArgs); };
	Key->OwningCharacter = nullptr;
	Unlock();
	TestTrue(TEXT("Stale key owner cannot unlock"), LockedDoor->bNeedKeyActor && !Key->IsActorBeingDestroyed());
	Key->OwningCharacter = Player;
	LockedDoor->RegisterDoorBarricade(EItemTimeline::Past, true);
	Unlock();
	TestTrue(TEXT("Barricade prevents consuming key"), LockedDoor->bNeedKeyActor && Player->GetHeldItem() == Key);
	LockedDoor->RegisterDoorBarricade(EItemTimeline::Past, false);
	Unlock();
	TestFalse(TEXT("Legitimate held key unlocks"), LockedDoor->bNeedKeyActor);
	TestTrue(TEXT("Successful unlock consumes key once"), Key->IsActorBeingDestroyed() && Player->GetHeldItem() == nullptr);
	Unlock();
	TestFalse(TEXT("Repeated unlock is harmless"), LockedDoor->bNeedKeyActor);
	LockedDoor->Destroy();

	ARune_Item* Rune = Spawn<ARune_Item>(Scope.World);
	Prepare(Rune);
	Rune->RuneId = TEXT("Rune.One");
	struct { ABase_Item* Item; } PickupArgs{Rune};
	Player->ProcessEvent(Player->FindFunctionChecked(TEXT("ServerPickupItem")), &PickupArgs);
	if (!TestTrue(TEXT("Rune legitimately held"), Player->GetHeldItem() == Rune)) return false;
	ARunePentagram* Pentagram = Scope.World->SpawnActor<ARunePentagram>();
	Pentagram->SetActorLocation(Eye + FVector(200,0,0));
	struct FPlacementArgs { AActor* Pentagram; FName Slot; FName Required; FTransform Transform; int32 Count; } Args{
		Pentagram, TEXT("Invented.Slot"), TEXT("Invented.Rule"), FTransform(FVector(99999,99999,99999)), 1};
	auto Request = [&]() { Rune->ProcessEvent(Rune->FindFunctionChecked(TEXT("ServerPlaceRuneInPentagram")), &Args); };
	Request();
	TestFalse(TEXT("Invented slot rejected"), Rune->bPlacedInPentagram);
	Args.Slot = TEXT("Pentagram.Slot.One");
	Pentagram->SetActorLocation(Eye + FVector(2000,0,0));
	Request();
	TestFalse(TEXT("Remote pentagram rejected"), Rune->bPlacedInPentagram);
	Pentagram->SetActorLocation(Eye + FVector(200,0,0));
	Request();
	TestTrue(TEXT("Valid slot request places held rune"), Rune->bPlacedInPentagram);
	TestTrue(TEXT("Server slot transform overrides spoofed transform"), Rune->GetActorTransform().Equals(Pentagram->RuneSlotOne->GetComponentTransform(),0.01));
	TestEqual(TEXT("Coordinator updates progress"), Pentagram->GetPlacedRuneCount(),1);
	TestFalse(TEXT("Spoofed count cannot complete ritual"), Pentagram->bPentagramCompleted);
	TestNull(TEXT("Placement releases hand"), Player->GetHeldItem());
	TestTrue(TEXT("Transition immediately before ritual accepted"), Player->TrySetPlayerTimelineOnAuthority(EItemTimeline::Future));
	TestTrue(TEXT("Immediate independent return to Past accepted"), Player->TrySetPlayerTimelineOnAuthority(EItemTimeline::Past));
	for (FName NextId : {FName(TEXT("Rune.Two")), FName(TEXT("Rune.Three"))})
	{
		ARune_Item* NextRune = Spawn<ARune_Item>(Scope.World);
		Prepare(NextRune);
		NextRune->SetActorLocation(Eye + FVector(0,200,0));
		NextRune->RuneId = NextId;
		PickupArgs.Item = NextRune;
		Player->ProcessEvent(Player->FindFunctionChecked(TEXT("ServerPickupItem")), &PickupArgs);
		TestTrue(TEXT("Next ritual rune held"), Player->GetHeldItem() == NextRune);
		struct { AActor* Target; } InteractArgs{Pentagram};
		Player->ProcessEvent(Player->FindFunctionChecked(TEXT("Server_InteractWithEnvironment")), &InteractArgs);
		TestTrue(TEXT("Normal environment request places next rune"), NextRune->bPlacedInPentagram);
	}
	TestTrue(TEXT("Normal third rune completes pentagram"), Pentagram->bPentagramCompleted);
	TestEqual(TEXT("Third rune assigns completing player"), Pentagram->CompletingPlayer.Get(), Player);
	TestTrue(TEXT("Authoritative ritual changes timeline"), Player->GetTimeline() == EItemTimeline::Future);
	const EItemTimeline Before = Player->GetTimeline();
	Player->SetPlayerTimeline(static_cast<EItemTimeline>(255));
	TestTrue(TEXT("Invalid enum rejected"), Player->GetTimeline() == Before);
	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTimelineAuthorityTest,
	"Hrono.Items.TimelineAuthority", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTimelineAuthorityTest::RunTest(const FString& Parameters)
{
	using namespace ItemWorldTests;
	FWorldScope Scope(false);
	UClass* CharacterClass = LoadClass<AHronoCharacter>(nullptr,
		TEXT("/Game/_Alex/HE_CharacterHrono1.HE_CharacterHrono1_C"));
	if (!TestNotNull(TEXT("Character class"), CharacterClass)) return false;
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AHronoCharacter* Player = Scope.World->SpawnActor<AHronoCharacter>(CharacterClass, FVector::ZeroVector, FRotator::ZeroRotator, Params);
	if (!TestNotNull(TEXT("Player"), Player)) return false;
	Player->CharacterTimeline = EItemTimeline::Past;
	struct { EItemTimeline Timeline; } Args{EItemTimeline::Future};
	Player->ProcessEvent(Player->FindFunctionChecked(TEXT("ServerSetPlayerTimeline")), &Args);
	TestTrue(TEXT("Legacy client RPC cannot select Future"), Player->GetTimeline() == EItemTimeline::Past);
	TestFalse(TEXT("Completion without a server death rejected"), Player->CompleteDeathTimelineTransition());
	USkeletalMeshComponent* DeathMesh = Player->GetMesh();
	DeathMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	TestTrue(TEXT("Authority starts death"), Player->BeginDeathTimelineTransition(DeathMesh));
	TestTrue(TEXT("Death captures original timeline"), Player->GetDeathOriginalTimeline() == EItemTimeline::Past);
	TestTrue(TEXT("Server montage ticks without rendering"), DeathMesh->VisibilityBasedAnimTickOption == EVisibilityBasedAnimTickOption::AlwaysTickPose);
	TestFalse(TEXT("Duplicate death cannot restart presentation"), Player->BeginDeathTimelineTransition(DeathMesh));
	Player->ProcessEvent(Player->FindFunctionChecked(TEXT("ServerSetPlayerTimeline")), &Args);
	TestTrue(TEXT("RPC cannot prematurely finish pending death"), Player->GetTimeline() == EItemTimeline::Past);
	TestTrue(TEXT("Authority completes captured transition"), Player->CompleteDeathTimelineTransition());
	TestTrue(TEXT("Death reaches expected Future"), Player->GetTimeline() == EItemTimeline::Future);
	TestTrue(TEXT("Death restores authored animation ticking"), DeathMesh->VisibilityBasedAnimTickOption == EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered);
	TestFalse(TEXT("Duplicate completion does not toggle back"), Player->CompleteDeathTimelineTransition());
	TestTrue(TEXT("Death metadata keeps pre-transition timeline"), Player->GetDeathOriginalTimeline() == EItemTimeline::Past);
	TestTrue(TEXT("Independent rapid authority transition accepted"), Player->TrySetPlayerTimelineOnAuthority(EItemTimeline::Past));
	TestTrue(TEXT("No time guard rejects legitimate transition"), Player->GetTimeline() == EItemTimeline::Past);
	TestTrue(TEXT("Next death can begin"), Player->BeginDeathTimelineTransition(DeathMesh));
	Player->CancelDeathTimelineTransition();
	TestFalse(TEXT("Interrupted death cannot complete"), Player->CompleteDeathTimelineTransition());
	TestTrue(TEXT("Interruption keeps original timeline"), Player->GetTimeline() == EItemTimeline::Past);
	TestTrue(TEXT("Interruption restores montage ticking"), DeathMesh->VisibilityBasedAnimTickOption == EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered);
	TestTrue(TEXT("Begin death before an independent ritual"), Player->BeginDeathTimelineTransition(DeathMesh));
	TestTrue(TEXT("Independent ritual supersedes death"), Player->TrySetPlayerTimelineOnAuthority(EItemTimeline::Future));
	TestFalse(TEXT("Stale montage cannot supersede ritual"), Player->CompleteDeathTimelineTransition());
	TestTrue(TEXT("Ritual result preserved"), Player->GetTimeline() == EItemTimeline::Future);
	Player->SetRole(ROLE_AutonomousProxy);
	Player->SetPlayerTimeline(EItemTimeline::Past);
	Player->SwitchPlayerTimeline();
	TestTrue(TEXT("Local client APIs do not mutate timeline"), Player->GetTimeline() == EItemTimeline::Future);
	TestTrue(TEXT("Client may play death presentation"), Player->BeginDeathTimelineTransition(DeathMesh));
	TestFalse(TEXT("Client cannot complete a death transition"), Player->CompleteDeathTimelineTransition());
	TestFalse(TEXT("Client cannot use trusted authority API"), Player->TrySetPlayerTimelineOnAuthority(EItemTimeline::Past));
	Player->SetRole(ROLE_Authority);
	TestFalse(TEXT("Invalid requested timeline rejected"), Player->TrySetPlayerTimelineOnAuthority(static_cast<EItemTimeline>(255)));
	TestFalse(TEXT("Both rejected for player"), Player->TrySetPlayerTimelineOnAuthority(EItemTimeline::Both));
	ARunePentagram* Deferred = Scope.World->SpawnActor<ARunePentagram>();
	TObjectPtr<ARune_Item>* Occupied[] = {&Deferred->PlacedRuneOne, &Deferred->PlacedRuneTwo, &Deferred->PlacedRuneThree};
	for (int32 Index = 0; Index < 3; ++Index)
	{
		ARune_Item* Placed = Spawn<ARune_Item>(Scope.World);
		Placed->RuneId = FName(*FString::Printf(TEXT("Test.Rune.%d"),Index));
		Placed->PlaceRuneInPentagram(Deferred, FName(*FString::Printf(TEXT("Test.Slot.%d"),Index)), Placed->RuneId, FTransform::Identity, 3);
		*Occupied[Index] = Placed;
	}
	Player->SetRole(ROLE_AutonomousProxy);
	TestFalse(TEXT("Ritual cannot commit a rejected transition"), Deferred->TryCompletePentagram(Player));
	TestFalse(TEXT("Rejected transition leaves pentagram incomplete"), Deferred->bPentagramCompleted);
	TestNull(TEXT("Rejected transition does not publish completing player"), Deferred->CompletingPlayer.Get());
	Player->SetRole(ROLE_Authority);
	TestTrue(TEXT("Another event may already reach pending ritual target"), Player->TrySetPlayerTimelineOnAuthority(EItemTimeline::Past));
	Deferred->bShowDebugStatusOnScreen = false;
	static_cast<AActor*>(Deferred)->Tick(0.1f);
	TestTrue(TEXT("Deferred ritual retries without debug tick presentation"), Deferred->bPentagramCompleted);
	TestTrue(TEXT("Retry preserves captured target rather than toggling again"), Player->GetTimeline() == EItemTimeline::Past && Deferred->CompletingPlayerNewTimeline == EItemTimeline::Past);
	TestEqual(TEXT("Retry retains original third-rune player"), Deferred->CompletingPlayer.Get(), Player);
	TestFalse(TEXT("Repeated completion cannot fire twice"), Deferred->TryCompletePentagram(Player));

	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWardrobeSafetySourcesTest,
	"Hrono.Items.WardrobeSafetySources", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWardrobeSafetySourcesTest::RunTest(const FString& Parameters)
{
	using namespace ItemWorldTests;
	FWorldScope Scope;
	UClass* CharacterClass = LoadClass<AHronoCharacter>(nullptr,
		TEXT("/Game/_Alex/HE_CharacterHrono1.HE_CharacterHrono1_C"));
	if (!TestNotNull(TEXT("Character class"), CharacterClass)) return false;
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AHronoCharacter* Player = Scope.World->SpawnActor<AHronoCharacter>(CharacterClass, FVector(0,0,200), FRotator::ZeroRotator, Params);
	if (!TestNotNull(TEXT("Player"), Player)) return false;
	Player->CharacterTimeline = EItemTimeline::Past;
	Player->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Player->GetCapsuleComponent()->SetGenerateOverlapEvents(true);
	Player->GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Overlap);
	UWardrobeSafetyTestObserver* Observer = NewObject<UWardrobeSafetyTestObserver>(Player);
	Player->OnHidingSafetyChanged.AddDynamic(Observer, &UWardrobeSafetyTestObserver::OnSafetyChanged);
	Player->OnHidingWardrobeSafetyLost.AddDynamic(Observer, &UWardrobeSafetyTestObserver::OnSafetyLost);
	auto Wardrobe = [&](const FVector& Location, EItemTimeline Timeline)
	{
		AHidingWardrobe* Source = Scope.World->SpawnActor<AHidingWardrobe>(Location, FRotator::ZeroRotator, Params);
		Source->ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Source->RightDoorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Source->FrameMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Source->SafetyVolume->SetBoxExtent(FVector(120,120,150));
		Source->SafetyVolume->SetGenerateOverlapEvents(true);
		Source->SetItemTimeline(Timeline);
		Source->SafetyVolume->UpdateOverlaps();
		return Source;
	};
	AHidingWardrobe* Past = Wardrobe(FVector(-60,0,200), EItemTimeline::Past);
	AHidingWardrobe* Future = Wardrobe(FVector(60,0,200), EItemTimeline::Future);
	Player->GetCapsuleComponent()->UpdateOverlaps();
	TestTrue(TEXT("Real capsule overlaps both wardrobe volumes"), Past->IsCharacterInsideSafetyVolume(Player) && Future->IsCharacterInsideSafetyVolume(Player));
	TestTrue(TEXT("Matching Past wardrobe provides safety"), Past->CanProvideSafetyFor(Player));
	TestFalse(TEXT("Other timeline wardrobe cannot protect"), Future->CanProvideSafetyFor(Player));
	if (!TestTrue(TEXT("Real begin-overlap registers safety"), Player->IsSafeInHidingWardrobe())) return false;
	Past->SetItemTimeline(EItemTimeline::Future);
	TestFalse(TEXT("Wardrobe timeline change removes safety immediately"), Player->IsSafeInHidingWardrobe());
	TestTrue(TEXT("Character may change to Future inside volume"), Player->TrySetPlayerTimelineOnAuthority(EItemTimeline::Future));
	TestTrue(TEXT("Previously unsafe overlaps protect without re-entry"), Player->IsSafeInHidingWardrobe());
	Past->SetItemTimeline(EItemTimeline::Both);
	Player->TrySetPlayerTimelineOnAuthority(EItemTimeline::Past);
	TestTrue(TEXT("Both wardrobe protects either timeline"), Player->IsSafeInHidingWardrobe());
	Past->ApplyDoorRotationFromServer(Past->LeftDoorPivot->GetFName(), FRotator(0,5,0));
	TestFalse(TEXT("Exactly unsafe angle removes last protection"), Player->IsSafeInHidingWardrobe());
	Past->ApplyDoorRotationFromServer(Past->LeftDoorPivot->GetFName(), FRotator(0,4.9,0));
	TestTrue(TEXT("Below unsafe angle restores protection"), Player->IsSafeInHidingWardrobe());
	Future->SetItemTimeline(EItemTimeline::Both);
	const int32 LossesBeforeOtherSourceOpens = Observer->LostCount;
	Past->ApplyDoorRotationFromServer(Past->RightDoorPivot->GetFName(), FRotator(0,-20,0));
	TestTrue(TEXT("Opening one source cannot clear the other"), Player->IsSafeInHidingWardrobe());
	TestEqual(TEXT("Opening one source emits no false loss"), Observer->LostCount, LossesBeforeOtherSourceOpens);
	Future->bAllowHiding = false;
	Future->Tick(0.016f);
	TestFalse(TEXT("Disabled last source cannot protect"), Player->IsSafeInHidingWardrobe());
	Future->bAllowHiding = true;
	Future->Tick(0.016f);
	TestTrue(TEXT("Enabled source restores protection"), Player->IsSafeInHidingWardrobe());
	Past->ApplyDoorRotationFromServer(Past->RightDoorPivot->GetFName(), FRotator::ZeroRotator);
	Past->Destroy();
	TestTrue(TEXT("Destroying one source preserves another"), Player->IsSafeInHidingWardrobe());
	const int32 LossesBeforeLastDestroy = Observer->LostCount;
	Future->Destroy();
	TestFalse(TEXT("Destroying the final source removes protection"), Player->IsSafeInHidingWardrobe());
	TestEqual(TEXT("Last source destruction emits exactly one loss"), Observer->LostCount, LossesBeforeLastDestroy + 1);
	Player->SetSafeInHidingWardrobe(true);
	TestFalse(TEXT("Legacy bare bool cannot invent protection"), Player->IsSafeInHidingWardrobe());
	AHidingWardrobe* OutOfRange = Wardrobe(FVector(1000,0,200), EItemTimeline::Both);
	Player->UpdateWardrobeSafetySource(OutOfRange, true);
	TestFalse(TEXT("Registration cannot invent a physical overlap"), Player->IsSafeInHidingWardrobe());
	AHidingWardrobe* A = Wardrobe(FVector(-60,0,200), EItemTimeline::Both);
	AHidingWardrobe* B = Wardrobe(FVector(60,0,200), EItemTimeline::Both);
	Player->SetActorLocation(FVector(180,0,200));
	Player->GetCapsuleComponent()->UpdateOverlaps();
	TestFalse(TEXT("Actual exit from A"), A->IsCharacterInsideSafetyVolume(Player));
	TestTrue(TEXT("Still physically inside B"), B->IsCharacterInsideSafetyVolume(Player));
	TestTrue(TEXT("EndOverlap A preserves B protection"), Player->IsSafeInHidingWardrobe());
	Player->SetSafeInHidingWardrobe(false);
	TestTrue(TEXT("Legacy false cannot override registered valid source"), Player->IsSafeInHidingWardrobe());
	Player->SetActorLocation(FVector(450,0,200));
	Player->GetCapsuleComponent()->UpdateOverlaps();
	TestFalse(TEXT("Exit last physical volume removes protection"), Player->IsSafeInHidingWardrobe());
	Player->UpdateWardrobeSafetySource(nullptr, true);
	TestFalse(TEXT("Null registration is harmless"), Player->IsSafeInHidingWardrobe());
	A->Destroy();
	B->Destroy();
	OutOfRange->Destroy();

	// Two timeline sources must transfer protection without a false hazard event.
	Player->SetActorLocation(FVector(0,0,200));
	AHidingWardrobe* SwapPast = Wardrobe(FVector(-60,0,200), EItemTimeline::Past);
	AHidingWardrobe* SwapFuture = Wardrobe(FVector(60,0,200), EItemTimeline::Future);
	const int32 ChangesBeforeSwap = Observer->Changes.Num();
	Player->TrySetPlayerTimelineOnAuthority(EItemTimeline::Future);
	TestTrue(TEXT("Timeline swap transfers to the other valid source"), Player->IsSafeInHidingWardrobe());
	TestEqual(TEXT("Timeline swap does not pulse safety events"), Observer->Changes.Num(), ChangesBeforeSwap);
	Player->TrySetPlayerTimelineOnAuthority(EItemTimeline::Past);
	SwapFuture->Destroy();
	Observer->bBeginDeathOnLoss = true;
	Player->TrySetPlayerTimelineOnAuthority(EItemTimeline::Future);
	TestTrue(TEXT("Safety loss callback can start death after timeline guard releases"), Observer->bDeathAccepted);
	TestTrue(TEXT("Death callback captures the committed new timeline"), Player->GetDeathOriginalTimeline() == EItemTimeline::Future);
	Observer->bBeginDeathOnLoss = false;
	Player->CancelDeathTimelineTransition();
	SwapPast->Destroy();
	Player->TrySetPlayerTimelineOnAuthority(EItemTimeline::Past);

	// A callback can remove the source being refreshed.
	AHidingWardrobe* Reentrant = Wardrobe(FVector(1000,0,200), EItemTimeline::Both);
	Observer->DestroyOnGain = Reentrant;
	const int32 ChangesBeforeReentrant = Observer->Changes.Num();
	const int32 LossesBeforeReentrant = Observer->LostCount;
	Player->SetActorLocation(FVector(1000,0,200));
	TestFalse(TEXT("Reentrant source destruction settles to unsafe"), Player->IsSafeInHidingWardrobe());
	TestEqual(TEXT("Reentrant source dispatches gain then loss"), Observer->Changes.Num(), ChangesBeforeReentrant + 2);
	TestEqual(TEXT("Reentrant source emits exactly one loss"), Observer->LostCount, LossesBeforeReentrant + 1);

	AHidingWardrobe* Hiding = Wardrobe(FVector(0,0,200), EItemTimeline::Past);
	Hiding->HidingPoint->SetRelativeLocation(FVector::ZeroVector);
	Hiding->ExitPoint->SetRelativeLocation(FVector(350,0,0));
	Hiding->bRequireBothDoorsOpenToHide = false;
	Player->SetActorLocation(FVector(300,0,200));
	if (!TestTrue(TEXT("Enter matching wardrobe through normal API"), Hiding->TryEnterWardrobe(Player))) return false;
	TestTrue(TEXT("Hiding disables movement"), Player->GetCharacterMovement()->MovementMode == MOVE_None);
	TestTrue(TEXT("Hidden capsule remains detectable"), Hiding->IsCharacterInsideSafetyVolume(Player));
	Player->TrySetPlayerTimelineOnAuthority(EItemTimeline::Future);
	TestFalse(TEXT("Timeline switch cannot retain old safety"), Player->IsSafeInHidingWardrobe());
	Hiding->Tick(0.016f);
	TestNull(TEXT("Mismatched hidden occupant exits instead of remaining locked"), Hiding->HiddenPlayer.Get());
	TestTrue(TEXT("Timeline mismatch restores walking"), Player->GetCharacterMovement()->MovementMode == MOVE_Walking);
	TestTrue(TEXT("Exit uses authored point"), Player->GetActorLocation().Equals(Hiding->ExitPoint->GetComponentLocation()));
	TestEqual(TEXT("Exit after transition ignores old timeline doors"), Player->GetCapsuleComponent()->GetCollisionResponseToChannel(COLLISION_CHANNEL_DOOR_PAST), ECR_Ignore);
	TestEqual(TEXT("Exit after transition blocks current timeline doors"), Player->GetCapsuleComponent()->GetCollisionResponseToChannel(COLLISION_CHANNEL_DOOR_FUTURE), ECR_Block);
	Hiding->Destroy();
	AHidingWardrobe* Shared = Wardrobe(FVector(0,0,200), EItemTimeline::Both);
	Shared->bRequireBothDoorsOpenToHide = false;
	Shared->ExitPoint->SetRelativeLocation(FVector(350,0,0));
	TestTrue(TEXT("Enter Both wardrobe"), Shared->TryEnterWardrobe(Player));
	Player->TrySetPlayerTimelineOnAuthority(EItemTimeline::Past);
	Shared->Tick(0.016f);
	TestTrue(TEXT("Both wardrobe retains its occupant after transition"), Shared->HiddenPlayer == Player);
	TestTrue(TEXT("Both wardrobe retains protection after transition"), Player->IsSafeInHidingWardrobe());
	TestTrue(TEXT("Normal exit from Both wardrobe succeeds"), Shared->ExitWardrobe(Player));
	TestTrue(TEXT("Normal exit restores walking"), Player->GetCharacterMovement()->MovementMode == MOVE_Walking);
	TestEqual(TEXT("Both exit restores current Past door collision"), Player->GetCapsuleComponent()->GetCollisionResponseToChannel(COLLISION_CHANNEL_DOOR_PAST), ECR_Block);
	TestEqual(TEXT("Both exit ignores previous Future doors"), Player->GetCapsuleComponent()->GetCollisionResponseToChannel(COLLISION_CHANNEL_DOOR_FUTURE), ECR_Ignore);
	TestFalse(TEXT("Normal exit clears final protection"), Player->IsSafeInHidingWardrobe());
	Player->SetRole(ROLE_AutonomousProxy);
	Player->UpdateWardrobeSafetySource(Shared, true);
	Player->SetSafeInHidingWardrobe(true);
	TestFalse(TEXT("Client cannot grant safety"), Player->IsSafeInHidingWardrobe());
	Player->SetRole(ROLE_Authority);
	Player->OnHidingSafetyChanged.RemoveDynamic(Observer, &UWardrobeSafetyTestObserver::OnSafetyChanged);
	Player->OnHidingWardrobeSafetyLost.RemoveDynamic(Observer, &UWardrobeSafetyTestObserver::OnSafetyLost);
	return true;
}

#endif
