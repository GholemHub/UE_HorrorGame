#pragma once

#include "CoreMinimal.h"
#include "Items/Base_Item.h"
#include "Clock.generated.h"

class ARoom;
class UClockSecondHandSoundComponent;
class USceneComponent;
class USoundBase;

/** Editable time-of-day used as the clock's initial and reset value. */
USTRUCT(BlueprintType)
struct HRONO_API FClockTimeOfDay
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock",
		meta = (ClampMin = "0", ClampMax = "23", UIMin = "0", UIMax = "23"))
	int32 Hour = 12;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock",
		meta = (ClampMin = "0", ClampMax = "59", UIMin = "0", UIMax = "59"))
	int32 Minute = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock",
		meta = (ClampMin = "0", ClampMax = "59", UIMin = "0", UIMax = "59"))
	int32 Second = 0;

	float ToSeconds() const;
	static FClockTimeOfDay FromSeconds(float Seconds);
	void Normalize();
};

/** The speed currently used by a clock. */
UENUM(BlueprintType)
enum class EClockSpeedMode : uint8
{
	Synchronized UMETA(DisplayName = "Synchronized"),
	Fast UMETA(DisplayName = "Fast"),
	Slow UMETA(DisplayName = "Slow")
};

/** Native clock anomalies distributed between rooms by the Scare Director. */
UENUM(BlueprintType)
enum class EClockAnomalyType : uint8
{
	Normal UMETA(DisplayName = "Normal"),
	Reverse UMETA(DisplayName = "Reverse"),
	Frozen UMETA(DisplayName = "Frozen"),
	JumpForward UMETA(DisplayName = "Jump Forward"),
	JumpBackward UMETA(DisplayName = "Jump Backward"),
	ErraticJumps UMETA(DisplayName = "Erratic Jumps"),
	Stutter UMETA(DisplayName = "Move And Stop"),
	Fast UMETA(DisplayName = "Fast"),
	Slow UMETA(DisplayName = "Slow")
};

/** How this placed clock behaves when its owning room is cursed. */
UENUM(BlueprintType)
enum class EClockCursedBehavior : uint8
{
	Fast UMETA(DisplayName = "Fast (Evidence)"),
	Slow UMETA(DisplayName = "Slow (Evidence)"),
	Synchronized UMETA(DisplayName = "Synchronized (No Evidence)"),
	RandomDeviation UMETA(DisplayName = "Random Fast Or Slow")
};

/**
 * Native three-hand clock.
 *
 * StartingTime sets the displayed time at BeginPlay. ARoom and ScareDirector
 * can assign deterministic replicated anomalies without Blueprint Tick logic.
 */
UCLASS(BlueprintType, Blueprintable)
class HRONO_API AClock : public ABase_Item
{
	GENERATED_BODY()

public:
	AClock();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void Use_Implementation(AActor* Character) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Sets the clock to InteractionResetTime and plays TimeResetSound. Server only. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Clock")
	void ResetClock(AActor* Character = nullptr);

	/** Changes the editable/reset time and optionally applies it immediately. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Clock")
	void SetStartingTime(FClockTimeOfDay NewStartingTime, bool bResetImmediately = true);

	/** Changes only the currently displayed time, preserving StartingTime. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Clock")
	void SetCurrentTime(FClockTimeOfDay NewCurrentTime);

	UFUNCTION(BlueprintPure, Category = "Clock")
	FClockTimeOfDay GetDisplayedTime() const;

	/** Current seconds since midnight, including a fractional second. */
	UFUNCTION(BlueprintPure, Category = "Clock")
	float GetCurrentClockTime() const;

	/** Assigns the room that owns this clock and immediately applies its state. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Clock|Room")
	void AssignToRoom(ARoom* NewOwningRoom);

	/** Called by ARoom whenever its authoritative cursed state changes. */
	void ApplyRoomCursedState(bool bRoomIsCursed);

