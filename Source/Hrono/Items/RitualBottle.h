#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RitualBottle.generated.h"

class AHronoCharacter;
class ARitualBottle;
class UArrowComponent;
class USceneComponent;
class UStaticMeshComponent;

USTRUCT(BlueprintType)
struct FRitualBottleSpinState
{
	GENERATED_BODY()

	/** Increases for every authoritative spin and lets clients detect a new spin. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ritual Bottle")
	int32 SequenceId = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ritual Bottle")
	bool bIsSpinning = false;

	/** Server-synchronised time at which the spin began. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ritual Bottle")
	double StartServerTime = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ritual Bottle")
	float Duration = 0.0f;

	/** Starting relative yaw of BottlePivot. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ritual Bottle")
	float StartPivotYaw = 0.0f;

	/** Full turns plus the exact angle needed to face the selected direction. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ritual Bottle")
	float TotalSpinDegrees = 0.0f;

	/** Easing and wobble values captured by the server for this exact spin. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ritual Bottle")
	float EasingExponent = 3.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ritual Bottle")
	float SpinWobbleAmplitude = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ritual Bottle")
	float SpinWobbleCycles = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ritual Bottle")
	TObjectPtr<AHronoCharacter> SelectedVictim = nullptr;

	/** Zero for First Victim and one for Second Victim. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ritual Bottle")
	int32 SelectedVictimIndex = INDEX_NONE;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FRitualBottleSpinStartedDelegate,
	ARitualBottle*, Bottle);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FRitualBottleVictimSelectedDelegate,
	AHronoCharacter*, SelectedVictim,
	int32, VictimIndex);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FRitualBottleSpinFinishedDelegate,
	AHronoCharacter*, SelectedVictim,
	int32, VictimIndex);

/**
 * Server-authoritative, deterministic bottle used by the table ritual.
 *
 * The actor never uses physics. Only BottlePivot rotates, so the bottle cannot
 * drift or fall from the table. FirstVictimDirection and SecondVictimDirection
 * are fixed arrows that can be aimed at the two seats in a Blueprint child.
 * Replicated spin parameters make the server and every client render the same
 * spin and stop on the same victim.
 */
UCLASS(Blueprintable)
class HRONO_API ARitualBottle : public AActor
{
	GENERATED_BODY()

public:
	ARitualBottle();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Fixed actor root. This component is never moved by the spin. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ritual Bottle|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	/** The only component that rotates while the bottle is spinning. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ritual Bottle|Components")
	TObjectPtr<USceneComponent> BottlePivot;

	/** Assign the existing bottle mesh in a Blueprint child. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ritual Bottle|Components")
	TObjectPtr<UStaticMeshComponent> BottleMesh;

	/** Rotate this arrow so it points along the bottle neck. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ritual Bottle|Directions")
	TObjectPtr<UArrowComponent> SelectionDirection;

	/** Fixed direction for First Victim. Aim it at the first seat. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ritual Bottle|Directions")
	TObjectPtr<UArrowComponent> FirstVictimDirection;

	/** Fixed direction for Second Victim. Aim it at the second seat. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ritual Bottle|Directions")
	TObjectPtr<UArrowComponent> SecondVictimDirection;

	/** Minimum number of complete turns before the bottle reaches its target. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ritual Bottle|Spin", meta = (ClampMin = "1", UIMin = "1"))
	int32 MinFullRotations = 4;

	/** Maximum number of complete turns before the bottle reaches its target. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ritual Bottle|Spin", meta = (ClampMin = "1", UIMin = "1"))
	int32 MaxFullRotations = 7;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ritual Bottle|Spin", meta = (ClampMin = "0.1", UIMin = "0.1", Units = "s"))
	float SpinDuration = 4.5f;

	/** Higher values make the bottle slow down more strongly near the end. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ritual Bottle|Spin", meta = (ClampMin = "1.0", UIMin = "1.0"))
	float DecelerationExponent = 3.0f;

	/** Optional small random offset around the selected direction. Zero is exact. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ritual Bottle|Spin", meta = (ClampMin = "0.0", ClampMax = "15.0", Units = "deg"))
	float FinalAngleJitter = 0.0f;

	/** Small decaying visual wobble. It is always zero at the final frame. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ritual Bottle|Spin", meta = (ClampMin = "0.0", ClampMax = "10.0", Units = "deg"))
	float WobbleAmplitude = 1.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ritual Bottle|Spin", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float WobbleCycles = 3.0f;

	/**
	 * Randomly chooses one of the valid characters and starts an authoritative spin.
	 * Call this from the server-side ritual manager. Returns false if already spinning
	 * or if both victim references are invalid.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Ritual Bottle",
		meta = (DisplayName = "Spin Bottle"))
	bool SpinBottle(AHronoCharacter* FirstVictim, AHronoCharacter* SecondVictim);

	/** Stops any spin, restores the initial bottle direction and clears its result. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Ritual Bottle",
		meta = (DisplayName = "Reset Bottle"))
	void ResetBottle();

	/** Clears the completed victim without changing the bottle rotation. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Ritual Bottle",
		meta = (DisplayName = "Clear Selected Victim"))
	void ClearSelectedVictim();

	UFUNCTION(BlueprintPure, Category = "Ritual Bottle")
	AHronoCharacter* GetSelectedVictim() const { return SpinState.SelectedVictim; }

	UFUNCTION(BlueprintPure, Category = "Ritual Bottle")
	int32 GetSelectedVictimIndex() const { return SpinState.SelectedVictimIndex; }

	UFUNCTION(BlueprintPure, Category = "Ritual Bottle")
	bool IsBottleSpinning() const { return SpinState.bIsSpinning; }

	/** Bind from BP_TableRitualManager if another actor must react to the spin. */
	UPROPERTY(BlueprintAssignable, Category = "Ritual Bottle|Events")
	FRitualBottleSpinStartedDelegate OnSpinStarted;

