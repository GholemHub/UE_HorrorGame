#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HronoSharedTools.h"
#include "Hunt/GhostHuntTypes.h"
#include "MannequinDemon.generated.h"

class AHronoCharacter;
class AScareDirector;
class ADrag_Item;
class ABase_Item;
class UStaticMesh;
class UStaticMeshComponent;
class USceneComponent;
struct FHitResult;

UENUM(BlueprintType)
enum class EMannequinMood : uint8
{
	Sad, Neutral, Happy
};

UENUM(BlueprintType)
enum class EMannequinState : uint8
{
	Dormant, Spawned, Observing, Stalking, Approaching, BehindPlayer,
	GrabWarning, Grab, Repelled, Contained, SubmissiveToBabai, Disabled,
	ApproachingDoor, WaitingAtDoor, SeekingItem, OfferingItem, HappyChase, GameOver
};

UENUM(BlueprintType)
enum class EMannequinObserver : uint8
{
	None,
	FutureDirect UMETA(DisplayName="Direct camera"),
	PastMonocle UMETA(DisplayName="Opposite timeline monocle"),
	Both
};

UENUM(BlueprintType)
enum class EMannequinSight : uint8
{
	NotEvaluated, NoViewer, WrongTimeline, NoMonocle, NoCapture,
	OutOfRange, OutsideView, Occluded, Visible
};

