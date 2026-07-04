#include "Components/DraggableComponent.h"

#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

#include "Components/Drag_Component.h"
#include "Items/Base_Item.h"

// =========================================================
// Constructor
// =========================================================

UDraggableComponent::UDraggableComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(true);
}

// =========================================================
// BeginPlay
// =========================================================

void UDraggableComponent::BeginPlay()
{
    Super::BeginPlay();

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    USceneComponent* Root = Owner->GetRootComponent();
    if (!Root)
    {
        return;
    }

    // =====================================================
    // CREATE ONLY ITEM MESH
    // =====================================================
    ItemMesh = NewObject<UStaticMeshComponent>(Owner, TEXT("ItemMesh"));

    if (!ItemMesh)
    {
        return;
    }

    ItemMesh->RegisterComponent();
    ItemMesh->AttachToComponent(
        Root,
        FAttachmentTransformRules::KeepRelativeTransform
    );

    ItemMesh->SetMobility(EComponentMobility::Movable);
    ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // Optional: make sure it replicates movement if needed
    ItemMesh->SetIsReplicated(true);

    // =====================================================
    // OPTIONAL: find drag component
    // =====================================================
    DragComponent = Owner->FindComponentByClass<UDrag_Component>();

    UE_LOG(LogTemp, Log, TEXT("DraggableComponent: ItemMesh created and attached"));
}

// =========================================================
// Cache components from owner
// =========================================================

void UDraggableComponent::CacheOwnerComponents()
{
    if (!OwnerActor)
    {
        return;
    }

    // Try to find meshes automatically if not assigned
    TArray<UStaticMeshComponent*> Meshes;
    OwnerActor->GetComponents<UStaticMeshComponent>(Meshes);

    if (Meshes.Num() > 0)
    {

        if (Meshes.Num() > 1)
        {
            ItemMesh = Meshes[1];
        }
    }

    DragComponent = OwnerActor->FindComponentByClass<UDrag_Component>();
}

// =========================================================
// Tick
// =========================================================

void UDraggableComponent::TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!ItemMesh)
    {
        return;
    }

    // Shelf auto-update (same behavior as your original Tick logic dependency)
    RefreshShelfOpenState();
}

// =========================================================
// REPLICATION
// =========================================================

void UDraggableComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UDraggableComponent, DoorRotation);
    DOREPLIFETIME(UDraggableComponent, bIsClosed);
    DOREPLIFETIME(UDraggableComponent, ShelfPosition);
    DOREPLIFETIME(UDraggableComponent, bIsShelfOpen);
}

// =========================================================
// DOOR
// =========================================================

void UDraggableComponent::SetDoorRotation(const FRotator& NewRotation)
{
    if (!HasAuthorityOwner())
    {
        return;
    }

    DoorRotation = NewRotation;

    if (ItemMesh)
    {
        ItemMesh->SetRelativeRotation(DoorRotation);
    }

    RefreshDoorClosedState();
}

void UDraggableComponent::RefreshDoorClosedState()
{
    if (!HasAuthorityOwner())
    {
        return;
    }

    const bool bNewClosed =
        FMath::Abs(DoorRotation.Yaw) <= DoorClosedYawTolerance;

    if (bNewClosed == bIsClosed)
    {
        return;
    }

    bIsClosed = bNewClosed;

    if (GEngine)
    {
        UE_LOG(LogTemp, Log, TEXT("[SERVER] Door %s"),
            bIsClosed ? TEXT("closed") : TEXT("open"));
    }

    OnDoorStateChanged.Broadcast(bIsClosed);
}

void UDraggableComponent::OnRep_DoorRotation()
{
    if (!ItemMesh)
    {
        return;
    }

    // prevent override during local dragging
    if (DragComponent && DragComponent->bIsRotating)
    {
        return;
    }

    ItemMesh->SetRelativeRotation(DoorRotation);
}

void UDraggableComponent::OnRep_IsClosed()
{
    if (GEngine)
    {
        UE_LOG(LogTemp, Log, TEXT("[CLIENT] Door %s"),
            bIsClosed ? TEXT("closed") : TEXT("open"));
    }

    OnDoorStateChanged.Broadcast(bIsClosed);
}

// =========================================================
// SHELF
// =========================================================

void UDraggableComponent::SetShelfPosition(const FVector& NewPosition)
{
    if (!HasAuthorityOwner() || !ItemMesh)
    {
        return;
    }

    ShelfPosition = NewPosition;
    ItemMesh->SetRelativeLocation(ShelfPosition);

    RefreshShelfOpenState();
}

void UDraggableComponent::RefreshShelfOpenState()
{
    if (!ItemMesh)
    {
        return;
    }

    FVector CurrentPosition = ItemMesh->GetRelativeLocation();

    float CurrentDistance = FMath::Abs(CurrentPosition.Y);

    bool bIsNowOpen =
        CurrentDistance > (ShelfMaxDistance * 0.5f);

    if (bIsNowOpen == bIsShelfOpen)
    {
        return;
    }

    bIsShelfOpen = bIsNowOpen;

    if (bIsShelfOpen)
    {
        OnShelfOpened();
    }
    else
    {
        OnShelfClosed();
    }

    UpdateShelfCollision();
}

void UDraggableComponent::OnShelfOpened()
{
    OnShelfOpen.Broadcast();

    UE_LOG(LogTemp, Log, TEXT("Shelf opened"));
}

void UDraggableComponent::OnShelfClosed()
{
    OnShelfClose.Broadcast();

    UE_LOG(LogTemp, Log, TEXT("Shelf closed"));
}

void UDraggableComponent::UpdateShelfCollision()
{
    if (!ItemMesh)
    {
        return;
    }

    ItemMesh->SetCollisionResponseToChannel(
        ECC_Pawn,
        bIsShelfOpen ? ECR_Ignore : ECR_Block
    );
}

// =========================================================
// UTIL
// =========================================================


bool UDraggableComponent::HasAuthorityOwner() const
{
    return OwnerActor && OwnerActor->HasAuthority();
}