	/** Fired automatically after the bottle has stopped, on server and clients. */
	UPROPERTY(BlueprintAssignable, Category = "Ritual Bottle|Events")
	FRitualBottleVictimSelectedDelegate OnVictimSelected;

	/** Fired immediately after OnVictimSelected. Useful for sounds and next ritual step. */
	UPROPERTY(BlueprintAssignable, Category = "Ritual Bottle|Events")
	FRitualBottleSpinFinishedDelegate OnSpinFinished;

	/** Automatic Blueprint event implemented directly inside a bottle Blueprint child. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ritual Bottle|Events",
		meta = (DisplayName = "On Bottle Spin Started"))
	void BP_OnBottleSpinStarted();

	/** Automatic Blueprint event implemented directly inside a bottle Blueprint child. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ritual Bottle|Events",
		meta = (DisplayName = "On Bottle Victim Selected"))
	void BP_OnBottleVictimSelected(AHronoCharacter* SelectedVictim, int32 VictimIndex);

	/** Automatic Blueprint event implemented directly inside a bottle Blueprint child. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ritual Bottle|Events",
		meta = (DisplayName = "On Bottle Spin Finished"))
	void BP_OnBottleSpinFinished(AHronoCharacter* SelectedVictim, int32 VictimIndex);

	UPROPERTY(ReplicatedUsing = OnRep_SpinState, VisibleInstanceOnly, BlueprintReadOnly,
		Category = "Ritual Bottle")
	FRitualBottleSpinState SpinState;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_SpinState();

private:
	double GetSynchronizedWorldTime() const;
	void ApplySpinAtTime(double CurrentServerTime);
	void ApplyFinalRotation();
	void HandleSpinStarted();
	void HandleSpinCompleted();

	FRotator InitialPivotRotation = FRotator::ZeroRotator;
	int32 LastStartedSequence = INDEX_NONE;
	int32 LastCompletedSequence = INDEX_NONE;
};
