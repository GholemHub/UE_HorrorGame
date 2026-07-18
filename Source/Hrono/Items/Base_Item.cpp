// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Base_Item.h"
#include "Net/UnrealNetwork.h"
#include "GameplayTagsManager.h"
#include "HronoCharacter.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

// Sets default values
ABase_Item::ABase_Item()
{

	bReplicates = true;
	SetReplicateMovement(true); // CRITICAL: Allows the drop fall/position to replicate!

	PrimaryActorTick.bCanEverTick = true;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(DefaultSceneRoot);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ItemMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

}


void ABase_Item::Use_Implementation(AActor* Character)
{
	//UE_LOG(LogTemp, Warning, TEXT("Default Use"));
}

void ABase_Item::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABase_Item, OwningCharacter);
	DOREPLIFETIME(ABase_Item, ItemTimeline);
}

void ABase_Item::SetItemTimeline(EItemTimeline NewTimeline)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Item] %s SetItemTimeline attempted on non-authority, ignoring"), *GetName());
		return;
	}

	if (ItemTimeline == NewTimeline)
	{
		return;
	}

	ItemTimeline = NewTimeline;
	CurrentCachedTimeline = EItemTimeline::Both;
	ApplyItemTimelineState();
	ForceNetUpdate();
}

void ABase_Item::OnRep_ItemTimeline()
{
	CurrentCachedTimeline = EItemTimeline::Both;
	ApplyItemTimelineState();
}

void ABase_Item::UpdateMeshForLocalPlayer()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	auto Character = Cast<AHronoCharacter>(PC->GetPawn());
	if (!Character) return;

	EItemTimeline TargetTimeline = Character->GetTimeline();

	// Only switch when the local player's timeline actually changed.
	if (CurrentCachedTimeline == TargetTimeline)
	{
		return;
	}

	
	UpdateVisibilityForLocalPlayer(TargetTimeline);

	CurrentCachedTimeline = TargetTimeline;
}

void ABase_Item::UpdateVisibilityForLocalPlayer(EItemTimeline ViewerTimeline)
{
	
	const bool bShouldBeVisible = (ItemTimeline == EItemTimeline::Both || ItemTimeline == ViewerTimeline);

	if (USceneComponent* Root = GetRootComponent())
	{
		Root->SetVisibility(bShouldBeVisible, /*bPropagateToChildren=*/true);
	}
}

bool ABase_Item::TryPickUp(AHronoCharacter* Character)
{
	
	if (ItemTimeline != EItemTimeline::Both && ItemTimeline != Character->GetTimeline())
	{
		return false;
	}


	// Only allow pickup on authority
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Item] %s Pickup attempted on non-authority, ignoring"), *GetName());
		return false;
	}

	// Call pickup logic (NOT the base destroy behavior)
	OnPickedUp(Character);

	return bIsPickedUp;
}

bool ABase_Item::AttachToCharacter()
{
	TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents(this);
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (!IsValid(PrimitiveComponent))
		{
			continue;
		}

		PrimitiveComponent->SetSimulatePhysics(false);
		PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PrimitiveComponent->SetMobility(EComponentMobility::Movable);
	}

	auto Player = Cast<AHronoCharacter>(OwningCharacter);
	if (!Player) {
		UE_LOG(LogTemp, Warning, TEXT("[Item] %s has no owning character"), *GetName());

		return false;
	}

	if (USceneComponent* Root = GetRootComponent())
	{
		Root->SetMobility(EComponentMobility::Movable);

		// A non-root primitive is detached by Unreal when physics simulation starts.
		// Reattach and reset it before moving the actor root. KeepWorldTransform would
		// preserve the offset accumulated while the dropped mesh was falling, causing
		// the second pickup to appear far behind the character.
		if (IsValid(ItemMesh) && ItemMesh != Root)
		{
			if (ItemMesh->GetAttachParent() != Root)
			{
				ItemMesh->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
			}

			// Physics detaches a non-root mesh and changes its relative transform to
			// world space. Restore the Blueprint-authored mesh transform so HoldOffset
			// is the only transform that controls how the held item is positioned.
			ItemMesh->SetRelativeTransform(ItemMeshRelativeTransform);
		}
	}

	const bool bAttached = AttachToComponent(
		Player->InteractionPoint,
		FAttachmentTransformRules::SnapToTargetIncludingScale
	);
	if (!bAttached)
	{
		UE_LOG(LogTemp, Error, TEXT("[Item] Failed to attach %s to %s"), *GetName(), *GetNameSafe(Player->InteractionPoint));
		return false;
	}

	SetActorRelativeTransform(HoldOffset);

	bIsPickedUp = true;
	UE_LOG(LogTemp, Warning, TEXT("[Item] Attached %s to %s"), *GetName(), *GetNameSafe(Player->InteractionPoint));
	return true;
}
#include "Items/Dozimetr.h"

