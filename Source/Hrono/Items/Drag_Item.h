// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/Drag_Component.h"
#include "Items/Base_Item.h"
#include "Engine/Engine.h"

#include "Drag_Item.generated.h"

/** Broadcast whenever the door finishes closing (true) or starts to open (false).
 *  Fires on the server and on every client so gameplay/UI can react to door state. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDoorStateChanged, bool, bIsClosed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShelfStateChanged);

UCLASS()
class HRONO_API ADrag_Item : public ABase_Item
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADrag_Item();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* FrameMesh;

	/*UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* DoorMesh;*/

	UPROPERTY(VisibleAnywhere)
	class UDrag_Component* DragComponent;

	virtual void UpdateMeshForLocalPlayer() override;

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

	/** Yaw (in degrees) at or under which the panel is treated as fully closed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door")
	float DoorClosedYawTolerance = 1.0f;

	/** Authority-only: recomputes bIsClosed from the door's current Yaw and
	 *  broadcasts OnDoorStateChanged when the open/closed state changes. */
	void RefreshDoorClosedState();

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
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Shelf")
	FVector ShelfPosition = FVector::ZeroVector;

	UFUNCTION(BlueprintCallable, Category = "Shelf")  // Changed this line
		void RefreshShelfOpenState();


protected:
	UFUNCTION(BlueprintCallable, Category = "Shelf")
	void OnShelfOpened();

	UFUNCTION(BlueprintCallable, Category = "Shelf")
	void OnShelfClosed();

	UFUNCTION(BlueprintCallable, Category = "Shelf")
	void UpdateShelfCollision();

};
