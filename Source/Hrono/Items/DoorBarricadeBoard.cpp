#include "Items/DoorBarricadeBoard.h"

#include "HronoCharacter.h"
#include "HronoCollisionChannels.h"
#include "Items/AxeItem.h"
#include "Items/Drag_Item.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "GeometryCollection/GeometryCollectionObject.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundBase.h"

ADoorBarricadeBoard::ADoorBarricadeBoard()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	BoardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoardMesh"));
	BoardMesh->SetupAttachment(SceneRoot);
	BoardMesh->SetCollisionObjectType(ECC_WorldDynamic);
	BoardMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BoardMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	BoardMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	BoardMesh->SetGenerateOverlapEvents(false);

	DestructibleBoard = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("DestructibleBoard"));
	DestructibleBoard->SetupAttachment(SceneRoot);
	DestructibleBoard->SetVisibility(false, true);
	DestructibleBoard->SetHiddenInGame(true, true);
	DestructibleBoard->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DestructibleBoard->SetGenerateOverlapEvents(false);
	DestructibleBoard->SetSimulatePhysics(false);

	static ConstructorHelpers::FObjectFinder<UGeometryCollection> BoardCollection(
		TEXT("/Game/_Alex/ChaosDestruction/GC_BP_DoorBarricadeBoard.GC_BP_DoorBarricadeBoard"));
	if (BoardCollection.Succeeded())
	{
		DefaultBoardCollection = BoardCollection.Object;
	}
}

void ADoorBarricadeBoard::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (DestructibleBoard && DefaultBoardCollection && !DestructibleBoard->GetRestCollection())
	{
		DestructibleBoard->SetRestCollection(DefaultBoardCollection);
	}
}

void ADoorBarricadeBoard::BeginPlay()
{
	Super::BeginPlay();

	// Keep the authored fractured mesh aligned with the intact board even when
	// the Blueprint child adjusts the native BoardMesh transform or scale.
	if (BoardMesh && DestructibleBoard)
	{
		DestructibleBoard->SetRelativeTransform(BoardMesh->GetRelativeTransform());
	}

	ApplyTimelineCollision();
	UpdateLocalVisibility();

	if (HasAuthority())
	{
		if (!IsValid(BlockedDoor))
		{
			BlockedDoor = FindNearestDoor();
		}
		RegisterWithDoor();
		ForceNetUpdate();

		if (!IsValid(BlockedDoor))
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[DoorBarricade] %s found no draggable door within %.0f cm"),
				*GetName(), AutoFindDoorRadius);
		}
	}
}

void ADoorBarricadeBoard::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		UnregisterFromDoor();
	}

	Super::EndPlay(EndPlayReason);
}

void ADoorBarricadeBoard::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateLocalVisibility();
}

void ADoorBarricadeBoard::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADoorBarricadeBoard, BoardTimeline);
	DOREPLIFETIME(ADoorBarricadeBoard, BlockedDoor);
	DOREPLIFETIME(ADoorBarricadeBoard, bBroken);
}

void ADoorBarricadeBoard::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority() || bBroken)
	{
		return;
	}

	AHronoCharacter* Character = Cast<AHronoCharacter>(Interactor);
	AAxeItem* Axe = Character ? Cast<AAxeItem>(Character->GetHeldItem()) : nullptr;
	if (!IsValid(Character)
		|| !IsValid(Axe)
		|| !Axe->CanBreakBarricades()
		|| !DoesTimelineMatch(Character->GetTimeline())
		|| FVector::DistSquared(Character->GetActorLocation(), GetActorLocation())
			> FMath::Square(BreakDistance))
	{
		return;
	}

	// The axe owns the swing cadence. Refuse the interaction while it is still
	// returning to its held pose (or during its short recovery), so repeated
	// interaction RPCs cannot break boards faster than the animation allows.
	if (!Axe->TryStartSwing())
	{
		return;
	}

	bBroken = true;
	ADrag_Item* DoorThatWasBlocked = BlockedDoor;
	UnregisterFromDoor();
	MulticastBoardBroken(Character, DoorThatWasBlocked);
	ForceNetUpdate();
	if (DebrisLifetime > 0.0f)
	{
		SetLifeSpan(DebrisLifetime);
	}
}

void ADoorBarricadeBoard::SetBlockedDoor(ADrag_Item* NewDoor)
{
	if (!HasAuthority() || bBroken || BlockedDoor == NewDoor)
	{
		return;
	}

	UnregisterFromDoor();
	BlockedDoor = IsValid(NewDoor) ? NewDoor : FindNearestDoor();
	RegisterWithDoor();
	ForceNetUpdate();
}

void ADoorBarricadeBoard::OnRep_BoardTimeline()
{
	ApplyTimelineCollision();
	UpdateLocalVisibility();
}

void ADoorBarricadeBoard::OnRep_BlockedDoor()
{
	// Door block counts replicate from the door itself. This notify exists so a
	// Blueprint can immediately observe the resolved auto-found door.
}

void ADoorBarricadeBoard::OnRep_Broken()
{
	if (bBroken)
	{
		ActivateChaosDestruction();
	}
}

