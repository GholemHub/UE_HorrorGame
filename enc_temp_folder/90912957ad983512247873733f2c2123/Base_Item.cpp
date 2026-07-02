// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Base_Item.h"
#include "Net/UnrealNetwork.h"
#include "GameplayTagsManager.h"
#include "HronoCharacter.h"

// Sets default values
ABase_Item::ABase_Item()
{

	bReplicates = true;
	SetReplicateMovement(true); // CRITICAL: Allows the drop fall/position to replicate!

	PrimaryActorTick.bCanEverTick = true;


	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	//RootComponent = ItemMesh;
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ItemMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

}

void ABase_Item::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABase_Item, OwningCharacter);
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

	return true;
}

void ABase_Item::AttachToCharacter()
{
	if (UStaticMeshComponent* Mesh = GetItemMesh())
	{
		Mesh->SetSimulatePhysics(false);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		UE_LOG(LogTemp, Warning, TEXT("[Item] %s Disabled physics for attachment"), *GetName());
	}

	auto Player = Cast<AHronoCharacter>(OwningCharacter);
	if (!Player) {
		UE_LOG(LogTemp, Warning, TEXT("OwnongCharacter is null"));

		return;
	}
	const bool bAttached = AttachToComponent(
		Player->InteractionPoint,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale
	);

	bIsPickedUp = true;
	
}

void ABase_Item::OnPickedUp(AHronoCharacter* Character)
{
	OwningCharacter = Character;

	// Set native network ownership to allow safe attachment replication
	SetOwner(Character);

	AttachToCharacter();
	UE_LOG(LogTemp, Warning, TEXT("PickUp"));
}


void ABase_Item::OnRep_OwningCharacter()
{
	if (OwningCharacter)
	{
		// Clients run attachment logic here
		AttachToCharacter();
		UE_LOG(LogTemp, Warning, TEXT("[Item] %s OnRep: Attaching to %s"), *GetName(), *OwningCharacter->GetName());
	}
	else
	{
		// OwningCharacter was cleared — item was dropped on client side
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		// Ensure it becomes visible again on clients
		SetActorHiddenInGame(false);

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

	bIsPickedUp = false;

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
}

// Called every frame
void ABase_Item::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateMeshForLocalPlayer();
	
}

