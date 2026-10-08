#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TableRitualManager.generated.h"

class AChair;
class AHronoCharacter;
class AOuijaBoard;
class ARitualBottle;
class ARitualCandleActor;
class USoundBase;

UENUM(BlueprintType)
enum class ETableRitualPhase : uint8
{
	Idle,
	Preparing,
	SlidingChair,
	LightingCandles,
	SpinningBottle,
	VictimChosen,
	Retrying,
	Exhausted
};

/** A single snapshot drives late-join chair and board presentation. */
USTRUCT(BlueprintType)
struct FTableRitualState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ETableRitualPhase Phase = ETableRitualPhase::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 AttemptsRemaining = 3;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AHronoCharacter> Victim = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 VictimIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bBoardVisible = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float BoardYaw = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector ChairStart = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector ChairTarget = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	double ChairSlideServerTime = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ChairSlideDuration = 0.0f;
};

/** Server-owned table ritual progression; Blueprint child supplies placed references only. */
UCLASS(Blueprintable)
class HRONO_API ATableRitualManager : public APawn
{
	GENERATED_BODY()

public:
	ATableRitualManager();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Table Ritual|References")
	TObjectPtr<AChair> TableChairA;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Table Ritual|References")
	TObjectPtr<AChair> TableChairB;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Table Ritual|References")
	TObjectPtr<AActor> SlidingChair;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Table Ritual|References")
	TObjectPtr<ARitualBottle> RitualBottle;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Table Ritual|References")
	TObjectPtr<ARitualCandleActor> RitualCandle;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Table Ritual|References")
	TObjectPtr<AOuijaBoard> OuijaBoard;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Table Ritual|References")
	TObjectPtr<AActor> VictimRitualPoint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Table Ritual|Timing", meta=(ClampMin="1"))
	int32 InitialAttempts = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Table Ritual|Timing", meta=(ClampMin="0.05", Units="s"))
	float ChairSlideDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Table Ritual|Timing", meta=(ClampMin="0.0", Units="cm"))
	float ChairSlideDistance = 20.0f;

	/** Failsafe when a death montage callback or the selected player disappears. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Table Ritual|Timing", meta=(ClampMin="5.0", Units="s"))
	float VictimReturnTimeout = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Table Ritual|Audio")
	TObjectPtr<USoundBase> ChairSlideSound;

	UPROPERTY(ReplicatedUsing=OnRep_RitualState, VisibleInstanceOnly, BlueprintReadOnly,
		Category="Table Ritual|Runtime")
	FTableRitualState RitualState;

	/** Called by chair seating or explicit server-side ritual actions. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Table Ritual")
	bool TryStartTableRitual();

	/** Compatibility entry points for existing Blueprint event references. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Table Ritual|Compatibility")
	void AcceptBottleVictimChoiceLegacy(bool bSecondVictim);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Table Ritual|Compatibility")
	void CompleteVictimReturnLegacy();

	/** Atomically returns the selected victim, unlocks doors and advances the attempt. */
	bool CompleteVictimReturn(AHronoCharacter* Character);
	bool IsSelectedVictim(const AHronoCharacter* Character) const;

protected:
	UFUNCTION()
	void OnRep_RitualState();

	UFUNCTION()
	void HandleCharacterSat(AHronoCharacter* Character);

	UFUNCTION()
	void HandleBottleVictimSelected(AHronoCharacter* Character, int32 VictimIndex);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastChairSlideCreak();

private:
	FTimerHandle PhaseTimer;
	FTimerHandle VictimTimeoutTimer;
	FRotator BoardInitialRotation = FRotator::ZeroRotator;
	bool bVictimReturnHandled = false;

	bool HaveBothSeatedPlayers() const;
	void SetPhase(ETableRitualPhase NewPhase);
	void ResetToIdle();
	void BeginChairSlide();
	void FinishChairSlide();
	void BeginCandleLighting();
	void BeginBottleSpin();
	void BeginRetry();
	void FinishRetryDelay();
	void HandleVictimReturnTimeout();
	void ApplyPresentation();
	double GetServerTime() const;
};
