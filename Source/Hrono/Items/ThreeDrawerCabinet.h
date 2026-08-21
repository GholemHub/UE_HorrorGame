#pragma once

#include "CoreMinimal.h"
#include "Items/Drag_Item.h"
#include "ThreeDrawerCabinet.generated.h"

class UPrimitiveComponent;

/** DrawerIndex is 0 = bottom, 1 = middle, 2 = top. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnCabinetDrawerStateChanged,
	int32, DrawerIndex,
	bool, bIsOpen);

/**
 * One replicated cabinet actor with three independently draggable drawers.
 *
 * The inherited ItemMesh/DragComponent pair represents the bottom drawer.
 * Middle and top drawers each own a mesh, drag component, replicated position,
 * and open-state bit. ItemTimeline is shared by the complete cabinet, while
 * Base_Item's local visibility pass evaluates that timeline per local player.
 */
UCLASS(Blueprintable)
class HRONO_API AThreeDrawerCabinet : public ADrag_Item
{
	GENERATED_BODY()

public:
	AThreeDrawerCabinet();

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void UpdateMeshForLocalPlayer() override;
	virtual USceneComponent* GetPrimaryDoorMovementComponent() const override;
	virtual USceneComponent* FindShelfMovementComponent(
		FName ShelfComponentName) const override;
	virtual void ApplyShelfPositionFromServer(
		FName ShelfComponentName,
		const FVector& NewPosition) override;

	/** Alias for inherited ItemMesh. This is the bottom drawer. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cabinet|Drawers")
	TObjectPtr<UStaticMeshComponent> BottomDrawerMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cabinet|Drawers")
	TObjectPtr<UStaticMeshComponent> MiddleDrawerMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cabinet|Drawers")
	TObjectPtr<UStaticMeshComponent> TopDrawerMesh;

	/** Alias for inherited DragComponent. This controls the bottom drawer. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cabinet|Drawers")
	TObjectPtr<UDrag_Component> BottomDrawerDragComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cabinet|Drawers")
	TObjectPtr<UDrag_Component> MiddleDrawerDragComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cabinet|Drawers")
	TObjectPtr<UDrag_Component> TopDrawerDragComponent;

	/** Item spawn points that move with their respective drawers. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cabinet|Item Points")
	TObjectPtr<USceneComponent> BottomDrawerPointSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cabinet|Item Points")
	TObjectPtr<USceneComponent> MiddleDrawerPointSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cabinet|Item Points")
	TObjectPtr<USceneComponent> TopDrawerPointSet;

	UPROPERTY(
		ReplicatedUsing = OnRep_MiddleDrawerPosition,
		VisibleInstanceOnly,
		BlueprintReadOnly,
		Category = "Cabinet|Replication")
	FVector MiddleDrawerPosition = FVector(0.0f, 0.0f, 48.0f);

	UPROPERTY(
		ReplicatedUsing = OnRep_TopDrawerPosition,
		VisibleInstanceOnly,
		BlueprintReadOnly,
		Category = "Cabinet|Replication")
	FVector TopDrawerPosition = FVector(0.0f, 0.0f, 80.0f);

	/** One replicated bit per drawer: bottom, middle, top. */
	UPROPERTY(
		ReplicatedUsing = OnRep_DrawerOpenMask,
		VisibleInstanceOnly,
		BlueprintReadOnly,
		Category = "Cabinet|Replication")
	uint8 DrawerOpenMask = 0;

	UPROPERTY(BlueprintAssignable, Category = "Cabinet|Drawers")
	FOnCabinetDrawerStateChanged OnDrawerStateChanged;

	/** DrawerIndex is 0 = bottom, 1 = middle, 2 = top. */
	UFUNCTION(BlueprintPure, Category = "Cabinet|Drawers")
	bool IsDrawerOpen(int32 DrawerIndex) const;

	/** DrawerIndex is 0 = bottom, 1 = middle, 2 = top. */
	UFUNCTION(BlueprintPure, Category = "Cabinet|Drawers")
	UStaticMeshComponent* GetDrawerMesh(int32 DrawerIndex) const;

	/** DrawerIndex is 0 = bottom, 1 = middle, 2 = top. */
	UFUNCTION(BlueprintPure, Category = "Cabinet|Item Points")
	USceneComponent* GetDrawerPointSet(int32 DrawerIndex) const;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_MiddleDrawerPosition();

	UFUNCTION()
	void OnRep_TopDrawerPosition();

	UFUNCTION()
	void OnRep_DrawerOpenMask(uint8 PreviousMask);

	uint8 CalculateDrawerOpenMask() const;
	void RefreshDrawerOpenState();
	void BroadcastDrawerStateChanges(uint8 PreviousMask);
	void ConfigureDrawerCollision(UPrimitiveComponent* Component) const;

	EItemTimeline DrawerCollisionTimeline = EItemTimeline::Both;
};