	/** Changes the active mode while preserving the current hand positions. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Clock")
	void SetClockSpeedMode(EClockSpeedMode NewMode);

	UFUNCTION(BlueprintPure, Category = "Clock")
	EClockSpeedMode GetClockSpeedMode() const { return ClockSpeedMode; }

	/** Applies one deterministic anomaly while preserving the currently displayed time. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Clock|Anomaly")
	void ConfigureClockAnomaly(EClockAnomalyType NewAnomaly, int32 NewAnomalySeed);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Clock|Anomaly")
	void ClearClockAnomaly();

	UFUNCTION(BlueprintPure, Category = "Clock|Anomaly")
	EClockAnomalyType GetClockAnomalyType() const { return ClockAnomalyType; }

	UFUNCTION(BlueprintPure, Category = "Clock|Anomaly")
	bool HasClockAnomaly() const { return ClockAnomalyType != EClockAnomalyType::Normal; }

	UFUNCTION(BlueprintPure, Category = "Clock|Anomaly")
	FString GetClockAnomalyDescription() const;

	UFUNCTION(BlueprintPure, Category = "Clock|Room")
	ARoom* GetOwningRoom() const { return OwningRoom; }

	/** Time shown when play begins and after ResetClock. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_ClockState,
		Category = "Clock|Time")
	FClockTimeOfDay StartingTime;

	/** Time applied when a player interacts with the clock. Defaults to 12:00:00. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_ClockState,
		Category = "Clock|Time")
	FClockTimeOfDay InteractionResetTime;

	/** Runtime seconds since midnight. Kept writable for existing Blueprint compatibility. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Clock|Time")
	float TimeNow = 0.0f;

	/** Match the submitted Blueprint when false; interpolate all three hands when true. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock|Time")
	bool bSmoothHandMovement = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock|Behavior")
	EClockCursedBehavior CursedRoomBehavior = EClockCursedBehavior::Fast;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock|Behavior",
		meta = (ClampMin = "1.0"))
	float FastRateMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock|Behavior",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SlowRateMultiplier = 0.5f;

	/** Randomized rate range used by Reverse patterns. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock|Anomaly|Tuning")
	FVector2D ReverseRateRange = FVector2D(0.75f, 1.75f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock|Anomaly|Tuning")
	FVector2D AnomalousFastRateRange = FVector2D(2.0f, 5.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock|Anomaly|Tuning")
	FVector2D AnomalousSlowRateRange = FVector2D(0.1f, 0.65f);

	/** X is the shortest and Y the longest pause between jumps, in real seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock|Anomaly|Tuning")
	FVector2D JumpIntervalRange = FVector2D(1.5f, 5.0f);

	/** X/Y are the minimum/maximum clock-seconds moved by one jump. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock|Anomaly|Tuning")
	FVector2D JumpAmountRange = FVector2D(15.0f, 300.0f);

	/** Length of one move-then-stop cycle, in real seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock|Anomaly|Tuning")
	FVector2D StutterPeriodRange = FVector2D(2.5f, 6.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock|Visual")
	FName HourHandComponentName = TEXT("Hour");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock|Visual")
	FName MinuteHandComponentName = TEXT("Minute");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock|Visual")
	FName SecondHandComponentName = TEXT("Second");

	/** Added before the Blueprint formulas; normally left at zero. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock|Visual")
	FRotator HandRotationOffset = FRotator::ZeroRotator;

	/** Played for every client when the player resets the clock. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock|Audio")
	TObjectPtr<USoundBase> TimeResetSound;

	/** Dedicated one-shot controller attached to the Blueprint's Second component. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Clock|Components")
	TObjectPtr<UClockSecondHandSoundComponent> SecondHandSoundComponent;

	/** Legacy reset sound property retained for existing Blueprint instances. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clock|Audio|Legacy")
	TObjectPtr<USoundBase> UseSound;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_ClockState,
		Category = "Clock|State")
	EClockSpeedMode ClockSpeedMode = EClockSpeedMode::Synchronized;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_ClockState,
		Category = "Clock|Anomaly")
	EClockAnomalyType ClockAnomalyType = EClockAnomalyType::Normal;

	/** Replicated seed makes jumps and rates identical on every client. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_ClockState,
		Category = "Clock|Anomaly")
	int32 ClockAnomalySeed = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_OwningRoom,
		Category = "Clock|State")
	TObjectPtr<ARoom> OwningRoom;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_ClockState();

	UFUNCTION()
	void OnRep_OwningRoom();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayResetSound();

private:
	static constexpr float SecondsPerDay = 24.0f * 60.0f * 60.0f;

	double GetSynchronizedWorldTime() const;
	float GetRateMultiplier(EClockSpeedMode Mode) const;
	float GetAnomalousElapsedTime(double ElapsedRealSeconds) const;
	float GetAnomalyAudioRate() const;
	void ResolveAnomalyParameters(float& OutPrimary, float& OutSecondary) const;
	uint32 HashAnomalyStep(int32 Step) const;
	EClockSpeedMode ResolveCursedSpeedMode() const;
	USceneComponent* FindHandComponent(FName ComponentName) const;
	void ResolveClockHands();
	void UpdateClockVisual();
	void ApplyHandRotations(float SecondsSinceMidnight);
	void UpdateSecondHandSoundAttachment();
	bool IsAudibleForLocalPlayer() const;
	void SetCurrentTimeSeconds(float NewTimeSeconds);

	UPROPERTY(ReplicatedUsing = OnRep_ClockState)
	float ClockTimeAtAnchor = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_ClockState)
	double ServerTimeAtAnchor = 0.0;

	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> HourHandComponent;

	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> MinuteHandComponent;

	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> SecondHandComponent;

	bool bClockAnchorInitialized = false;
	int32 LastAudibleSecond = INDEX_NONE;
};
