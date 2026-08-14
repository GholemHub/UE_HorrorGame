#pragma once

#include "CoreMinimal.h"
#include "Interface/Enviroment_Interface.h"
#include "Items/Drag_Item.h"
#include "HidingWardrobe.generated.h"

class AHronoCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWardrobePlayerEvent, AHronoCharacter*, Player);

/**
 * A replicated two-door wardrobe built on top of Drag_Item.
 * ItemMesh is the left door mesh; RightDoorMesh is the right door mesh. Each door
 * rotates around a dedicated editable pivot and owns an independent DragComponent.
 */
UCLASS(Blueprintable)
class HRONO_API AHidingWardrobe : public ADrag_Item, public IEnviroment_Interface
{
	GENERATED_BODY()

public:
	AHidingWardrobe();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;
	virtual void AnimateDoor(bool bOpen) override;
	virtual void RefreshDoorClosedState() override;
	virtual USceneComponent* GetPrimaryDoorMovementComponent() const override;
	virtual USceneComponent* FindDoorMovementComponent(FName DoorComponentName) const override;
	virtual void ApplyDoorRotationFromServer(FName DoorComponentName, const FRotator& NewRotation) override;
	virtual void Interact_Implementation(AActor* Interactor) override;

	/** Hinge position for ItemMesh (the left door). Move this to the left edge of the frame. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wardrobe|Doors")
	TObjectPtr<USceneComponent> LeftDoorPivot;

	/** Hinge position for RightDoorMesh. Move this to the right edge of the frame. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wardrobe|Doors")
	TObjectPtr<USceneComponent> RightDoorPivot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wardrobe|Doors")
	TObjectPtr<UStaticMeshComponent> RightDoorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wardrobe|Doors")
	TObjectPtr<UDrag_Component> RightDoorDragComponent;

	/** Position of the hidden player's capsule/camera inside the wardrobe. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wardrobe|Hiding")
	TObjectPtr<USceneComponent> HidingPoint;

	/** Safe position used when the player leaves the wardrobe. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wardrobe|Hiding")
	TObjectPtr<USceneComponent> ExitPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wardrobe|Hiding")
	bool bAllowHiding = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wardrobe|Hiding")
	bool bRequireBothDoorsOpenToHide = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wardrobe|Hiding",
		meta = (EditCondition = "bRequireBothDoorsOpenToHide", ClampMin = "0.0", ClampMax = "170.0"))
	float MinimumDoorOpenAngleToHide = 45.0f;

	/** If enabled, interacting with a closed wardrobe starts opening both doors. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wardrobe|Hiding")
	bool bOpenDoorsWhenHidingIsBlocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wardrobe|Doors",
		meta = (ClampMin = "0.0", ClampMax = "170.0"))
	float RightDoorOpenAngle = 105.0f;

	UPROPERTY(ReplicatedUsing = OnRep_RightDoorRotation, VisibleInstanceOnly, BlueprintReadOnly,
		Category = "Wardrobe|Doors")
	FRotator RightDoorRotation = FRotator::ZeroRotator;

	UPROPERTY(ReplicatedUsing = OnRep_HiddenPlayer, VisibleInstanceOnly, BlueprintReadOnly,
		Category = "Wardrobe|Hiding")
	TObjectPtr<AHronoCharacter> HiddenPlayer;

	UPROPERTY(BlueprintAssignable, Category = "Wardrobe|Hiding")
	FWardrobePlayerEvent OnPlayerEnteredWardrobe;

	UPROPERTY(BlueprintAssignable, Category = "Wardrobe|Hiding")
	FWardrobePlayerEvent OnPlayerExitedWardrobe;

	UFUNCTION(BlueprintPure, Category = "Wardrobe|Hiding")
	bool IsOccupied() const { return HiddenPlayer != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Wardrobe|Hiding")
	bool AreDoorsOpenForHiding() const;

	UFUNCTION(BlueprintPure, Category = "Wardrobe|Hiding")
	bool CanPlayerHide(const AHronoCharacter* Player) const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Wardrobe|Hiding")
	bool TryEnterWardrobe(AHronoCharacter* Player);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Wardrobe|Hiding")
	bool ExitWardrobe(AHronoCharacter* Player);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnRep_RightDoorRotation();

	UFUNCTION()
	void OnRep_HiddenPlayer(AHronoCharacter* PreviousPlayer);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStartRightDoorAnimation(FRotator TargetRotation, float Duration);

private:
	void UpdateRightDoorAnimation(float DeltaTime);
	void ApplyHidingState(AHronoCharacter* Player, bool bEntering);
	void ConfigureRightDoorCollision();

	bool bRightDoorAnimationActive = false;
	float RightDoorAnimationElapsed = 0.0f;
	float ActiveRightDoorAnimationDuration = 1.0f;
	FRotator RightDoorAnimationStartRotation = FRotator::ZeroRotator;
	FRotator RightDoorAnimationTargetRotation = FRotator::ZeroRotator;
};
