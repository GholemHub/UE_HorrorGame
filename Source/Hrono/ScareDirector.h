#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Hunt/GhostHuntTypes.h"
#include "ScareDirector.generated.h"

class AScareDirector;
class ABase_Item;
class USceneComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGhostThreatStateChangedSignature, EGhostThreatState, OldState, EGhostThreatState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGhostHuntStateChangedSignature, EGhostHuntState, OldState, EGhostHuntState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGhostHuntPhaseSignature, EItemTimeline, TargetTimeline);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGhostHuntOmenSignature, EGhostHuntOmen, Omen, EItemTimeline, TargetTimeline);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FGhostHuntStimulusSignature, EGhostHuntStimulus, Stimulus, AActor*, SubjectActor, AActor*, InterestActor, FVector, StimulusLocation, EItemTimeline, StimulusTimeline);

/**
 * Server-authoritative, timer-driven Hunt Director.
 *
 * The exact Threat value and AI knowledge are intentionally never replicated. Clients receive
 * only the coarse threat band, hunt state/target, and individual omen events needed to present
 * environmental feedback. The actor orchestrates hunts; Blueprints own audiovisual effects and
 * the Demon AI owns movement, perception, and attacks.
 */
UCLASS(BlueprintType, Blueprintable)
class HRONO_API AScareDirector : public AActor
{
	GENERATED_BODY()

public:
	AScareDirector();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Finds the placed replicated Director. Prefer this over hard-coded actor references. */
	UFUNCTION(BlueprintPure, Category = "Hunt", meta = (WorldContext = "WorldContextObject"))
	static AScareDirector* GetHuntDirector(const UObject* WorldContextObject);

	// ---- Server-only Threat API ---------------------------------------------------------------

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Hunt|Threat")
	void AddThreat(float Amount);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Hunt|Threat")
	void RemoveThreat(float Amount);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Hunt|Threat")
	void SetThreat(float Value);

	/** Preferred debug-friendly variants: the reason is shown when the aggression band changes. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Hunt|Threat")
	void AddThreatWithReason(float Amount, const FString& Reason);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Hunt|Threat")
	void RemoveThreatWithReason(float Amount, const FString& Reason);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Hunt|Threat")
	void SetThreatWithReason(float Value, const FString& Reason);

	/** Returns zero on clients because the hidden value is not replicated. */
	UFUNCTION(BlueprintPure, Category = "Hunt|Threat")
	float GetThreat() const;

	UFUNCTION(BlueprintPure, Category = "Hunt|Threat")
	bool CanStartHunt() const;

	UFUNCTION(BlueprintPure, Category = "Hunt|Threat")
	EGhostThreatState GetThreatState() const { return CurrentThreatState; }

	UFUNCTION(BlueprintPure, Category = "Hunt|Debug", meta = (DevelopmentOnly))
	FString GetLastThreatStateChangeReason() const { return LastThreatStateChangeReason; }

	UFUNCTION(BlueprintPure, Category = "Hunt|Debug", meta = (DevelopmentOnly))
	FString GetLastHuntStateChangeReason() const { return LastHuntStateChangeReason; }

	// ---- Hunt API ----------------------------------------------------------------------------

