#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GravityAnomalyComponent.generated.h"

class ABase_Item;
class ARoom;
class UPrimitiveComponent;

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

	/** One-shot hook. bReleased=false at start; true when the attempt ends, with or without an impulse. */
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

	/** Start the fall/freeze/sideways impulse sequence when a tagged item is dropped inside the cursed room. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Anomaly|Settings")
	bool bReactToItemDrop = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Anomaly|Settings", meta = (ClampMin = "1"))
	int32 MinAffectedObjects = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Anomaly|Settings", meta = (ClampMin = "1"))
	int32 MaxAffectedObjects = 3;

	/** Earliest time after the drop at which a still-falling item can freeze. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Anomaly|Movement", meta = (ClampMin = "0.0"))
	float FallBeforePause = 0.2f;

	/** Latest possible freeze time. Each attempt chooses once in this range. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Anomaly|Movement", meta = (ClampMin = "0.0"))
	float MaxFallBeforePause = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Anomaly|Movement", meta = (ClampMin = "0.0"))
	float PauseDuration = 0.1f;

	/** Sideways velocity change in cm/s when gravity resumes; mass independent. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Anomaly|Movement", meta = (ClampMin = "0.0"))
	float SideImpulseSpeed = 180.0f;

	/** Extra downward velocity change in cm/s to make the renewed fall readable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Anomaly|Movement", meta = (ClampMin = "0.0"))
	float DownwardImpulseSpeed = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Anomaly|Strong Event", meta = (ClampMin = "1.0"))
	float StrongImpulseMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Anomaly|Strong Event", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StrongEventChance = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Anomaly|Strong Event", meta = (ClampMin = "3", ClampMax = "6"))
	int32 StrongEventObjectCount = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Anomaly|Eligibility")
	FName AllowedActorTag = TEXT("GravityAnomaly");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Anomaly|Debug")
	bool bDebug = false;

private:
	struct FActiveProp
	{
		TWeakObjectPtr<ABase_Item> Item;
		FVector SideDirection = FVector::ForwardVector;
		float Elapsed = 0.0f;
		float FallDelay = 0.2f;
		bool bStrong = false;
		bool bPaused = false;
		bool bLanded = false;
		bool bWasFalling = false;
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

	void RefreshCandidates();
	void HandleItemDropped(ABase_Item* Item);
	bool IsInsideRoom(const ABase_Item* Item) const;
	bool IsEligible(const ABase_Item* Item) const;
	void ScheduleNextEvent();
	void TriggerEvent();
	bool StartProp(ABase_Item* Item, bool bStrong);
	void ReleaseProp(int32 Index, bool bPreserveMomentum = false);
	void StopAll();
	void SyncActivity();
};
