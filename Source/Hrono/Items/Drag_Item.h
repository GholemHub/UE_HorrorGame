// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/Drag_Component.h"
#include "Items/Base_Item.h"
#include "Engine/Engine.h"
#include "HronoSharedTools.h"

#include "Drag_Item.generated.h"


/** Broadcast whenever the door finishes closing (true) or starts to open (false).
 *  Fires on the server and on every client so gameplay/UI can react to door state. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDoorStateChanged, bool, bIsClosed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShelfStateChanged);

/** Broadcast when the player starts dragging this item.
 *  bIsShelf is true for a shelf drag and false for a door drag. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDragStarted, bool, bIsShelf);

class USoundBase;
class UAudioComponent;
class USplineComponent;
class UPrimitiveComponent;

UCLASS()
class HRONO_API ADrag_Item : public ABase_Item
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Timeline")
	EItemTimeline GetItemTimeline() const
	{
		return ItemTimeline;
	}

public:	
	// Sets default values for this actor's properties
	ADrag_Item();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	USceneComponent* SceneRoot;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* FrameMesh;

	/*UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* DoorMesh;*/

	UPROPERTY(VisibleAnywhere)
	class UDrag_Component* DragComponent;

	/** Editable set of points (spline control points) used to author a path/positions
	 *  for this drag item. Points can be edited directly in the viewport and are
	 *  exposed to Blueprints. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* PointSet;

	virtual void UpdateMeshForLocalPlayer() override;

	// =========================================================
	// AUDIO (placeholder sounds — assign any sound in Blueprint)
	// =========================================================

	/** Runtime looping audio source used for the door/shelf movement sound. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
	UAudioComponent* MoveAudioComponent;

	/** Played once when the door finishes opening. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Door")
	TObjectPtr<USoundBase> DoorOpenSound;

	/** Played once when the door finishes closing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Door")
	TObjectPtr<USoundBase> DoorCloseSound;

	/** Looping sound played while the door is being moved/dragged. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Door")
	TObjectPtr<USoundBase> DoorMoveSound;

	/** Played once when the shelf finishes opening. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Shelf")
	TObjectPtr<USoundBase> ShelfOpenSound;

	/** Played once when the shelf finishes closing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Shelf")
	TObjectPtr<USoundBase> ShelfCloseSound;

	/** Looping sound played while the shelf is being moved/dragged. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Shelf")
	TObjectPtr<USoundBase> ShelfMoveSound;

	/** Starts the looping move sound. Pass true for the shelf sound, false for the door sound. */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void StartMoveSound(bool bShelf);

	/** Stops the looping move sound. */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void StopMoveSound();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Server-authoritative door panel rotation. Replicated so every machine
	 *  (especially the server that validates movement) keeps the door's collision
	 *  geometry in the same open/closed state as the player who opened it. */
	UPROPERTY(ReplicatedUsing = OnRep_DoorRotation)
	FRotator DoorRotation;

	UFUNCTION()
	void OnRep_DoorRotation();

	UPROPERTY(ReplicatedUsing = OnRep_IsClosed)
	bool bIsClosed = true;
	UFUNCTION()
	void OnRep_IsClosed();

	/** Fired on the server and on clients whenever the door finishes opening or closing. */
	UPROPERTY(BlueprintAssignable, Category = "Door")
	FOnDoorStateChanged OnDoorStateChanged;

	/** Fired when the player starts dragging this item (door or shelf).
	 *  Subscribe in Blueprints to react to the start of a drag interaction. */
	UPROPERTY(BlueprintAssignable, Category = "Drag")
	FOnDragStarted OnDragStarted;

	/** Broadcasts OnDragStarted. Called by UDrag_Component when a drag begins. */
	void NotifyDragStarted(bool bShelf);

	/** Yaw (in degrees) at or under which the panel is treated as fully closed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door")
	float DoorClosedYawTolerance = 1.0f;

	/** Opens or closes the door with a smooth ease-in/ease-out animation.
	 *  Call this on the server (for example from an authoritative trigger event).
	 *  The animation is multicast so every player sees the panel moving. */
	UFUNCTION(BlueprintCallable, Category = "Door|Animation", meta = (DisplayName = "Animate Door Open/Close"))
	virtual void AnimateDoor(bool bOpen);

	/**
	 * When false, Animate Door Open/Close ignores every request. Manual player
	 * dragging remains available because it does not use AnimateDoor.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Animation")
	bool bAllowAnimateDoorOpenClose = true;

	/** Absolute yaw angle used by AnimateDoor when opening. The direction is
	 *  selected automatically from ItemType (InvertLeft opens in +Yaw). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Animation", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "180.0"))
	float AnimatedDoorOpenAngle = 90.0f;

	/** Time, in seconds, required to fully open or close the door. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Animation", meta = (ClampMin = "0.01", UIMin = "0.1"))
	float DoorAnimationDuration = 1.25f;

	/** Shape of the ease-in/ease-out motion. 1 is linear; 2-3 feels heavier. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Animation", meta = (ClampMin = "1.0", UIMin = "1.0", UIMax = "5.0"))
	float DoorAnimationEaseExponent = 2.0f;

	/** Optional legacy per-frame rotation panel. Disabled by default to avoid every
	 *  placed door overwriting the same two on-screen debug keys. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Debug")
	bool bShowDoorDebugOnScreen = false;

	/** Authority-only: recomputes bIsClosed from the door's current Yaw and
	 *  broadcasts OnDoorStateChanged when the open/closed state changes. */
	virtual void RefreshDoorClosedState();

	/** Component rotated by the primary DragComponent. Subclasses can provide a
	 *  dedicated hinge pivot while ItemMesh remains the clickable door mesh. */
	virtual USceneComponent* GetPrimaryDoorMovementComponent() const;

	/** Selects the correct drag component for the primitive hit by the interaction trace. */
	virtual UDrag_Component* FindDragComponentForHit(const UPrimitiveComponent* HitComponent) const;

	/** Resolves a replicated door/pivot identifier to a movement component. */
	virtual USceneComponent* FindDoorMovementComponent(FName DoorComponentName) const;

	/** Authority-only entry point used by Character RPCs for primary or subclass door panels. */
	virtual void ApplyDoorRotationFromServer(FName DoorComponentName, const FRotator& NewRotation);

	/** Resolves a replicated shelf/drawer identifier to its movement component. */
	virtual USceneComponent* FindShelfMovementComponent(FName ShelfComponentName) const;

	/** Authority-only entry point used by Character RPCs for one shelf/drawer panel. */
	virtual void ApplyShelfPositionFromServer(FName ShelfComponentName, const FVector& NewPosition);

	// In Drag_Item.h
