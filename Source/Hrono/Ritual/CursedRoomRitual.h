#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HronoSharedTools.h"
#include "CursedRoomRitual.generated.h"

class ABase_Item;
class ADrag_Item;
class ALight_Env;
class ARitualGoatSkull;
class ARoom;
class ASwitcher_Env;
class ULightComponentBase;
class UPrimitiveComponent;
class USceneComponent;
class USoundBase;

UENUM(BlueprintType)
enum class ECursedRoomRitualState : uint8
{
	Idle,
	Preparing,
	Rising,
	Hovering,
	Scratching,
	FallingSilent,
	Completed,
	Failed
};

UENUM(BlueprintType)
enum class ERitualHouseLightMode : uint8
{
	Normal,
	Flickering,
	Blackout
};

/** Replicated presentation state. The cursed-room answer remains server-only. */
USTRUCT(BlueprintType)
struct HRONO_API FCursedRoomRitualReplicatedState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cursed Room Ritual")
	ECursedRoomRitualState State = ECursedRoomRitualState::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cursed Room Ritual")
	int32 SequenceId = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cursed Room Ritual")
	double StartServerTime = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cursed Room Ritual")
	float Duration = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cursed Room Ritual")
	int32 RandomSeed = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cursed Room Ritual")
	TObjectPtr<ARitualGoatSkull> PastSkull = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cursed Room Ritual")
	TObjectPtr<ARitualGoatSkull> FutureSkull = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cursed Room Ritual")
	FTransform PastStartTransform = FTransform::Identity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cursed Room Ritual")
	FTransform FutureStartTransform = FTransform::Identity;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FCursedRoomRitualStateChangedSignature,
	ECursedRoomRitualState, PreviousState,
	ECursedRoomRitualState, NewState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FWrongRoomRitualSignature,
	ARoom*, TestedRoom,
	ARitualGoatSkull*, PastSkull,
	ARitualGoatSkull*, FutureSkull);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FCursedRoomRitualCompletedSignature,
	ABase_Item*, PastKey,
	ABase_Item*, FutureKey);

/**
 * Invisible server-authoritative coordinator. One Past and one Future skull
 * automatically start the test when both have been dropped into the same ARoom.
 */
