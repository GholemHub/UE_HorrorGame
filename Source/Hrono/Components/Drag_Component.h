// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Drag_Component.generated.h"

class UPrimitiveComponent;
class USceneComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HRONO_API UDrag_Component : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDrag_Component();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION()
	void StartDrag(APlayerController* PC);
	
	UFUNCTION()
	void StopDrag();

	/** Scene component that actually rotates. Null uses the owner's primary door mesh/pivot. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	TObjectPtr<USceneComponent> TargetMovementComponentOverride;

	/** Primitive that must be hit to select this drag component. Null uses ItemMesh. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	TObjectPtr<UPrimitiveComponent> InteractionPrimitiveOverride;

	/** Independent limits are required by double doors: left opens positive, right negative. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	bool bUseCustomDoorAngleLimits = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door", meta = (EditCondition = "bUseCustomDoorAngleLimits"))
	float MinimumDoorYaw = -90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door", meta = (EditCondition = "bUseCustomDoorAngleLimits"))
	float MaximumDoorYaw = 90.0f;

	/** Multiplies horizontal mouse input for a custom door. Use -1 to invert a panel. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door", meta = (EditCondition = "bUseCustomDoorAngleLimits"))
	float DoorMouseInputDirection = 1.0f;

	/** Wardrobe-only view-relative input mode. It projects the panel's positive-Yaw
	 *  movement onto Camera Right, so the panel follows the mouse on screen from
	 *  both outside and inside. Ordinary doors must leave this disabled. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	bool bUseWardrobeFrontBackInput = false;

	/** Enable this only when the actor/mesh Forward axis points into the wardrobe
	 *  instead of toward its front. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door",
		meta = (EditCondition = "bUseWardrobeFrontBackInput"))
	bool bInvertDoorFrontSide = false;

	/** Side captured when drag begins. It remains stable for the entire gesture. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Door|Debug")
	bool bPlayerBehindDoor = false;

	USceneComponent* GetTargetMovementComponent() const;
	UPrimitiveComponent* GetInteractionPrimitive() const;
	bool MatchesHitComponent(const UPrimitiveComponent* HitComponent) const;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	bool bIsShelf = false;

	/** Treat this drag item as a sliding wardrobe/cupboard panel instead of a
	 *  hinged door. Horizontal mouse movement slides the mesh along CupBoardSlideAxis. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CupBoard")
	bool bIsCupBoard = false;

	UFUNCTION()
	void XDrag();
	
	void ShelfDrag();

	void CupBoardDrag();


	UPROPERTY()
	FRotator InitialRotation;

	UPROPERTY()
	float CurrentDoorAngle = 0.f;

	UPROPERTY()
	bool bIsRotating = false;

	UPROPERTY()
	APlayerController* RotatingController = nullptr;

	/** +1 from the front, -1 from behind when side-aware input is enabled. */
	float ActiveDoorSideMultiplier = 1.0f;

	/** View-relative Yaw sign locked for the lifetime of one mouse drag. */
	float ActiveViewRelativeYawDirection = -1.0f;
	bool bHasActiveViewRelativeYawDirection = false;

	UPROPERTY()
	FRotator StartRelativeRotation;

	UPROPERTY(EditAnywhere, Category = "Inspect")
	float RotationSpeed = 4.f;

	// In Drag_Component.h
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelf")
	float ShelfSpeed = 4.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelf")
	float ShelfMaxDistance = 50.f;  // How far shelf can pull out

	/** Local-space direction in which the cupboard panel opens.
	 *  Use (0, -1, 0) for right-to-left or (0, 1, 0) for left-to-right. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CupBoard")
	FVector CupBoardSlideAxis = FVector(0.0f, -1.0f, 0.0f);

	/** Mouse sensitivity for the sliding cupboard panel. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CupBoard", meta = (ClampMin = "0.0"))
	float CupBoardSlideSpeed = 4.0f;

	/** Maximum distance the cupboard panel may travel from its closed position. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CupBoard", meta = (ClampMin = "0.0"))
	float CupBoardMaxDistance = 100.0f;

	/** Initial relative mesh location captured at BeginPlay and used as the closed pose. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "CupBoard")
	FVector CupBoardClosedLocation = FVector::ZeroVector;

};