	/**
	 * Requests a story/gameplay-driven hunt. It bypasses Threat, but respects cooldown unless
	 * bIgnoreCooldown is explicitly true. Triggered hunts cannot become false alarms unless
	 * bAllowFalseAlarm is explicitly true.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Hunt")
	bool RequestTriggeredHunt(
		EItemTimeline TargetTimeline = EItemTimeline::Both,
		bool bUseWarningPhase = true,
		bool bIgnoreCooldown = false,
		bool bAllowFalseAlarm = false);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Hunt")
	void EndHunt();

	UFUNCTION(BlueprintPure, Category = "Hunt")
	EGhostHuntState GetHuntState() const { return CurrentHuntState; }

	UFUNCTION(BlueprintPure, Category = "Hunt")
	EGhostHuntType GetHuntType() const { return CurrentHuntType; }

	UFUNCTION(BlueprintPure, Category = "Hunt")
	EItemTimeline GetHuntTimelineTarget() const { return CurrentHuntTimelineTarget; }

	UFUNCTION(BlueprintPure, Category = "Hunt")
	bool IsHuntActive() const;

	UFUNCTION(BlueprintPure, Category = "Hunt|Timeline")
	bool DoesHuntAffectTimeline(EItemTimeline Timeline) const;

	UFUNCTION(BlueprintPure, Category = "Hunt|AI")
	FVector GetHuntSearchOrigin() const { return ActiveSearchOrigin; }

	UFUNCTION(BlueprintPure, Category = "Hunt|AI")
	bool HasLastKnownPlayerPosition() const { return bHasLastKnownPlayerPosition; }

	UFUNCTION(BlueprintPure, Category = "Hunt|AI")
	FVector GetLastKnownPlayerPosition() const { return LastKnownPlayerPosition; }

	/** Set the Demon pawn or its AI Controller. It should implement GhostHuntAIInterface. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Hunt|AI")
	void SetHuntDemon(AActor* NewHuntDemon);

	/**
	 * Selects a random configured Babaj spawn point and invokes the Spawn Babaj Blueprint event.
	 * Returns false when called on a client or when no valid spawn point is available.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Hunt|Babaj",
		meta = (DisplayName = "Spawn Babaj At Random Point"))
	bool SpawnBabajAtRandomPoint();

	// ---- Authoritative AI perception / hiding bridge ----------------------------------------

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Hunt|AI")
	void ReportPlayerDetected(AActor* Player, FVector DetectedPosition, EItemTimeline PlayerTimeline);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Hunt|AI")
	void ReportPlayerNoise(AActor* NoiseSource, FVector NoiseLocation, EItemTimeline NoiseTimeline);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Hunt|AI")
	void ReportPlayerLost(AActor* Player, FVector LastSeenPosition, EItemTimeline PlayerTimeline);

	/**
	 * Report a wardrobe/closet entry. If observed, the hiding place becomes an investigation
	 * stimulus. If unobserved, only the last visible position is shared with the Demon.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Hunt|AI")
	void ReportPlayerEnteredHiding(
		AActor* Player,
		AActor* HidingPlace,
		bool bDemonObservedEntry,
		FVector LastVisiblePosition,
		EItemTimeline PlayerTimeline);

	// ---- Events ------------------------------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Hunt|Events")
	FGhostThreatStateChangedSignature OnThreatStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Hunt|Events")
	FGhostHuntStateChangedSignature OnHuntStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Hunt|Events")
	FGhostHuntPhaseSignature OnHuntWarningStarted;

	UPROPERTY(BlueprintAssignable, Category = "Hunt|Events")
	FGhostHuntPhaseSignature OnHuntStarted;

	UPROPERTY(BlueprintAssignable, Category = "Hunt|Events")
	FGhostHuntPhaseSignature OnHuntEnded;

	UPROPERTY(BlueprintAssignable, Category = "Hunt|Events")
	FGhostHuntPhaseSignature OnFalseAlarm;

	UPROPERTY(BlueprintAssignable, Category = "Hunt|Events")
	FGhostHuntOmenSignature OnHuntOmenTriggered;

	/** Server-only gameplay signal for the authoritative AI bridge. */
	UPROPERTY(BlueprintAssignable, Category = "Hunt|Events")
	FGhostHuntStimulusSignature OnHuntStimulus;

	// Blueprint override events are convenient inside the existing BP_ScareDirector child.
	UFUNCTION(BlueprintImplementableEvent, Category = "Hunt|Events", meta = (DisplayName = "Receive Threat State Changed"))
	void ReceiveThreatStateChanged(EGhostThreatState OldState, EGhostThreatState NewState);

	UFUNCTION(BlueprintImplementableEvent, Category = "Hunt|Events", meta = (DisplayName = "Receive Hunt State Changed"))
	void ReceiveHuntStateChanged(EGhostHuntState OldState, EGhostHuntState NewState);

	UFUNCTION(BlueprintImplementableEvent, Category = "Hunt|Events", meta = (DisplayName = "Receive Hunt Warning Started"))
	void ReceiveHuntWarningStarted(EItemTimeline TargetTimeline);

	UFUNCTION(BlueprintImplementableEvent, Category = "Hunt|Events", meta = (DisplayName = "Receive Hunt Started"))
	void ReceiveHuntStarted(EItemTimeline TargetTimeline);

	UFUNCTION(BlueprintImplementableEvent, Category = "Hunt|Events", meta = (DisplayName = "Receive Hunt Ended"))
	void ReceiveHuntEnded(EItemTimeline TargetTimeline);