/** One server-driven stalker. Blueprint supplies appearance, audio and authored activation. */
UCLASS(Blueprintable)
class HRONO_API AMannequinDemon : public ACharacter
{
	GENERATED_BODY()

public:
	AMannequinDemon();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing=OnRep_State, Category="Mannequin")
	EMannequinState State = EMannequinState::Dormant;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing=OnRep_Targets, Category="Mannequin")
	TObjectPtr<AHronoCharacter> CurrentTarget;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing=OnRep_Targets, Category="Mannequin")
	TObjectPtr<AHronoCharacter> CurrentPartner;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing=OnRep_Targets, Category="Mannequin")
	EItemTimeline MannequinTimeline = EItemTimeline::Future;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing=OnRep_Mood, Category="Mannequin|Mood")
	EMannequinMood Mood = EMannequinMood::Sad;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing=OnRep_CarriedItem, Category="Mannequin|Mood")
	TObjectPtr<ABase_Item> CarriedItem;
	/** Position for a carried item. Set its socket/relative pose on BP_MannequinDemon. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Mannequin|Mood")
	TObjectPtr<USceneComponent> ItemHandPoint;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Mannequin|Mood")
	TObjectPtr<UStaticMeshComponent> MaskComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Mannequin|Mood")
	TObjectPtr<UStaticMesh> SadMaskMesh;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Mannequin|Mood")
	TObjectPtr<UStaticMesh> NeutralMaskMesh;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Mannequin|Mood")
	TObjectPtr<UStaticMesh> HappyMaskMesh;
	/** Skeletal bone or socket for MoodMask; defaults to the head bone. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Mood")
	FName MaskSocketName = TEXT("head");
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Mannequin|Mood")
	FName ItemHandSocketName = TEXT("hand_r");
	/** Eligible item classes configured on BP_MannequinDemon. Empty lists allow no items. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Mood|Items")
	TArray<TSubclassOf<ABase_Item>> AllowedPickupClasses;
	/** Optional exact level actors configured on a placed mannequin instance. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Mannequin|Mood|Items")
	TArray<TObjectPtr<ABase_Item>> AllowedPickupActors;
	bool IsMoodItemAllowed(const ABase_Item* Item) const;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Mood", meta=(ClampMin="0", Units="cm"))
	float ItemSearchDistance = 2000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Mood", meta=(ClampMin="0", Units="cm"))
	float ItemSearchTriggerDistance = 600.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Mood", meta=(ClampMin="0", Units="cm"))
	float ItemPickupDistance = 110.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Mood", meta=(ClampMin="0", Units="cm"))
	float OfferDistance = 300.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Mood", meta=(ClampMin="0.1", Units="s"))
	float OfferWaitSeconds = 60.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Mood", meta=(ClampMin="0", Units="cm"))
	float HappyKillDistance = 100.0f;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category="Mannequin|Observation")
	EMannequinObserver Observer = EMannequinObserver::None;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category="Mannequin|Observation", meta=(DisplayName="Direct sight"))
	EMannequinSight FutureSight = EMannequinSight::NotEvaluated;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category="Mannequin|Observation", meta=(DisplayName="Opposite timeline lens sight"))
	EMannequinSight PastMonocleSight = EMannequinSight::NotEvaluated;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category="Mannequin|Observation")
	FName FutureSightBlocker = NAME_None;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category="Mannequin|Observation")
	FName PastSightBlocker = NAME_None;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category="Mannequin|Stalking")
	TObjectPtr<ADrag_Item> BlockingDoor;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Mannequin|Control")
	bool ActivateMannequin();
	/** Optional server spawn entry for BP_ScareDirector or cursed-room logic; destroys failed spawns. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Mannequin|Control",
		meta=(WorldContext="WorldContextObject"))
	static AMannequinDemon* SpawnAndActivateMannequin(const UObject* WorldContextObject,
		TSubclassOf<AMannequinDemon> MannequinClass);
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Mannequin|Control")
	void DeactivateMannequin();
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Mannequin|Control")
	bool WakeMannequin();
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Mannequin|Control")
	bool ForceTarget(AHronoCharacter* NewTarget);
	/** QA control for server PIE; requires bDebugEnabled and an active mannequin. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Mannequin|Debug")
	bool ForceMoodForTesting(EMannequinMood NewMood);
	/** Server-only entry point for a trusted interaction (item remains owned by the caller). */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Mannequin|Containment")
	bool TryContainMannequin(AActor* ContainmentItem);
	UFUNCTION(BlueprintPure, Category="Mannequin|Observation")
	bool IsMannequinObservedByPlayer(const AHronoCharacter* Player) const;
	UFUNCTION(BlueprintPure, Category="Mannequin|Observation")
	bool IsPlayerObservingMannequinThroughMonocle(const AHronoCharacter* Player) const;
	UFUNCTION(BlueprintPure, Category="Mannequin|Warning")
	float GetGrabWarningProgress() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Observation", meta=(ClampMin="1", ClampMax="89"))
	float ObservationAngle = 37.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Observation", meta=(ClampMin="100", Units="cm"))
	float ObservationDistance = 1600.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Observation", meta=(ClampMin="0.05", Units="s"))
	float ObservationCheckInterval = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Observation", meta=(ClampMin="0", Units="s"))
	float ObservationGraceTime = 0.25f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Observation", meta=(ClampMin="1", ClampMax="45"))
	float MonocleHalfAngle = 45.0f;
	/** Circular aperture radius in the SceneCapture image, where 1 is the image edge. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Observation", meta=(ClampMin="0.1", ClampMax="1.0"))
	float MonocleApertureRadius = 0.85f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Observation", meta=(ClampMin="0", Units="cm"))
	float VisibilitySampleHeight = 60.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Stalking", meta=(ClampMin="100", Units="cm"))
	float MinSpawnDistance = 500.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Stalking", meta=(ClampMin="200", Units="cm"))
	float MaxSpawnDistance = 1100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Stalking", meta=(ClampMin="100", Units="cm"))
	float MinStalkDistance = 250.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Stalking", meta=(ClampMin="0", ClampMax="180"))
	float PreferredBehindAngle = 135.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Stalking", meta=(ClampMin="50", Units="cm/s"))
	float StalkingSpeed = 260.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Stalking", meta=(ClampMin="50", Units="cm/s"))
	float MoveSpeed = 410.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Stalking", meta=(ClampMin="1"))
	int32 RequiredStalkStepsBeforeGrab = 3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Stalking", meta=(ClampMin="0", Units="s"))
	float MinTimeBeforeGrabAttempt = 20.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Stalking", meta=(ClampMin="0.1", Units="s"))
	float ApproachCooldown = 4.0f;
	/** Desired clear space between the Mannequin and target capsules. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Stalking", meta=(ClampMin="0", Units="cm"))
	float ApproachClearance = 10.0f;
	/** Legacy path-length budget for the final behind-player move, not the stopping gap. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Stalking", meta=(ClampMin="80", Units="cm"))
	float BehindPlayerDistance = 145.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Warning", meta=(ClampMin="0.5", Units="s"))
	float GrabWarningDuration = 3.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Warning", meta=(ClampMin="0.5", Units="s"))
	float RescueWindowDuration = 2.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Warning")
	bool bSeeingMannequinDuringGrabWarningTriggersFinalGrab = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Cooldown", meta=(ClampMin="0", Units="s"))
	float PostGrabCooldown = 35.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Cooldown", meta=(ClampMin="0", Units="s"))
	float RepelledDelay = 12.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Cooldown", meta=(ClampMin="0", Units="s"))
	float TargetSwitchCooldown = 45.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Containment", meta=(ClampMin="1", Units="s"))
	float ContainmentDuration = 60.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Containment", meta=(ClampMin="0", Units="s"))
	float ContainmentWarningLeadTime = 8.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Containment", meta=(ClampMin="50", Units="cm"))
	float ContainmentInteractionDistance = 220.0f;
	/** Item must have this tag and be a world item; a held item is rejected. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Containment")
	FName ContainmentItemTag = TEXT("MannequinContainment");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Babai", meta=(ClampMin="0", Units="s"))
	float BabaiRecoveryDelay = 8.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Debug")
	bool bDebugEnabled = false;
	/** Legacy Blueprint setting; local timeline audience now controls visibility. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Debug")
	bool bShowDormantMeshForTesting = false;
	/** Server-side reason returned by the last failed ActivateMannequin call. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Mannequin|Debug")
	FString LastActivationFailure;
	/** Local only. Press L while playing to toggle the sight and AI overlay. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category="Mannequin|Debug")
	bool bLocalDebugOverlay = false;
	void ToggleLocalDebugOverlay();

	/** Snapshot hook: called on initial replication too; use it to start/stop persistent loops. */
	UFUNCTION(BlueprintImplementableEvent, Category="Mannequin|Events")
	void OnStateSnapshotApplied(EMannequinState NewState);
	UFUNCTION(BlueprintImplementableEvent, Category="Mannequin|Events") void OnSpawned();
	UFUNCTION(BlueprintImplementableEvent, Category="Mannequin|Events") void OnTargetChanged(AHronoCharacter* NewTarget, AHronoCharacter* NewPartner);
	UFUNCTION(BlueprintImplementableEvent, Category="Mannequin|Events") void OnFrozenByTarget();
	UFUNCTION(BlueprintImplementableEvent, Category="Mannequin|Events") void OnFrozenByMonocle();
	UFUNCTION(BlueprintImplementableEvent, Category="Mannequin|Events") void OnStalkingStarted();
	UFUNCTION(BlueprintImplementableEvent, Category="Mannequin|Events") void OnApproachingStarted();
	UFUNCTION(BlueprintImplementableEvent, Category="Mannequin|Events") void OnBehindPlayer();
	UFUNCTION(BlueprintImplementableEvent, Category="Mannequin|Events") void OnBreathingStarted();
	UFUNCTION(BlueprintImplementableEvent, Category="Mannequin|Events") void OnGrabWarningStarted();
	UFUNCTION(BlueprintImplementableEvent, Category="Mannequin|Events") void OnGrabWarningProgress(float Progress);
	UFUNCTION(BlueprintImplementableEvent, Category="Mannequin|Events") void OnGrabStarted();
	UFUNCTION(BlueprintImplementableEvent, Category="Mannequin|Events") void OnGrabCancelled();
	UFUNCTION(BlueprintImplementableEvent, Category="Mannequin|Events") void OnPlayerCaptured(AHronoCharacter* CapturedPlayer);
	UFUNCTION(BlueprintImplementableEvent, Category="Mannequin|Events") void OnRepelled();
	UFUNCTION(BlueprintImplementableEvent, Category="Mannequin|Events") void OnRepelledByMonocle();
	UFUNCTION(BlueprintImplementableEvent, Category="Mannequin|Events") void OnRescueSucceeded();
	UFUNCTION(BlueprintImplementableEvent, Category="Mannequin|Events") void OnContained();
	UFUNCTION(BlueprintImplementableEvent, Category="Mannequin|Events") void OnContainmentWarning();
	UFUNCTION(BlueprintImplementableEvent, Category="Mannequin|Events") void OnContainmentExpired();
	UFUNCTION(BlueprintImplementableEvent, Category="Mannequin|Events") void OnEnterSubmissiveToBabai();
	UFUNCTION(BlueprintImplementableEvent, Category="Mannequin|Events") void OnExitSubmissiveToBabai();
	/** Server-only override; parent performs the safe timeline switch when enabled. */
	UFUNCTION(BlueprintNativeEvent, Category="Mannequin|Capture")
	bool ApplyCapturePunishment(AHronoCharacter* CapturedPlayer);
	virtual bool ApplyCapturePunishment_Implementation(AHronoCharacter* CapturedPlayer);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mannequin|Capture")
	bool bTransferCapturedPlayerToOppositeTimeline = true;
	UFUNCTION(BlueprintImplementableEvent, Category="Mannequin|Mood")
	void OnMoodSnapshotApplied(EMannequinMood NewMood);
	UFUNCTION(BlueprintImplementableEvent, Category="Mannequin|Mood")
	void OnMannequinGameOver(AHronoCharacter* Victim);