void ABase_Item::OnPickedUp(AHronoCharacter* Character)
{
	OwningCharacter = Character;

	// Set native network ownership to allow safe attachment replication
	SetOwner(Character);

	if (!AttachToCharacter())
	{
		SetOwner(nullptr);
		OwningCharacter = nullptr;
		return;
	}
	UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
	UE_LOG(LogTemp, Warning, TEXT("PickUp"));
	auto Dozimetr = Cast<ADozimetr>(this);
	if (Dozimetr) {
		Dozimetr->On();
	}

}


void ABase_Item::OnRep_OwningCharacter()
{
	if (OwningCharacter)
	{
		// Clients run attachment logic here
		AttachToCharacter();
		UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
		UE_LOG(LogTemp, Warning, TEXT("[Item] %s OnRep: Attaching to %s"), *GetName(), *OwningCharacter->GetName());
	}
	else
	{
		// OwningCharacter was cleared — item was dropped on client side
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		// Ensure it becomes visible again on clients
		SetActorHiddenInGame(false);

		UGameplayStatics::PlaySoundAtLocation(this, DropSound, GetActorLocation());

		// FIX 1: Turn actor-level collision back on! 
		// Changing the mesh component collision alone will fail if the actor itself is disabled.
		SetActorEnableCollision(true);

		if (UStaticMeshComponent* Mesh = GetItemMesh())
		{
			// Re-enable collision so clients can see/re-interact with it
			Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

			// FIX 2: Clients MUST simulate physics if the server is simulating physics!
			// Unreal's built-in network movement code uses the server's physics simulation 
			// to drive and smoothly interpolate the client's simulated body.
			Mesh->SetSimulatePhysics(true);
		}

		bIsPickedUp = false;
	}
}

void ABase_Item::Drop()
{
	if (!bIsPickedUp || !OwningCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Item] %s Cannot drop - not currently picked up"), *GetName());
		return;
	}

	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Item] %s Drop attempted on non-authority, ignoring"), *GetName());
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Item] %s Dropped by %s"), *GetName(), *OwningCharacter->GetName());

	DetachFromCharacter();

	UGameplayStatics::PlaySoundAtLocation(this, DropSound, GetActorLocation());

	bIsPickedUp = false;

	auto Dozimetr = Cast<ADozimetr>(this);
	if (Dozimetr) {
		Dozimetr->Off();
	}

	// Clear both native ownership and your replication variable
	SetOwner(nullptr);
	OwningCharacter = nullptr;
}

void ABase_Item::DetachFromCharacter()
{
	UE_LOG(LogTemp, Warning, TEXT("[Item] %s Detaching from character"), *GetName());

	// Detach from parent
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	// Ensure it's visible in the world after being dropped
	SetActorHiddenInGame(false);

	// Re-enable physics and collision on the Server
	if (UStaticMeshComponent* Mesh = GetItemMesh())
	{
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Mesh->SetSimulatePhysics(true); // Server simulates the actual physical drop
		UE_LOG(LogTemp, Warning, TEXT("[Item] %s Re-enabled physics on Server"), *GetName());
	}
	SetActorEnableCollision(true);

}




// Called when the game starts or when spawned
void ABase_Item::BeginPlay()
{
	Super::BeginPlay();
	if (IsValid(ItemMesh))
	{
		ItemMeshRelativeTransform = ItemMesh->GetRelativeTransform();
	}
	ApplyItemTimelineState();
}

void ABase_Item::ApplyItemTimelineState()
{
	if (ItemTimeline == EItemTimeline::Future && FutureMesh)
	{
		ItemMesh->SetStaticMesh(FutureMesh);
		ItemTag = UGameplayTagsManager::Get().RequestGameplayTag(FName("Item.Future"));
	}
	else if (ItemTimeline == EItemTimeline::Past && PastMesh)
	{
		ItemMesh->SetStaticMesh(PastMesh);
		ItemTag = UGameplayTagsManager::Get().RequestGameplayTag(FName("Item.Past"));
	}


	if (ItemTimeline == EItemTimeline::Future)
	{


		ItemMesh->SetCollisionObjectType(COLLISION_CHANNEL_DOOR_FUTURE);
		ItemMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, ECR_Block);
		ItemMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, ECR_Ignore);
	}
	else if (ItemTimeline == EItemTimeline::Past)
	{


		ItemMesh->SetCollisionObjectType(COLLISION_CHANNEL_DOOR_PAST);
		ItemMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, ECR_Block);
		ItemMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, ECR_Ignore);

	}
	else // EItemTimeline::Both — blocks all pawns
	{


		ItemMesh->SetCollisionObjectType(COLLISION_CHANNEL_DOOR_PAST);
		ItemMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, ECR_Block);
		ItemMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, ECR_Block);
	}

	// Refresh visibility immediately after a timeline change instead of waiting for Tick.
	UpdateMeshForLocalPlayer();
}

// Called every frame
void ABase_Item::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateMeshForLocalPlayer();
	
}

