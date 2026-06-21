// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Drag_Item.h"
#include "Components/Drag_Component.h"
#include "HronoCollisionChannels.h"
#include "Net/UnrealNetwork.h"



// Sets default values
ADrag_Item::ADrag_Item()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true; // Door open/closed state must replicate so server collision matches clients

	FrameMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrameMesh"));
	RootComponent = FrameMesh;


	ItemMesh->SetupAttachment(FrameMesh);

	DragComponent = CreateDefaultSubobject<UDrag_Component>(TEXT("DragComponent"));

}

void ADrag_Item::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADrag_Item, DoorRotation);
    DOREPLIFETIME(ADrag_Item, bIsClosed);

}

void ADrag_Item::OnRep_DoorRotation()
{
    // Runs on remote clients only. Skip while this client is actively dragging so we
    // don't fight the local prediction in UDrag_Component.
    if (DragComponent && DragComponent->bIsRotating)
    {
        return;
    }

    if (ItemMesh)
    {
        // Apply the exact authoritative rotation so every machine matches the server.
        ItemMesh->SetRelativeRotation(DoorRotation);
    }
}

void ADrag_Item::RefreshDoorClosedState()
{
    // Authority is the single source of truth for the replicated bIsClosed flag.
    if (!HasAuthority())
    {
        return;
    }

    // The door is closed when its Yaw is (almost) zero.
    const bool bNewClosed = FMath::Abs(DoorRotation.Yaw) <= DoorClosedYawTolerance;
    if (bNewClosed == bIsClosed)
    {
        return;
    }

    bIsClosed = bNewClosed;

    // OnRep_IsClosed only fires on remote clients, so broadcast here for the
    // server/listen-server host as well.
    UE_LOG(LogTemp, Log, TEXT("[SERVER] Door %s"), bIsClosed ? TEXT("closed") : TEXT("open"));
    OnDoorStateChanged.Broadcast(bIsClosed);
}

void ADrag_Item::OnRep_IsClosed()
{
    // Runs on remote clients when the authority changes bIsClosed.
    UE_LOG(LogTemp, Log, TEXT("[CLIENT] Door %s"), bIsClosed ? TEXT("closed") : TEXT("open"));
    OnDoorStateChanged.Broadcast(bIsClosed);
}

void ADrag_Item::UpdateMeshForLocalPlayer()
{
    Super::UpdateMeshForLocalPlayer();
}

// Called when the game starts or when spawned
// ADrag_Item::BeginPlay
void ADrag_Item::BeginPlay()
{
    Super::BeginPlay();

    // Configure collision channels so the server (and client) physics correctly
    // filters which pawns can collide with this door based on timeline.
    if (ItemTimeline == EItemTimeline::Future)
    {
        FrameMesh->SetCollisionObjectType(COLLISION_CHANNEL_DOOR_FUTURE);
        FrameMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, ECR_Block);
        FrameMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, ECR_Ignore);

        ItemMesh->SetCollisionObjectType(COLLISION_CHANNEL_DOOR_FUTURE);
        ItemMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, ECR_Block);
        ItemMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, ECR_Ignore);
    }
    else if (ItemTimeline == EItemTimeline::Past)
    {
        FrameMesh->SetCollisionObjectType(COLLISION_CHANNEL_DOOR_PAST);
        FrameMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, ECR_Block);
        FrameMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, ECR_Ignore);

        ItemMesh->SetCollisionObjectType(COLLISION_CHANNEL_DOOR_PAST);
        ItemMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, ECR_Block);
        ItemMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, ECR_Ignore);

    }
    else // EItemTimeline::Both — blocks all pawns
    {
        FrameMesh->SetCollisionObjectType(COLLISION_CHANNEL_DOOR_PAST);
        FrameMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, ECR_Block);
        FrameMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, ECR_Block);

        ItemMesh->SetCollisionObjectType(COLLISION_CHANNEL_DOOR_PAST);
        ItemMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, ECR_Block);
        ItemMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, ECR_Block);
    }
}

// Called every frame
void ADrag_Item::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    UpdateMeshForLocalPlayer();

    if (GEngine)
    {
   

        FRotator Rot = ItemMesh->GetRelativeRotation();
        if (HasAuthority())
        {
            FString Role1 = HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT");

            FString Msg = FString::Printf(
                TEXT("[%s] Door Rot: %.1f %.1f %.1f"),
                *Role1,
                Rot.Pitch, Rot.Yaw, Rot.Roll
            );

            int32 Key = HasAuthority() ? 1 : 2;

            GEngine->AddOnScreenDebugMessage(
                Key,
                0.0f,
                FColor::Yellow,
                Msg
            );

            GEngine->AddOnScreenDebugMessage(
                Key,
                0.0f,
                FColor::Yellow,
                FString::Printf(TEXT("bIsClosed :: %i"), bIsClosed)
            );
        }
        else {
            FString Role1 = HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT");

            FString Msg = FString::Printf(
                TEXT("[%s] Door Rot: %.1f %.1f %.1f"),
                *Role1,
                Rot.Pitch, Rot.Yaw, Rot.Roll
            );

            int32 Key = HasAuthority() ? 1 : 2;

            GEngine->AddOnScreenDebugMessage(
                Key,
                0.0f,
                FColor::Yellow,
                Msg
            );

            GEngine->AddOnScreenDebugMessage(
                Key,
                0.0f,
                FColor::Yellow,
                FString::Printf(TEXT("bIsClosed :: %i"), bIsClosed)
            );
        }

    }
   
}