	UFUNCTION(BlueprintImplementableEvent, Category = "Hunt|Events", meta = (DisplayName = "Receive False Alarm"))
	void ReceiveFalseAlarm(EItemTimeline TargetTimeline);

	UFUNCTION(BlueprintImplementableEvent, Category = "Hunt|Events", meta = (DisplayName = "Receive Hunt Omen Triggered"))
	void ReceiveHuntOmenTriggered(EGhostHuntOmen Omen, EItemTimeline TargetTimeline);

	/**
	 * Server-only request fired when aggression enters HuntEligible. Implement this event in
	 * BP_ScareDirector and use Spawn Actor from Class to create Babaj yourself.
	 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintAuthorityOnly, Category = "Hunt|Babaj",
		meta = (DisplayName = "Spawn Babaj"))
	void ReceiveSpawnBabaj(
		ABase_Item* SpawnPoint,
		FTransform SpawnTransform,
		TSubclassOf<AActor> SuggestedBabajClass,
		float SuggestedLifetime);

	// ---- Editable tuning ---------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt|Threat", meta = (ClampMin = "1.0"))
	float MaxThreat = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt|Threat", meta = (ClampMin = "60.0"))
	float HuntThreshold = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt|Threat", meta = (ClampMin = "0.0"))
	float PassiveThreatPerInterval = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt|Threat", meta = (ClampMin = "0.1", Units = "s"))
	float PassiveThreatInterval = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt|Eligibility", meta = (ClampMin = "0.1", Units = "s"))
	float HuntCheckIntervalMin = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt|Eligibility", meta = (ClampMin = "0.1", Units = "s"))
	float HuntCheckIntervalMax = 18.0f;

	/** Probability per eligibility check, not per frame. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt|Eligibility", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HuntChance = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt|Cooldown", meta = (ClampMin = "0.0", Units = "s"))
	float MinimumHuntCooldown = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt|Cooldown", meta = (ClampMin = "0.0"))
	float ThreatAfterHunt = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt|Warning")
	TArray<EGhostHuntOmen> AvailableHuntOmens;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt|Warning", meta = (ClampMin = "1", ClampMax = "10"))
	int32 MinimumOmens = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt|Warning", meta = (ClampMin = "1", ClampMax = "10"))
	int32 MaximumOmens = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt|Warning", meta = (ClampMin = "0.0", Units = "s"))
	float OmenIntervalMin = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt|Warning", meta = (ClampMin = "0.0", Units = "s"))
	float OmenIntervalMax = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt|Warning", meta = (ClampMin = "0.0", Units = "s"))
	float PostOmenDelayMin = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt|Warning", meta = (ClampMin = "0.0", Units = "s"))
	float PostOmenDelayMax = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt|False Alarm", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FalseAlarmChance = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt|False Alarm", meta = (ClampMin = "0.0"))
	float FalseAlarmThreatReduction = 15.0f;

	/** Guarantees this many real warnings between false alarms, in addition to probability. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt|False Alarm", meta = (ClampMin = "0"))
	int32 MinimumRealWarningsBetweenFalseAlarms = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt|Duration", meta = (ClampMin = "0.0", Units = "s"))
	float ManifestationDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt|Duration", meta = (ClampMin = "1.0", Units = "s"))
	float HuntDurationMin = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt|Duration", meta = (ClampMin = "1.0", Units = "s"))
	float HuntDurationMax = 75.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt|Duration", meta = (ClampMin = "0.0", Units = "s"))
	float EndingStateDuration = 2.0f;

	/** Reuses Mirrorbound's existing Past/Future/Both timeline enum. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt|Timeline")
	EItemTimeline OrganicHuntTimelineTarget = EItemTimeline::Both;

	/**
	 * When enabled, entering Manifesting closes every rotating door and entering
	 * HuntEligible opens them. Shelves and sliding cupboard panels are ignored.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt|Doors")
	bool bAnimateDoorsOnThreatStateChanges = true;

	/** Every aggression-band transition turns off all switches and environment lights. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt|Lights")
	bool bTurnOffAllLightsOnThreatStateChange = true;

	/** Optional class passed to the Spawn Babaj Blueprint event as a convenient default. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt|Babaj")
	TSubclassOf<AActor> BabajClass;

	/**
	 * Optional exact set of placed BP_ItemPointSpawn actors. When empty, the Director
	 * automatically finds every actor of BabajSpawnPointClass in the current level.
	 */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Hunt|Babaj")
	TArray<TObjectPtr<ABase_Item>> BabajSpawnPoints;

