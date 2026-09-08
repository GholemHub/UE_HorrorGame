#include "Enviroment/PlayerVisibilityZone.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HronoCharacter.h"
#include "HronoCollisionChannels.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogPlayerVisibilityZone, Log, All);

APlayerVisibilityZone::APlayerVisibilityZone()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(false);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	VisibilityVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("VisibilityVolume"));
	VisibilityVolume->SetupAttachment(SceneRoot);
	VisibilityVolume->InitBoxExtent(FVector(200.0f, 200.0f, 150.0f));
	VisibilityVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	VisibilityVolume->SetCollisionObjectType(ECC_WorldDynamic);
	VisibilityVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	VisibilityVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	VisibilityVolume->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, ECR_Overlap);
	VisibilityVolume->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, ECR_Overlap);
	VisibilityVolume->SetGenerateOverlapEvents(true);
}

void APlayerVisibilityZone::BeginPlay()
{
	Super::BeginPlay();

	VisibilityVolume->OnComponentBeginOverlap.AddUniqueDynamic(
		this, &APlayerVisibilityZone::HandleVolumeBeginOverlap);
	VisibilityVolume->OnComponentEndOverlap.AddUniqueDynamic(
		this, &APlayerVisibilityZone::HandleVolumeEndOverlap);

	CheckZoneState();
	GetWorldTimerManager().SetTimer(
		OccupantCheckTimerHandle,
		this,
		&APlayerVisibilityZone::CheckZoneState,
		FMath::Max(0.1f, OccupantCheckInterval),
		true);
}

void APlayerVisibilityZone::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(OccupantCheckTimerHandle);

	if (IsValid(VisibilityVolume))
	{
		VisibilityVolume->OnComponentBeginOverlap.RemoveDynamic(
			this, &APlayerVisibilityZone::HandleVolumeBeginOverlap);
		VisibilityVolume->OnComponentEndOverlap.RemoveDynamic(
			this, &APlayerVisibilityZone::HandleVolumeEndOverlap);
	}

	PlayersInside.Reset();
	RefreshLocalPlayerVisibility();
	Super::EndPlay(EndPlayReason);
}

void APlayerVisibilityZone::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APlayerVisibilityZone, PlayersInside);
}

bool APlayerVisibilityZone::IsPlayerInside(const AHronoCharacter* Player) const
{
	return IsValid(Player) && PlayersInside.Contains(Player);
}

bool APlayerVisibilityZone::ShouldRevealToViewer(
	const UObject* WorldContextObject,
	const AHronoCharacter* Viewer,
	const AHronoCharacter* OtherPlayer)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	if (!World || !IsValid(Viewer) || !IsValid(OtherPlayer) || Viewer == OtherPlayer)
	{
		return false;
	}

	for (TActorIterator<APlayerVisibilityZone> It(World); It; ++It)
	{
		const APlayerVisibilityZone* Zone = *It;
		if (!IsValid(Zone) || Zone->IsActorBeingDestroyed() || !Zone->IsPlayerInside(OtherPlayer))
		{
			continue;
		}

		if (!Zone->bRequireViewerInside || Zone->IsPlayerInside(Viewer))
		{
			return true;
		}
	}

	return false;
}

void APlayerVisibilityZone::HandleVolumeBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	AHronoCharacter* Player = Cast<AHronoCharacter>(OtherActor);
	if (HasAuthority() && IsValid(Player)
		&& OtherComponent == Player->GetCapsuleComponent())
	{
		AddPlayer(Player);
	}
}

void APlayerVisibilityZone::HandleVolumeEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex)
{
	AHronoCharacter* Player = Cast<AHronoCharacter>(OtherActor);
	if (HasAuthority() && IsValid(Player)
		&& OtherComponent == Player->GetCapsuleComponent()
		&& !VisibilityVolume->IsOverlappingActor(Player))
	{
		RemovePlayer(Player);
	}
}

void APlayerVisibilityZone::OnRep_PlayersInside()
{
	RefreshLocalPlayerVisibility();
}

void APlayerVisibilityZone::AddPlayer(AHronoCharacter* Player)
{
	if (!HasAuthority() || !IsValid(Player) || PlayersInside.Contains(Player))
	{
		return;
	}

	PlayersInside.Add(Player);
	ForceNetUpdate();
	RefreshLocalPlayerVisibility();
	UE_LOG(LogPlayerVisibilityZone, Log,
		TEXT("[PlayerVisibilityZone] %s entered %s"),
		*GetNameSafe(Player), *GetNameSafe(this));
}

void APlayerVisibilityZone::RemovePlayer(AHronoCharacter* Player)
{
	if (!HasAuthority() || PlayersInside.Remove(Player) == 0)
	{
		return;
	}

	ForceNetUpdate();
	RefreshLocalPlayerVisibility();
	UE_LOG(LogPlayerVisibilityZone, Log,
		TEXT("[PlayerVisibilityZone] %s exited %s"),
		*GetNameSafe(Player), *GetNameSafe(this));
}

void APlayerVisibilityZone::CheckZoneState()
{
	if (HasAuthority())
	{
		ReconcileServerOccupants();
	}

	// Initial replicated data may arrive before the local controller possesses its
	// pawn or before the other pawn is spawned. Reapplying locally makes that order
	// irrelevant even when the replicated occupant list itself did not change.
	RefreshLocalPlayerVisibility();
}

void APlayerVisibilityZone::ReconcileServerOccupants()
{
	if (!HasAuthority() || !IsValid(VisibilityVolume))
	{
		return;
	}

	TArray<AActor*> OverlappingActors;
	VisibilityVolume->GetOverlappingActors(
		OverlappingActors, AHronoCharacter::StaticClass());

	TSet<AHronoCharacter*> ActualPlayers;
	for (AActor* Actor : OverlappingActors)
	{
		if (AHronoCharacter* Player = Cast<AHronoCharacter>(Actor))
		{
			ActualPlayers.Add(Player);
		}
	}

	TArray<TObjectPtr<AHronoCharacter>> PreviouslyTrackedPlayers = PlayersInside;
	for (AHronoCharacter* Player : PreviouslyTrackedPlayers)
	{
		if (!IsValid(Player) || !ActualPlayers.Contains(Player))
		{
			RemovePlayer(Player);
		}
	}

	for (AHronoCharacter* Player : ActualPlayers)
	{
		AddPlayer(Player);
	}
}

void APlayerVisibilityZone::RefreshLocalPlayerVisibility() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<AHronoCharacter> It(World); It; ++It)
	{
		AHronoCharacter* Character = *It;
		if (IsValid(Character) && Character->IsLocallyControlled())
		{
			Character->RefreshTimelineVisibilityForLocalPlayer();
			break;
		}
	}
}
