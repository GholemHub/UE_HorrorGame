#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "DraggableComponent.generated.h"

class UStaticMeshComponent;
class UDrag_Component;
class ABase_Item;

UENUM(BlueprintType)
enum class EItemType1 : uint8
{
    None,
    Key,
    Battery,
    Note,
    Tool,
    Draggable,
    DraggableInvertLeft,
    DraggableInvertRight
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDoorStateChanged1, bool, bIsClosed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShelfStateChanged1);

/**
 * Fully generic draggable/interactive component.
 * Can be attached to ANY actor (door, drawer, cabinet, etc).
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HRONO_API UDraggableComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UDraggableComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // =========================================================
    // OWNER REFERENCES (NO dependency on ADrag_Item)
    // =========================================================

    UPROPERTY()
    AActor* OwnerActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    UStaticMeshComponent* ItemMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    UDrag_Component* DragComponent;

    // =========================================================
    // ITEM TYPE
    // =========================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    EItemType1 ItemType = EItemType1::None;

    // =========================================================
    // DOOR SYSTEM
    // =========================================================

    UPROPERTY(ReplicatedUsing = OnRep_DoorRotation, BlueprintReadWrite, Category = "Door")
    FRotator DoorRotation;

    UFUNCTION()
    void OnRep_DoorRotation();

    UPROPERTY(ReplicatedUsing = OnRep_IsClosed, BlueprintReadWrite, Category = "Door")
    bool bIsClosed = true;

    UFUNCTION()
    void OnRep_IsClosed();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    float DoorClosedYawTolerance = 1.0f;

    UPROPERTY(BlueprintAssignable, Category = "Door")
    FOnDoorStateChanged1 OnDoorStateChanged;

    UFUNCTION(BlueprintCallable, Category = "Door")
    void RefreshDoorClosedState();

    // =========================================================
    // SHELF SYSTEM
    // =========================================================

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "Shelf")
    FVector ShelfPosition = FVector::ZeroVector;

    UPROPERTY(Replicated, BlueprintReadWrite, Category = "Shelf")
    bool bIsShelfOpen = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelf")
    float ShelfMaxDistance = 50.f;

    UPROPERTY(BlueprintAssignable, Category = "Shelf")
    FOnShelfStateChanged1 OnShelfOpen;

    UPROPERTY(BlueprintAssignable, Category = "Shelf")
    FOnShelfStateChanged1 OnShelfClose;

    UFUNCTION(BlueprintCallable, Category = "Shelf")
    void RefreshShelfOpenState();

    // =========================================================
    // KEY SYSTEM
    // =========================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key")
    ABase_Item* KeyActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key")
    bool bNeedKeyActor = false;

    // =========================================================
    // API (external control)
    // =========================================================

    UFUNCTION(BlueprintCallable, Category = "Door")
    void SetDoorRotation(const FRotator& NewRotation);

    UFUNCTION(BlueprintCallable, Category = "Shelf")
    void SetShelfPosition(const FVector& NewPosition);

    UFUNCTION(BlueprintCallable, Category = "State")
    bool IsDoorClosed() const { return bIsClosed; }

    UFUNCTION(BlueprintCallable, Category = "State")
    bool IsShelfOpen() const { return bIsShelfOpen; }

protected:

    // =========================================================
    // INTERNAL LOGIC
    // =========================================================

    UFUNCTION()
    void OnShelfOpened();

    UFUNCTION()
    void OnShelfClosed();

    void UpdateShelfCollision();

    void CacheOwnerComponents();

    bool HasAuthorityOwner() const;
};