#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DoorLockTrigger.generated.h"

class ADrag_Item;
class UBoxComponent;
class UPrimitiveComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDoorLockTriggerEvent);

/**
 * Closes and locks a configured group of doors when a Hrono player overlaps the
 * box. UnlockTriggeredDoors permanently releases only the locks owned by this
 * trigger, leaving key locks and locks from other triggers untouched.
 */
UCLASS(BlueprintType, Blueprintable)
class HRONO_API ADoorLockTrigger : public AActor
{
	GENERATED_BODY()

public:
	ADoorLockTrigger();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Collision volume that activates the trigger. Resize it in the level viewport. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door Lock Trigger|Components")
	TObjectPtr<UBoxComponent> TriggerBox;

	/** Placed doors that this trigger will close and lock. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Door Lock Trigger",
		meta = (DisplayName = "Doors"))
	TArray<TObjectPtr<ADrag_Item>> Doors;

	/** Smoothly closes each configured door as soon as the trigger activates. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Lock Trigger")
	bool bCloseDoorsWhenTriggered = true;

	/**
	 * Prevents later overlaps from locking the doors again. On Triggered is still
	 * sent for every valid character overlap so it can drive Blueprint gameplay.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Lock Trigger")
	bool bTriggerOnlyOnce = true;

	/**
	 * Locks the configured doors when the gameplay map begins and releases them
	 * permanently once HronoGameMode sees all required players on this map.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Lock Trigger|Session Start")
	bool bLockUntilAllPlayersPresent = false;

	/** True after a player has activated this trigger at least once. */
	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Door Lock Trigger|State")
	bool bHasTriggered = false;

	/** Actor responsible for the most recent successful activation. */
	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Door Lock Trigger|State")
	TObjectPtr<AActor> LastTriggeringActor;

	/**
	 * Unlock flag. Once true, this actor no longer locks doors, including on future
	 * overlaps. Set it through Unlock Triggered Doors so existing locks are released.
	 */
	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Door Lock Trigger|State")
	bool bDoorsUnlocked = false;

	/** Doors that received a lock during activation. Invalid entries are omitted. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Door Lock Trigger|State")
	TArray<TObjectPtr<ADrag_Item>> TriggeredDoors;

	/** Manually performs the same action as entering TriggerBox. Server only. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Door Lock Trigger")
	bool ActivateDoorLock();

	/**
	 * Activates the lock and supplies the actor that caused it. TriggerBox calls
	 * this automatically with the overlapping HronoCharacter.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Door Lock Trigger",
		meta = (DisplayName = "Activate Door Lock (With Triggering Actor)"))
	bool ActivateDoorLockWithActor(AActor* TriggeringActor);

	/**
	 * Releases all locks created by this trigger and permanently sets the unlock
	 * flag. Call this Blueprint method when the player is allowed to return.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Door Lock Trigger")
	bool UnlockTriggeredDoors();

	UPROPERTY(BlueprintAssignable, Category = "Door Lock Trigger|Events")
	FDoorLockTriggerEvent OnDoorsTriggered;

	UPROPERTY(BlueprintAssignable, Category = "Door Lock Trigger|Events")
	FDoorLockTriggerEvent OnDoorsUnlocked;

	/**
	 * Automatically appears as Event On Triggered in a Blueprint derived from this
	 * actor. It runs on the server and every connected client.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Door Lock Trigger|Events",
		meta = (DisplayName = "On Triggered"))
	void OnTriggered(AActor* TriggeringActor);

private:
	UFUNCTION()
	void HandleTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastNotifyTriggered(AActor* TriggeringActor);

	bool LockConfiguredDoors();
	void ReleaseOwnedLocks();
};