UCLASS(BlueprintType, Blueprintable)
class HRONO_API ACursedRoomRitual : public AActor
{
	GENERATED_BODY()

public:
	ACursedRoomRitual();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "Cursed Room Ritual", meta = (WorldContext = "WorldContextObject"))
	static ACursedRoomRitual* FindRitual(const UObject* WorldContextObject);

	/** Called by ARitualGoatSkull::Drop on the server. */
	void NotifySkullDropped(ARitualGoatSkull* DroppedSkull);

	UFUNCTION(BlueprintPure, Category = "Cursed Room Ritual")
	ECursedRoomRitualState GetRitualState() const { return ReplicatedState.State; }

	UFUNCTION(BlueprintPure, Category = "Cursed Room Ritual|Debug", meta = (DevelopmentOnly))
	FString GetRitualDebugStatus() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cursed Room Ritual|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Sequence",
		meta = (ClampMin = "0.0", Units = "s"))
	float PreparationDuration = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Sequence",
		meta = (ClampMin = "0.1", Units = "s"))
	float RiseDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Sequence",
		meta = (ClampMin = "0.0", Units = "cm"))
	float HoverHeight = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Sequence",
		meta = (ClampMin = "0.0", Units = "s"))
	float HoverDuration = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Sequence",
		meta = (ClampMin = "0.1", Units = "s"))
	float ScratchDuration = 7.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Failure",
		meta = (ClampMin = "0.0", Units = "s"))
	float WrongRoomFallingSilenceDuration = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Physics",
		meta = (ClampMin = "0.0", Units = "cm/s^2"))
	float CeilingHoldAcceleration = 980.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Physics",
		meta = (ClampMin = "0.05", Units = "s"))
	float RandomImpulseInterval = 0.25f;

	/** Velocity change, so differently scaled skulls react similarly. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Physics",
		meta = (ClampMin = "0.0", Units = "cm/s"))
	float RandomImpulseStrength = 450.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Sequence",
		meta = (ClampMin = "0.01", Units = "s"))
	float PhysicsUpdateInterval = 0.033f;

	/** Optional separate classes; both may point to the same existing key Blueprint. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Keys")
	TSubclassOf<ABase_Item> PastKeyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Keys")
	TSubclassOf<ABase_Item> FutureKeyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Keys")
	FVector KeySpawnOffset = FVector(0.0f, 0.0f, -10.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Keys",
		meta = (ClampMin = "0.1", Units = "s"))
	float KeyLandingTimeout = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Consequences",
		meta = (ClampMin = "0.0"))
	float CorrectRoomThreatIncrease = 10.0f;

	/** 0.55 means 55 percent of the Hunt Director's MaxThreat. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Consequences",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WrongRoomThreatFraction = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Lights",
		meta = (ClampMin = "0.04", Units = "s"))
	float HouseFlickerIntervalMin = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Lights",
		meta = (ClampMin = "0.04", Units = "s"))
	float HouseFlickerIntervalMax = 0.20f;

	/**
	 * Played once whenever the whole-house flicker changes from an ON pulse to an
	 * OFF pulse. This is intentionally one 2D sound per pulse, rather than one
	 * overlapping sound for every lamp in the house.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Lights|Audio")
	TObjectPtr<USoundBase> FlickerOffSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Debug")
	bool bDebugRitual = false;

	UPROPERTY(ReplicatedUsing = OnRep_RitualState, VisibleInstanceOnly, BlueprintReadOnly,
		Category = "Cursed Room Ritual|Runtime")
	FCursedRoomRitualReplicatedState ReplicatedState;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Cursed Room Ritual|Runtime")
	TObjectPtr<ABase_Item> SpawnedPastKey;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Cursed Room Ritual|Runtime")
	TObjectPtr<ABase_Item> SpawnedFutureKey;

	UPROPERTY(ReplicatedUsing = OnRep_HouseLightMode, VisibleInstanceOnly, BlueprintReadOnly,
		Category = "Cursed Room Ritual|Runtime")
	ERitualHouseLightMode HouseLightMode = ERitualHouseLightMode::Normal;

	UPROPERTY(BlueprintAssignable, Category = "Cursed Room Ritual|Events")
	FCursedRoomRitualStateChangedSignature OnRitualStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Cursed Room Ritual|Events")
	FWrongRoomRitualSignature OnWrongRoomRitual;

	UPROPERTY(BlueprintAssignable, Category = "Cursed Room Ritual|Events")
	FCursedRoomRitualCompletedSignature OnRitualCompleted;

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursed Room Ritual|Events")
	void BP_OnRitualStarted(ARitualGoatSkull* PastSkull, ARitualGoatSkull* FutureSkull);

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursed Room Ritual|Events")
	void BP_OnWrongRoom(ARoom* TestedRoom, ARitualGoatSkull* PastSkull, ARitualGoatSkull* FutureSkull);

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursed Room Ritual|Events")
	void BP_OnSkullsStartFloating(ARitualGoatSkull* PastSkull, ARitualGoatSkull* FutureSkull);

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursed Room Ritual|Events")
	void BP_OnSkullsStartHovering(ARitualGoatSkull* PastSkull, ARitualGoatSkull* FutureSkull);

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursed Room Ritual|Events")
	void BP_OnSkullsStartScratching(ARitualGoatSkull* PastSkull, ARitualGoatSkull* FutureSkull);

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursed Room Ritual|Events")
	void BP_OnRitualCompleted(ABase_Item* PastKey, ABase_Item* FutureKey);

protected:
	UFUNCTION()
	void OnRep_RitualState();

	UFUNCTION()
	void OnRep_HouseLightMode();

	UFUNCTION()
	void HandleSpawnedKeyHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse,
		const FHitResult& Hit);

private:
	ARoom* FindContainingRoom(const AActor* Actor) const;
	bool FindSkullPairInRoom(ARoom*& OutRoom, ARitualGoatSkull*& OutPast, ARitualGoatSkull*& OutFuture) const;
	bool IsDroppedSkullValid(const ARitualGoatSkull* Skull, EItemTimeline RequiredTimeline) const;
	void StartRitual(ARoom* TestedRoom, ARitualGoatSkull* PastSkull, ARitualGoatSkull* FutureSkull);
	void SetState(ECursedRoomRitualState NewState, float Duration);
	void HandleStateFinished();
	void ApplyReplicatedState();
	void UpdateActiveStage();
	void StartStageUpdates();
	void StopStageUpdates();
	void ApplyRiseTransform(ARitualGoatSkull* Skull, const FTransform& StartTransform, float Alpha) const;
	void ApplyUpwardAcceleration(ARitualGoatSkull* Skull, float Acceleration) const;
	void ApplyRandomImpulse(ARitualGoatSkull* Skull, int32 ImpulseIndex, int32 TimelineSalt) const;
	ABase_Item* SpawnTimelineKey(TSubclassOf<ABase_Item> KeyClass, EItemTimeline Timeline, ARitualGoatSkull* SourceSkull);
	void CompleteSuccessfulRitual();
	void FinishSuccessfulRitualAfterKeysLand();
	void TriggerWrongRoomConsequences();
	void UnlockActiveSkulls();
	void CloseAndLockTestedRoomDoors();
	void UnlockTestedRoomDoors();
	void SetHouseLightMode(ERitualHouseLightMode NewMode);
	void ApplyLocalHouseLightMode();
	void BeginLocalHouseFlicker();
	void EndLocalHouseFlicker(bool bRestorePreviousState);
	void UpdateLocalHouseFlicker();
	void ApplyLocalHouseBlackout();
	double GetSynchronizedServerTime() const;
	void DispatchStateEvents(ECursedRoomRitualState PreviousState);
	void LogStageEntered(ECursedRoomRitualState PreviousState) const;
	void LogStageFinished(ECursedRoomRitualState FinishedState) const;

	UPROPERTY(Transient)
	TObjectPtr<ARoom> ActiveTestedRoom;

	FTimerHandle StateTimerHandle;
	FTimerHandle StageUpdateTimerHandle;
	FTimerHandle HouseFlickerTimerHandle;
	FTimerHandle KeyLandingTimeoutHandle;
	ECursedRoomRitualState LastDispatchedState = ECursedRoomRitualState::Idle;
	int32 LastDispatchedSequenceId = INDEX_NONE;
	int32 LastAppliedImpulseIndex = INDEX_NONE;
	bool bSuccessfulConsequencesApplied = false;
	bool bLastHouseFlickerPulseOn = true;
	TSet<TWeakObjectPtr<ABase_Item>> LandedKeys;
	TArray<TWeakObjectPtr<ADrag_Item>> LockedRoomDoors;
	TMap<TWeakObjectPtr<ALight_Env>, bool> PreviousEnvironmentLightFlicker;
	TMap<TWeakObjectPtr<ULightComponentBase>, bool> PreviousGenericLightVisibility;
	TSet<TWeakObjectPtr<ASwitcher_Env>> FlickeringEmissiveSwitchers;
};
