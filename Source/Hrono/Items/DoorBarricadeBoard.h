#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HronoSharedTools.h"
#include "Interface/Enviroment_Interface.h"
#include "DoorBarricadeBoard.generated.h"

class ADrag_Item;
class AHronoCharacter;
class UGeometryCollectionComponent;
class USceneComponent;
class USoundBase;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FDoorBarricadeBrokenSignature,
	AHronoCharacter*, BreakingCharacter,
	ADrag_Item*, BlockedDoor);

/**
 * One replicated board that blocks an associated draggable door in its own
 * timeline. Pressing Interact while holding AxeItem breaks the board.
 */
UCLASS(BlueprintType, Blueprintable)
class HRONO_API ADoorBarricadeBoard : public AActor, public IEnviroment_Interface
{
	GENERATED_BODY()

public:
	ADoorBarricadeBoard();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Barricade|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	/** Assign the board mesh in a Blueprint child. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Barricade|Components")
	TObjectPtr<UStaticMeshComponent> BoardMesh;

	/** Hidden fractured version of BoardMesh. It is activated when the axe breaks the board. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Barricade|Components")
	TObjectPtr<UGeometryCollectionComponent> DestructibleBoard;

	/** Timeline in which this board is visible, solid, and blocks its door. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_BoardTimeline,
		Category = "Barricade|Timeline")
	EItemTimeline BoardTimeline = EItemTimeline::Both;

	/**
	 * Door blocked by this board. If empty on the server, the nearest suitable
	 * Drag_Item inside AutoFindDoorRadius is selected at BeginPlay.
	 */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_BlockedDoor,
		Category = "Barricade|Door")
	TObjectPtr<ADrag_Item> BlockedDoor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barricade|Door",
		meta = (ClampMin = "0.0", Units = "cm"))
	float AutoFindDoorRadius = 250.0f;

	/** Maximum server-authoritative distance from player to board when breaking it. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barricade|Interaction",
		meta = (ClampMin = "0.0", Units = "cm"))
	float BreakDistance = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barricade|Audio")
	TObjectPtr<USoundBase> BreakSound;

	/** Time before the broken board actor and its debris are cleaned up. Zero keeps them forever. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barricade|Destruction",
		meta = (ClampMin = "0.0", Units = "s"))
	float DebrisLifetime = 8.0f;

	UPROPERTY(BlueprintAssignable, Category = "Barricade|Events")
	FDoorBarricadeBrokenSignature OnBarricadeBroken;

	UFUNCTION(BlueprintPure, Category = "Barricade")
	bool IsBroken() const { return bBroken; }

	/** Authority-only runtime reassignment. Passing null enables another nearest-door search. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Barricade|Door")
	void SetBlockedDoor(ADrag_Item* NewDoor);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void OnRep_BoardTimeline();

	UFUNCTION()
	void OnRep_BlockedDoor();

	UFUNCTION()
	void OnRep_Broken();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastBoardBroken(AHronoCharacter* BreakingCharacter, ADrag_Item* DoorThatWasBlocked);

private:
	ADrag_Item* FindNearestDoor() const;
	void RegisterWithDoor();
	void UnregisterFromDoor();
	void ApplyTimelineCollision();
	void UpdateLocalVisibility();
	void ActivateChaosDestruction();
	bool DoesTimelineMatch(EItemTimeline OtherTimeline) const;

	bool bRegisteredWithDoor = false;
	bool bChaosDestructionActivated = false;

	UPROPERTY(ReplicatedUsing = OnRep_Broken)
	bool bBroken = false;
};
