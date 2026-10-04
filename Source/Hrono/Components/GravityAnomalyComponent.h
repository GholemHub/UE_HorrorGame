#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HronoSharedTools.h"
#include "GravityAnomalyComponent.generated.h"

class ABase_Item;
class ARoom;
class UPrimitiveComponent;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGravityAnomalyActivitySignature, bool, bActive);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FGravityAnomalyPropSignature, ABase_Item*, Item, bool, bReleased, bool, bStrongEvent);

/** Server-controlled, room-scoped gravity evidence for explicitly tagged loose pickups. */
UCLASS(ClassGroup = (Hrono), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class HRONO_API UGravityAnomalyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGravityAnomalyComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Runtime toggle. The owning Room must also be cursed. Server only. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Gravity Anomaly")
	void SetEnabled(bool bNewEnabled);

	UFUNCTION(BlueprintPure, Category = "Gravity Anomaly")
	bool IsCursedRoom() const;

	UFUNCTION(BlueprintPure, Category = "Gravity Anomaly")
	bool IsAnomalyActive() const { return ActiveItems.Num() > 0; }

	/** Current snapshot for attaching local loops, including on a late-joining client. */
	UFUNCTION(BlueprintPure, Category = "Gravity Anomaly")
	TArray<ABase_Item*> GetActiveItems() const;

	/** State hook for a sustained local audio/VFX loop, including late join and cleanup. */
	UPROPERTY(BlueprintAssignable, Category = "Gravity Anomaly|Events")
	FGravityAnomalyActivitySignature OnActivityChanged;

	/** One-shot hook. bReleased=false at start; true when the altered fall ends. */
	UPROPERTY(BlueprintAssignable, Category = "Gravity Anomaly|Events")
	FGravityAnomalyPropSignature OnPropPhaseChanged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Anomaly|Settings")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Anomaly|Settings", meta = (ClampMin = "0.1"))
	float MinEventInterval = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Anomaly|Settings", meta = (ClampMin = "0.1"))
	float MaxEventInterval = 30.0f;

	/** Probability that a scheduled opportunity becomes an event. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Anomaly|Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EventChance = 0.85f;

	/** Apply altered gravity immediately when a tagged item is dropped inside the cursed room. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Anomaly|Settings")
	bool bReactToItemDrop = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Anomaly|Settings", meta = (ClampMin = "1"))
	int32 MinAffectedObjects = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Anomaly|Settings", meta = (ClampMin = "1"))
	int32 MaxAffectedObjects = 3;

	/** Stable acceleration for one fall, sampled from one of these two non-normal bands. m/s². */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Anomaly|Movement", meta = (ClampMin = "8.5", ClampMax = "10.5", Units = "m/s^2"))
	float MinSlowGravity = 8.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Anomaly|Movement", meta = (ClampMin = "8.5", ClampMax = "10.5", Units = "m/s^2"))
	float MaxSlowGravity = 8.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Anomaly|Movement", meta = (ClampMin = "8.5", ClampMax = "10.5", Units = "m/s^2"))
	float MinFastGravity = 10.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Anomaly|Movement", meta = (ClampMin = "8.5", ClampMax = "10.5", Units = "m/s^2"))
	float MaxFastGravity = 10.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Anomaly|Movement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SlowEventChance = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Anomaly|Strong Event", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StrongEventChance = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Anomaly|Strong Event", meta = (ClampMin = "3", ClampMax = "6"))
	int32 StrongEventObjectCount = 4;

	/** Rare slow fall may become 10.5 m/s² after a player has seen it and looks away. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Anomaly|Horror")
	bool bEnableUnseenDrop = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Anomaly|Horror", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float UnseenDropChance = 0.05f;
	/** Optional heavy impact. Falls back to the affected item's DropSound if empty. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Anomaly|Horror")
	TObjectPtr<USoundBase> UnseenImpactSound;

	/** Server-only diagnostic value; zero means the item is not in an active fall. */
	UFUNCTION(BlueprintPure, Category = "Gravity Anomaly|Debug")
	float GetActiveGravityForItem(const ABase_Item* Item) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Anomaly|Eligibility")
	FName AllowedActorTag = TEXT("GravityAnomaly");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Anomaly|Debug")
	bool bDebug = false;

private:
	struct FActiveProp
	{
		TWeakObjectPtr<ABase_Item> Item;
		float TargetGravity = 9.81f;
		bool bStrong = false;
		bool bLanded = false;
		bool bWasFalling = false;
		bool bHorrorCandidate = false;
		bool bWasSeen = false;
		float SeenDuration = 0.0f;
		bool bUnseenDrop = false;
		bool bWasNotifyRigidBodyCollision = false;
	};

	UPROPERTY(ReplicatedUsing = OnRep_ActiveItems)
	TArray<TObjectPtr<ABase_Item>> ActiveItems;

	TSet<TWeakObjectPtr<ABase_Item>> Candidates;
	TArray<FActiveProp> ActiveProps;
	FTimerHandle NextEventTimer;
	FDelegateHandle ItemDropHandle;
	bool bLocalActivity = false;

	UFUNCTION()
	void OnRep_ActiveItems();

	UFUNCTION()
	void HandleCursedStateChanged(ARoom* Room, bool bCursed);

	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

	UFUNCTION()
	void HandlePropHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPropPhase(ABase_Item* Item, bool bReleased, bool bStrongEvent);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastUnseenImpact(ABase_Item* Item, EItemTimeline EventTimeline,
		FVector_NetQuantize Location);

	void RefreshCandidates();
	void HandleItemDropped(ABase_Item* Item);
	bool IsInsideRoom(const ABase_Item* Item) const;
	bool IsEligible(const ABase_Item* Item) const;
	bool IsWatchedByPlayer(const ABase_Item* Item) const;
	void ScheduleNextEvent();
	void TriggerEvent();
	bool StartProp(ABase_Item* Item, bool bStrong);
	void ReleaseProp(int32 Index);
	void StopAll();
	void SyncActivity();
};