private:
	UFUNCTION() void OnRep_State();
	UFUNCTION() void OnRep_Targets();
	UFUNCTION() void OnRep_Mood();
	UFUNCTION() void OnRep_CarriedItem();
	UFUNCTION() void HandleHuntStateChanged(EGhostHuntState OldState, EGhostHuntState NewState);
	UFUNCTION(NetMulticast, Reliable) void MulticastMoment(EMannequinState Moment, AHronoCharacter* CapturedPlayer);
	UFUNCTION(NetMulticast, Reliable) void MulticastSpecialEvent(uint8 EventCode);
	UFUNCTION(NetMulticast, Reliable) void MulticastGameOver(AHronoCharacter* Victim);
	void Evaluate();
	void RefreshLocalPresentation();
	void ApplyPhysicalState();
	void SetState(EMannequinState NewState, bool bEmitMoment = true);
	void StopMotion();
	bool SelectTarget(AHronoCharacter* Preferred = nullptr);
	bool IsEligibleTarget(const AHronoCharacter* Player) const;
	bool IsObserved(const AHronoCharacter* Player, bool bRequireMonocle) const;
	EMannequinSight EvaluateSight(const AHronoCharacter* Player, bool bRequireMonocle,
		FName& OutBlocker) const;
	void UpdateObservationSnapshot(EMannequinSight NewFuture, EMannequinSight NewPast,
		FName NewFutureBlocker, FName NewPastBlocker);
	void DrawLocalDebug(const AHronoCharacter* Viewer);
	ADrag_Item* FindClosedDoorBetweenTarget(FHitResult& OutHit) const;
	bool FindDoorApproachLocation(const FHitResult& DoorHit, FVector& OutLocation) const;
	bool FindValidLocation(const FVector& Center, float MinDistance, float MaxDistance,
		bool bRequireHidden, FVector& OutLocation, bool bLogFailure = false) const;
	bool FindBehindLocation(FVector& OutLocation) const;
	float GetCloseApproachCenterDistance() const;
	bool StartMove(const FVector& Destination, EMannequinState NewState);
	void EnterWarning();
	void BeginFinalGrabWindow();
	void Repel(bool bRescue);
	void CompleteGrab();
	void StartCooldown(float Duration);
	bool IsBabaiDangerous(EGhostHuntState HuntState) const;
	void EnterBabaiSubmissive();
	void ResumeAfterBabai();
	void EndContainment();
	void SetMood(EMannequinMood NewMood);
	void RefreshMask();
	void EvaluateMood(float Now);
	ABase_Item* FindMoodItem() const;
	bool IsOutsideTargetRoom() const;
	void FinishHappyKill();

	TWeakObjectPtr<AScareDirector> Director;
	FTimerHandle EvaluationTimer;
	FTimerHandle LocalPresentationTimer;
	FTimerHandle StateTimer;
	FTimerHandle ContainmentWarningTimer;
	FTimerHandle BabaiTimer;
	FVector MoveDestination = FVector::ZeroVector;
	bool bHasMoveDestination = false;
	bool bTargetObserved = false;
	bool bPartnerObservingThroughMonocle = false;
	bool bContainmentWasInterruptedByBabai = false;
	bool bMeshCollisionCaptured = false;
	bool bFactorySpawnPending = false;
	ECollisionEnabled::Type AuthoredMeshCollision = ECollisionEnabled::NoCollision;
	int32 CompletedStalkSteps = 0;
	float ActivatedAt = 0.0f;
	float LastObservedAt = -10000.0f;
	float LastApproachAt = -10000.0f;
	float NextTargetSwitchAt = 0.0f;
	float NextPartnerCheckAt = 0.0f;
	UPROPERTY(Replicated)
	float OfferStartedAt = -1.0f;
	float NextMoodPathAt = 0.0f;
	UPROPERTY()
	TObjectPtr<ABase_Item> DesiredItem;
	bool bMannequinGameOver = false;
	bool bLocalGameOverControlsApplied = false;
	UPROPERTY(Replicated)
	float WarningStartedAt = 0.0f;
	UPROPERTY(Replicated)
	float WarningEndsAt = 0.0f;
	float ContainmentEndsAt = 0.0f;
};