void ADoorBarricadeBoard::MulticastBoardBroken_Implementation(
	AHronoCharacter* BreakingCharacter,
	ADrag_Item* DoorThatWasBlocked)
{
	ActivateChaosDestruction();

	APlayerController* LocalController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	const AHronoCharacter* LocalCharacter = LocalController
		? Cast<AHronoCharacter>(LocalController->GetPawn())
		: nullptr;
	if (BreakSound && LocalCharacter && DoesTimelineMatch(LocalCharacter->GetTimeline()))
	{
		UGameplayStatics::PlaySoundAtLocation(this, BreakSound, GetActorLocation());
	}

	OnBarricadeBroken.Broadcast(BreakingCharacter, DoorThatWasBlocked);
}

ADrag_Item* ADoorBarricadeBoard::FindNearestDoor() const
{
	if (!GetWorld() || AutoFindDoorRadius <= 0.0f)
	{
		return nullptr;
	}

	ADrag_Item* BestDoor = nullptr;
	float BestDistanceSquared = FMath::Square(AutoFindDoorRadius);
	for (TActorIterator<ADrag_Item> It(GetWorld()); It; ++It)
	{
		ADrag_Item* Candidate = *It;
		if (!IsValid(Candidate)
			|| !IsValid(Candidate->DragComponent)
			|| Candidate->DragComponent->bIsShelf
			|| Candidate->DragComponent->bIsCupBoard
			|| (BoardTimeline != EItemTimeline::Both
				&& Candidate->ItemTimeline != EItemTimeline::Both
				&& Candidate->ItemTimeline != BoardTimeline))
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(GetActorLocation(), Candidate->GetActorLocation());
		if (DistanceSquared <= BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			BestDoor = Candidate;
		}
	}

	return BestDoor;
}

void ADoorBarricadeBoard::RegisterWithDoor()
{
	if (!HasAuthority() || bRegisteredWithDoor || bBroken || !IsValid(BlockedDoor))
	{
		return;
	}

	BlockedDoor->RegisterDoorBarricade(BoardTimeline, true);
	bRegisteredWithDoor = true;
}

void ADoorBarricadeBoard::UnregisterFromDoor()
{
	if (!HasAuthority() || !bRegisteredWithDoor)
	{
		return;
	}

	if (IsValid(BlockedDoor))
	{
		BlockedDoor->RegisterDoorBarricade(BoardTimeline, false);
	}
	bRegisteredWithDoor = false;
}

void ADoorBarricadeBoard::ApplyTimelineCollision()
{
	if (!BoardMesh)
	{
		return;
	}

	const bool bPast = BoardTimeline == EItemTimeline::Past || BoardTimeline == EItemTimeline::Both;
	const bool bFuture = BoardTimeline == EItemTimeline::Future || BoardTimeline == EItemTimeline::Both;
	BoardMesh->SetCollisionEnabled(bBroken
		? ECollisionEnabled::NoCollision
		: ECollisionEnabled::QueryAndPhysics);
	BoardMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	BoardMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	BoardMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, bPast ? ECR_Block : ECR_Ignore);
	BoardMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, bFuture ? ECR_Block : ECR_Ignore);

	if (DestructibleBoard && bChaosDestructionActivated)
	{
		DestructibleBoard->SetCollisionResponseToAllChannels(ECR_Ignore);
		DestructibleBoard->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		DestructibleBoard->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
		DestructibleBoard->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, bPast ? ECR_Block : ECR_Ignore);
		DestructibleBoard->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, bFuture ? ECR_Block : ECR_Ignore);
	}
}

void ADoorBarricadeBoard::UpdateLocalVisibility()
{
	if (!GetWorld())
	{
		return;
	}

	APlayerController* LocalController = GetWorld()->GetFirstPlayerController();
	const AHronoCharacter* LocalCharacter = LocalController
		? Cast<AHronoCharacter>(LocalController->GetPawn())
		: nullptr;
	if (LocalCharacter)
	{
		const bool bVisibleInLocalTimeline = DoesTimelineMatch(LocalCharacter->GetTimeline());
		if (BoardMesh)
		{
			BoardMesh->SetVisibility(!bBroken && bVisibleInLocalTimeline, true);
		}
		if (DestructibleBoard)
		{
			const bool bShowDebris = bBroken && bChaosDestructionActivated && bVisibleInLocalTimeline;
			DestructibleBoard->SetVisibility(bShowDebris, true);
			DestructibleBoard->SetHiddenInGame(!bShowDebris, true);
		}
	}
}

void ADoorBarricadeBoard::ActivateChaosDestruction()
{
	if (!bBroken)
	{
		return;
	}

	if (BoardMesh)
	{
		BoardMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		BoardMesh->SetVisibility(false, true);
	}

	if (!DestructibleBoard || !DestructibleBoard->GetRestCollection())
	{
		return;
	}

	if (!bChaosDestructionActivated)
	{
		bChaosDestructionActivated = true;
		DestructibleBoard->SetHiddenInGame(false, true);
		DestructibleBoard->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		DestructibleBoard->SetSimulatePhysics(true);
		ApplyTimelineCollision();
		DestructibleBoard->CrumbleActiveClusters();
		UpdateLocalVisibility();
	}
}

bool ADoorBarricadeBoard::DoesTimelineMatch(EItemTimeline OtherTimeline) const
{
	return BoardTimeline == EItemTimeline::Both
		|| OtherTimeline == EItemTimeline::Both
		|| BoardTimeline == OtherTimeline;
}