public:
	UPROPERTY(BlueprintAssignable, Category = "Shelf")
	FOnShelfStateChanged OnShelfOpen;

	UPROPERTY(BlueprintAssignable, Category = "Shelf")
	FOnShelfStateChanged OnShelfClose;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Shelf")
	bool bIsShelfOpen = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelf")
	float ShelfMaxDistance = 50.f;

	// In Drag_Item.h
public:
	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_ShelfPosition, Category = "Shelf")
	FVector ShelfPosition = FVector::ZeroVector;

	UFUNCTION()
	void OnRep_ShelfPosition();

	UFUNCTION(BlueprintCallable, Category = "Shelf")  // Changed this line
		void RefreshShelfOpenState();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelf")
	ABase_Item* KeyActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Shelf")
	bool bNeedKeyActor = false;

	/** Key tag required to unlock this actor. Leave empty to accept any Item.Key. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Door|Lock",
		meta = (EditCondition = "bNeedKeyActor", GameplayTagFilter = "Item.Key"))
	FGameplayTag RequiredKeyTag;

	/** Returns the configured key tag, or Item.Key for legacy doors. */
	UFUNCTION(BlueprintPure, Category = "Door|Lock")
	FGameplayTag GetRequiredKeyTag() const;

	/** Checks whether an item carries the tag required by this lock. */
	UFUNCTION(BlueprintPure, Category = "Door|Lock")
	bool CanUnlockWithItem(const ABase_Item* Item) const;


protected:
	/** Finds the drag settings that own a particular movement component. */
	UDrag_Component* FindDragComponentForMovementComponent(const USceneComponent* MovementComponent) const;

	/** Constrains an untrusted client position to this component's authored slide path. */
	FVector ClampShelfPositionForComponent(
		const USceneComponent* MovementComponent,
		const FVector& RequestedPosition) const;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStartDoorAnimation(FRotator TargetRotation, float Duration);

	void UpdateDoorAnimation(float DeltaTime);

	UPROPERTY(Transient)
	bool bDoorAnimationActive = false;

	UPROPERTY(Transient)
	float DoorAnimationElapsed = 0.0f;

	UPROPERTY(Transient)
	float ActiveDoorAnimationDuration = 1.0f;

	UPROPERTY(Transient)
	FRotator DoorAnimationStartRotation = FRotator::ZeroRotator;

	UPROPERTY(Transient)
	FRotator DoorAnimationTargetRotation = FRotator::ZeroRotator;

	UFUNCTION(BlueprintCallable, Category = "Shelf")
	void OnShelfOpened();

	UFUNCTION(BlueprintCallable, Category = "Shelf")
	void OnShelfClosed();

	UFUNCTION(BlueprintCallable, Category = "Shelf")
	void UpdateShelfCollision();

};