	/** Spawn-point Blueprint class used by automatic discovery when BabajSpawnPoints is empty. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt|Babaj")
	TSubclassOf<ABase_Item> BabajSpawnPointClass;

	/** Suggested lifetime passed to the Spawn Babaj Blueprint event. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt|Babaj",
		meta = (ClampMin = "0.1", ClampMax = "40.0", Units = "s"))
	float BabajLifetime = 40.0f;

	/** Optional paranormal room/origin. Falls back to the Demon, then this Director. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Hunt|AI")
	TObjectPtr<AActor> HuntOriginActor;

	/** Optional direct interface target. May be the Demon pawn or its AI Controller. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Hunt|AI")
	TObjectPtr<AActor> HuntDemon;

	// ---- Development-only controls (all are no-ops in Shipping) -----------------------------

	/** Shows a timer-refreshed Print String panel. Numeric Threat is visible only on authority. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt|Debug")
	bool bShowHuntDebugOnScreen = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt|Debug", meta = (ClampMin = "0.1", Units = "s"))
	float DebugScreenRefreshInterval = 1.0f;

	/** Delay between aggression-band steps in Hunt.TestScenario. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt|Debug", meta = (ClampMin = "0.5", Units = "s"))
	float DebugTestStepInterval = 2.5f;

	UFUNCTION(BlueprintCallable, Category = "Hunt|Debug", meta = (DevelopmentOnly))
	void DebugAddThreat(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Hunt|Debug", meta = (DevelopmentOnly))
	void DebugForceHuntEligible();

	UFUNCTION(BlueprintCallable, Category = "Hunt|Debug", meta = (DevelopmentOnly))
	void DebugForceHunt(EItemTimeline TargetTimeline = EItemTimeline::Both, bool bSkipWarning = false);

	UFUNCTION(BlueprintCallable, Category = "Hunt|Debug", meta = (DevelopmentOnly))
	void DebugEndHunt();

	UFUNCTION(BlueprintCallable, Category = "Hunt|Debug", meta = (DevelopmentOnly))
	void DebugTriggerOmen(EGhostHuntOmen Omen);

	/** Runs an accelerated Dormant -> Disturbed -> Manifesting -> Eligible -> Hunt cycle. */
	UFUNCTION(BlueprintCallable, Category = "Hunt|Debug", meta = (DevelopmentOnly))
	void DebugRunTestScenario(EItemTimeline TargetTimeline = EItemTimeline::Both);

	UFUNCTION(BlueprintCallable, Category = "Hunt|Debug", meta = (DevelopmentOnly))
	void SetHuntDebugOnScreenEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Hunt|Debug", meta = (DevelopmentOnly))
	FString GetDebugStatus() const;

	UFUNCTION(BlueprintPure, Category = "Hunt|Debug", meta = (DevelopmentOnly))
	TArray<EGhostHuntOmen> GetSelectedHuntOmens() const { return SelectedHuntOmens; }

	UFUNCTION(BlueprintPure, Category = "Hunt|Debug", meta = (DevelopmentOnly))
	float GetCooldownRemaining() const;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentThreatState, VisibleInstanceOnly, BlueprintReadOnly, Category = "Hunt|State")
	EGhostThreatState CurrentThreatState = EGhostThreatState::Dormant;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHuntState, VisibleInstanceOnly, BlueprintReadOnly, Category = "Hunt|State")
	EGhostHuntState CurrentHuntState = EGhostHuntState::None;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Hunt|State")
	EItemTimeline CurrentHuntTimelineTarget = EItemTimeline::Both;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Hunt|State")
	EGhostHuntType CurrentHuntType = EGhostHuntType::Organic;

	UFUNCTION()
	void OnRep_CurrentThreatState(EGhostThreatState PreviousState);

	UFUNCTION()
	void OnRep_CurrentHuntState(EGhostHuntState PreviousState);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastTriggerHuntOmen(EGhostHuntOmen Omen, EItemTimeline TargetTimeline);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFalseAlarmResolved(EItemTimeline TargetTimeline);

