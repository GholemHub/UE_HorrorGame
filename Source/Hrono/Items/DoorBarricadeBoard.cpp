#include "Items/DoorBarricadeBoard.h"

#include "HronoCharacter.h"
#include "HronoCollisionChannels.h"
#include "Items/AxeItem.h"
#include "Items/Drag_Item.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
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
}

void ADoorBarricadeBoard::BeginPlay()
{
	Super::BeginPlay();

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

	bBroken = true;
	ADrag_Item* DoorThatWasBlocked = BlockedDoor;
	UnregisterFromDoor();
	MulticastBoardBroken(Character, DoorThatWasBlocked);
	ForceNetUpdate();
	SetLifeSpan(0.2f);
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
	if (bBroken && BoardMesh)
	{
		BoardMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		BoardMesh->SetVisibility(false, true);
	}
}

void ADoorBarricadeBoard::MulticastBoardBroken_Implementation(
	AHronoCharacter* BreakingCharacter,
	ADrag_Item* DoorThatWasBlocked)
{
	OnRep_Broken();

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
}

void ADoorBarricadeBoard::UpdateLocalVisibility()
{
	if (!BoardMesh || bBroken || !GetWorld())
	{
		return;
	}

	APlayerController* LocalController = GetWorld()->GetFirstPlayerController();
	const AHronoCharacter* LocalCharacter = LocalController
		? Cast<AHronoCharacter>(LocalController->GetPawn())
		: nullptr;
	if (LocalCharacter)
	{
		BoardMesh->SetVisibility(DoesTimelineMatch(LocalCharacter->GetTimeline()), true);
	}
}

bool ADoorBarricadeBoard::DoesTimelineMatch(EItemTimeline OtherTimeline) const
{
	return BoardTimeline == EItemTimeline::Both
		|| OtherTimeline == EItemTimeline::Both
		|| BoardTimeline == OtherTimeline;
}
