// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Drag_Item.h"
#include "Components/Drag_Component.h"
#include "Net/UnrealNetwork.h"



// Sets default values
ADrag_Item::ADrag_Item()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    bReplicates = false;

	FrameMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrameMesh"));
	RootComponent = FrameMesh;

   
    ItemMesh->SetupAttachment(FrameMesh);

	DragComponent = CreateDefaultSubobject<UDrag_Component>(TEXT("DragComponent"));

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

    if (ItemTimeline == EItemTimeline::Future)
    {
        FrameMesh->SetCollisionObjectType(ECC_GameTraceChannel4);
        FrameMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block);
        FrameMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);
    }
    else if (ItemTimeline == EItemTimeline::Past)
    {
        FrameMesh->SetCollisionObjectType(ECC_GameTraceChannel3);
        FrameMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
        FrameMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Ignore);
    }

    const FString Role1 = HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT");
    UE_LOG(LogTemp, Warning,
        TEXT("[DoorCollision][%s] %s | Timeline=%s | ObjType=%d | RespVsPast=%d | RespVsFuture=%d"),
        *Role1, *GetName(), *UEnum::GetValueAsString(ItemTimeline),
        (int32)FrameMesh->GetCollisionObjectType(),
        (int32)FrameMesh->GetCollisionResponseToChannel(ECC_GameTraceChannel1),
        (int32)FrameMesh->GetCollisionResponseToChannel(ECC_GameTraceChannel2));
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