private:
	void HandlePassiveThreatTimer();
	void SetThreatInternal(float Value, const FString& Reason);
	void UpdateThreatState(const FString& Reason);
	EGhostThreatState CalculateThreatState() const;
	void SetHuntState(EGhostHuntState NewState, const FString& Reason = TEXT("Director state transition"));
	void DispatchThreatStateChanged(EGhostThreatState OldState, EGhostThreatState NewState);
	void DispatchHuntStateChanged(EGhostHuntState OldState, EGhostHuntState NewState);
	void AnimateAllDoorsForThreatState(bool bOpen, const FString& Reason);
	void TurnOffAllLightsForThreatState(EGhostThreatState NewState);
	USceneComponent* ResolveBabajSpawnComponent(const ABase_Item* SpawnPoint) const;
	void StartDebugScreenTimer();
	void HandleDebugScreenTimer();
	void PrintHuntDebugMessage(const FString& Message, const FLinearColor& Color, float Duration = 5.0f, bool bAlsoLog = true) const;
	FString BuildOnScreenDebugStatus() const;
	FLinearColor GetThreatDebugColor() const;
	void HandleDebugTestScenarioStep();
	void HandleDebugTestPlayerDetected();
	void HandleDebugTestPlayerLost();
	void BackupAndApplyDebugTestTuning();
	void RestoreDebugTestTuning();

	void ScheduleNextHuntCheck();
	void EvaluateOrganicHunt();
	void CancelHuntCheck();
	void BeginWarningPhase(EItemTimeline TargetTimeline, bool bAllowFalseAlarm);
	void SelectHuntOmens();
	void TriggerNextSelectedOmen();
	void ResolveWarningPhase();
	void ResolveFalseAlarm();
	void StartActualHunt();
	void EnterSearchingState();
	void BeginCooldown();
	void FinishCooldown();
	void ClearHuntTimers();

	void UpdateLastKnownPlayerPosition(const FVector& Position);
	void DispatchStimulus(EGhostHuntStimulus Stimulus, AActor* SubjectActor, AActor* InterestActor, const FVector& Location, EItemTimeline StimulusTimeline);
	void NotifyDemonOfState(EGhostHuntState NewState);
	AActor* ResolveHuntDemonInterfaceTarget() const;
	FVector ResolveSearchOrigin() const;
	FString GetSelectedOmensDebugString() const;

	/** Hidden authoritative value. Intentionally not a UPROPERTY and never replicated. */
	float Threat = 0.0f;
	bool bHuntOnCooldown = false;
	bool bPendingFalseAlarm = false;
	bool bHasLastKnownPlayerPosition = false;
	bool bDebugTestScenarioRunning = false;
	bool bDebugTuningBackedUp = false;
	int32 NextOmenIndex = 0;
	int32 RealWarningsSinceLastFalseAlarm = 2;
	int32 DebugTestScenarioStep = 0;

	FVector LastKnownPlayerPosition = FVector::ZeroVector;
	FVector ActiveSearchOrigin = FVector::ZeroVector;
	EItemTimeline DebugTestTimeline = EItemTimeline::Both;
	FString LastThreatStateChangeReason = TEXT("Director initialized");
	FString LastHuntStateChangeReason = TEXT("No hunt transition yet");
	TWeakObjectPtr<AActor> TargetedPlayer;
	TWeakObjectPtr<AActor> DebugTestPlayer;
	TArray<EGhostHuntOmen> SelectedHuntOmens;

	struct FDebugTuningBackup
	{
		float PassiveThreatPerInterval = 0.0f;
		float OmenIntervalMin = 0.0f;
		float OmenIntervalMax = 0.0f;
		float PostOmenDelayMin = 0.0f;
		float PostOmenDelayMax = 0.0f;
		float ManifestationDuration = 0.0f;
		float HuntDurationMin = 0.0f;
		float HuntDurationMax = 0.0f;
		float EndingStateDuration = 0.0f;
		float MinimumHuntCooldown = 0.0f;
	};

	FDebugTuningBackup DebugTuningBackup;

	FTimerHandle PassiveThreatTimerHandle;
	FTimerHandle DebugScreenTimerHandle;
	FTimerHandle DebugTestScenarioTimerHandle;
	FTimerHandle DebugTestPerceptionTimerHandle;
	FTimerHandle HuntCheckTimerHandle;
	FTimerHandle OmenTimerHandle;
	FTimerHandle PostOmenTimerHandle;
	FTimerHandle ManifestationTimerHandle;
	FTimerHandle HuntDurationTimerHandle;
	FTimerHandle EndingTimerHandle;
	FTimerHandle CooldownTimerHandle;
};
