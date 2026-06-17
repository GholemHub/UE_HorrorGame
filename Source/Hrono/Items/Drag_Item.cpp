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
}

void ADrag_Item::OnRep_DoorRotation()
{
	// The client that is actively dragging trusts its own local prediction; every
	// other machine (and the server visuals) applies the replicated authoritative value.
	if (DragComponent && DragComponent->bIsRotating)
	{
		return;
	}

	if (ItemMesh)
	{
		ItemMesh->SetRelativeRotation(DoorRotation);
	}
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

        }

    }
   
}


