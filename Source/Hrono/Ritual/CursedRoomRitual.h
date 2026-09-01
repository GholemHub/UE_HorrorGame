#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HronoSharedTools.h"
#include "CursedRoomRitual.generated.h"

class AHronoCharacter;
class ABase_Item;
class ARitualCandle;
class ARitualGoatSkull;
class ARitualSymbolVisual;
class ARoom;
class UCurveFloat;
class UNiagaraSystem;
class USceneComponent;
class USoundBase;

UENUM(BlueprintType)
enum class ECursedRoomRitualState : uint8
{
	Idle,
	Reacting,
	Floating,
	Rotating,
	Scratching,
	Completed,
	Failed
};

/** Everything clients need to reconstruct the current physical sequence. No code or cursed-room reference is included. */
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
	FTransform SkullStartTransform = FTransform::Identity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cursed Room Ritual")
	FTransform SkullTargetTransform = FTransform::Identity;

	/** Server-selected ceiling axis used for deterministic client-side side-to-side motion. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cursed Room Ritual")
	FVector ScratchDirection = FVector::RightVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cursed Room Ritual")
	TObjectPtr<ARitualGoatSkull> GoatSkull = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cursed Room Ritual")
	TObjectPtr<ARitualCandle> RitualCandle = nullptr;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FCursedRoomRitualStateChangedSignature,
	ECursedRoomRitualState, PreviousState,
	ECursedRoomRitualState, NewState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FWrongRoomRitualSignature,
	ARoom*, TestedRoom,
	ARitualGoatSkull*, GoatSkull,
	ARitualCandle*, RitualCandle);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCursedRoomRitualCompletedSignature);

/**
 * Invisible, server-authoritative coordinator for the cooperative cursed-room test.
 * Place exactly one in the level. It has no collision, mesh, altar, or marker.
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

	/** Finds the placed coordinator without a hard-coded map reference. */
	UFUNCTION(BlueprintPure, Category = "Cursed Room Ritual", meta = (WorldContext = "WorldContextObject"))
	static ACursedRoomRitual* FindRitual(const UObject* WorldContextObject);

	/** Called by the dropped Future candle through the project's existing E interaction RPC. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Cursed Room Ritual")
	bool TryActivateRitual(AHronoCharacter* ActivatingCharacter, ARitualCandle* Candle);

	/** Authority-only: the complete code is deliberately never replicated to clients. */
	UFUNCTION(BlueprintPure, BlueprintAuthorityOnly, Category = "Cursed Room Ritual|Code")
	TArray<FString> GetRitualCode() const;

	/** Authority-only API intended for an independent safe/keypad actor. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Cursed Room Ritual|Code")
	bool ValidateRitualCode(const TArray<FString>& InputSymbols) const;

	/** The masked clue received by this local player, for example ["7", "?", "2", "?"]. */
	UFUNCTION(BlueprintPure, Category = "Cursed Room Ritual|Code")
	TArray<FString> GetLocalVisibleClue() const { return LocalVisibleClue; }

	UFUNCTION(BlueprintPure, Category = "Cursed Room Ritual")
	ECursedRoomRitualState GetRitualState() const { return ReplicatedState.State; }

	UFUNCTION(BlueprintPure, Category = "Cursed Room Ritual|Debug", meta = (DevelopmentOnly))
	FString GetRitualDebugStatus() const;

	/** Called only by AHronoCharacter's validated owner RPC. */
	void DeliverCompleteClueToCharacter(AHronoCharacter* Character, int32 RequestedSequenceId);

	/** Called by AHronoCharacter's targeted Client RPC. Never contains the other timeline's symbols. */
	void ReceiveSymbolForLocalPlayer(
		int32 SequenceId,
		int32 SlotIndex,
		const FString& Symbol,
		EItemTimeline ClueTimeline,
		const FTransform& SymbolTransform);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cursed Room Ritual|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	// ---- Sequence tuning ---------------------------------------------------------------------

	/** Minimum server-selected duration of the initial correct-room skull shake. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Sequence",
		meta = (ClampMin = "0.0", Units = "s", DisplayName = "Reaction Duration Min"))
	float RitualStartReactionDuration = 1.0f;

	/** Maximum server-selected duration of the initial correct-room skull shake. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Sequence",
		meta = (ClampMin = "0.0", Units = "s", DisplayName = "Reaction Duration Max"))
	float RitualStartReactionDurationMax = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Sequence",
		meta = (ClampMin = "0.0", Units = "cm"))
	float CorrectRoomShakeDistance = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Sequence",
		meta = (ClampMin = "1.0"))
	float CorrectRoomShakeCycles = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Sequence",
		meta = (ClampMin = "1.0", Units = "cm"))
	float FloatHeight = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Sequence",
		meta = (ClampMin = "0.1", Units = "s"))
	float FloatDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Sequence",
		meta = (ClampMin = "0.1", Units = "s"))
	float SkullRotationDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Sequence",
		meta = (ClampMin = "0.1", Units = "s"))
	float ScratchDuration = 5.0f;

	/** Distance travelled to each side of the ceiling-path center. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Sequence",
		meta = (ClampMin = "0.0", Units = "cm"))
	float ScratchSideToSideDistance = 200.0f;

	/** Number of complete left/right oscillations during ScratchDuration. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Sequence",
		meta = (ClampMin = "0.5"))
	float ScratchSideToSideCycles = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Sequence",
		meta = (ClampMin = "0.1", Units = "s"))
	float WrongRoomReactionDuration = 1.5f;

	/** Optional normalized 0..1 curve used for skull position and rotation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Sequence")
	TObjectPtr<UCurveFloat> SkullMoveCurve;

	/** Mesh-specific rotation added while the horns turn toward the ceiling. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Sequence")
	FRotator SkullCeilingRotationOffset = FRotator(-90.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Sequence",
		meta = (ClampMin = "0.0", Units = "cm"))
	float CeilingClearance = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Sequence",
		meta = (ClampMin = "1.0", Units = "cm"))
	float CeilingTraceDistance = 600.0f;

	/** Rejects nearby props as a ceiling and falls back to FloatHeight. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Sequence",
		meta = (ClampMin = "0.0", Units = "cm"))
	float MinimumCeilingRise = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Sequence",
		meta = (ClampMin = "0.01", Units = "s"))
	float VisualUpdateInterval = 0.033f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Failure",
		meta = (ClampMin = "0.0", Units = "cm"))
	float WrongRoomShakeDistance = 7.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Failure",
		meta = (ClampMin = "1.0"))
	float WrongRoomShakeCycles = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Interaction",
		meta = (ClampMin = "1.0", Units = "cm"))
	float ActivationDistance = 350.0f;

	// ---- Code and symbol tuning ---------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Code",
		meta = (ClampMin = "1", ClampMax = "16"))
	int32 NumberOfSymbols = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Code")
	TArray<int32> PastVisibleSlots = { 0, 2 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Code")
	TArray<int32> FutureVisibleSlots = { 1, 3 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Code")
	TArray<FString> AvailableSymbols = { TEXT("0"), TEXT("1"), TEXT("2"), TEXT("3"), TEXT("4"),
		TEXT("5"), TEXT("6"), TEXT("7"), TEXT("8"), TEXT("9") };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Symbols",
		meta = (ClampMin = "1.0", Units = "cm"))
	float ScratchSymbolSpacing = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Symbols")
	FVector ScratchDirectionLocal = FVector(0.0f, 1.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Symbols")
	FVector FutureSymbolOffset = FVector(70.0f, 0.0f, 35.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Symbols",
		meta = (ClampMin = "1.0", Units = "cm"))
	float FutureSymbolSpacing = 35.0f;

	/** Blueprint child of ARitualSymbolVisual; add a decal, mesh, material, or Niagara presentation there. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Symbols")
	TSubclassOf<ARitualSymbolVisual> SymbolDecalClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Presentation")
	TArray<TObjectPtr<USoundBase>> RitualSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Presentation")
	TArray<TObjectPtr<UNiagaraSystem>> RitualVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Debug")
	bool bDebugRitual = false;

	// ---- Replicated outcome ------------------------------------------------------------------

	UPROPERTY(ReplicatedUsing = OnRep_RitualState, VisibleInstanceOnly, BlueprintReadOnly,
		Category = "Cursed Room Ritual|Runtime")
	FCursedRoomRitualReplicatedState ReplicatedState;

	UPROPERTY(BlueprintAssignable, Category = "Cursed Room Ritual|Events")
	FCursedRoomRitualStateChangedSignature OnRitualStateChanged;

	/** Server-side hook for later demon aggression, sound, flicker, or horror events. */
	UPROPERTY(BlueprintAssignable, Category = "Cursed Room Ritual|Events")
	FWrongRoomRitualSignature OnWrongRoomRitual;

	UPROPERTY(BlueprintAssignable, Category = "Cursed Room Ritual|Events")
	FCursedRoomRitualCompletedSignature OnRitualCompleted;

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursed Room Ritual|Events")
	void BP_OnRitualStarted(ARitualGoatSkull* GoatSkull, ARitualCandle* RitualCandle);

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursed Room Ritual|Events")
	void BP_OnWrongRoom(ARoom* TestedRoom, ARitualGoatSkull* GoatSkull, ARitualCandle* RitualCandle);

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursed Room Ritual|Events")
	void BP_OnSkullStartFloating(ARitualGoatSkull* GoatSkull);

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursed Room Ritual|Events")
	void BP_OnSkullStartScratching(ARitualGoatSkull* GoatSkull);

	/** Invoked only on the owning client allowed to see this exact symbol. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Cursed Room Ritual|Events")
	void BP_OnSymbolRevealed(
		int32 SlotIndex,
		const FString& Symbol,
		EItemTimeline ClueTimeline,
		const FTransform& SymbolTransform);

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursed Room Ritual|Events")
	void BP_OnRitualCompleted();

protected:
	UFUNCTION()
	void OnRep_RitualState();

private:
	ARoom* FindContainingRoom(const AActor* Actor) const;
	ARitualGoatSkull* FindAvailableSkullInRoom(const ARoom* Room) const;
	bool IsDroppedRitualItemValid(const ABase_Item* Item, EItemTimeline RequiredTimeline) const;
	void StartCorrectRitual(ARoom* TestedRoom, ARitualGoatSkull* Skull, ARitualCandle* Candle);
	void StartWrongRoomRitual(ARoom* TestedRoom, ARitualGoatSkull* Skull, ARitualCandle* Candle);
	void GenerateCodeOnServer();
	void NormalizeVisibleSlots();
	void SetState(ECursedRoomRitualState NewState, float Duration, const FTransform& Start, const FTransform& Target);
	void HandleStateFinished();
	void ApplyReplicatedState();
	void DispatchStateEvents(ECursedRoomRitualState PreviousState);
	void LogStageEntered(ECursedRoomRitualState PreviousState) const;
	void LogStageFinished(ECursedRoomRitualState FinishedState) const;
	void StartVisualUpdates();
	void UpdateRitualVisuals();
	void StopVisualUpdates();
	void RevealSymbolsUpTo(int32 DesiredCount);
	void RevealSymbol(int32 SlotIndex);
	void SendSymbolToTimeline(int32 SlotIndex, EItemTimeline Timeline);
	FTransform MakeSymbolTransform(int32 SlotIndex, EItemTimeline Timeline) const;
	FTransform GetRitualItemWorldTransform(const ABase_Item* Item) const;
	void SetRitualItemWorldTransform(ABase_Item* Item, const FTransform& WorldTransform) const;
	void RequestCompleteClueForLocalPlayer();
	double GetSynchronizedServerTime() const;
	float EvaluateMoveAlpha(float LinearAlpha) const;
	FString BuildMaskedCode(EItemTimeline Timeline) const;
	void ClearLocalSymbolVisuals();

	UPROPERTY(Transient)
	TObjectPtr<ARoom> ActiveTestedRoom;

	/** Server-only complete answer. It is intentionally absent from GetLifetimeReplicatedProps. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Cursed Room Ritual|Code",
		meta = (AllowPrivateAccess = "true"))
	TArray<FString> FullCode;

	FTransform InitialSkullTransform = FTransform::Identity;
	FTransform ScratchStartTransform = FTransform::Identity;
	FTransform ScratchEndTransform = FTransform::Identity;
	FVector CeilingSurfaceStart = FVector::ZeroVector;
	FVector CeilingSurfaceNormal = FVector::DownVector;
	FVector ScratchWorldDirection = FVector::RightVector;
	int32 RevealedSymbolCount = 0;

	FTimerHandle StateTimerHandle;
	FTimerHandle VisualTimerHandle;
	FTimerHandle LocalClueRetryTimerHandle;

	ECursedRoomRitualState LastDispatchedState = ECursedRoomRitualState::Idle;
	int32 LastDispatchedSequenceId = INDEX_NONE;
	int32 LocalVisibleSequenceId = INDEX_NONE;
	TArray<FString> LocalVisibleClue;
	TSet<int32> LocalReceivedSlots;
	TArray<TWeakObjectPtr<ARitualSymbolVisual>> LocalSymbolVisuals;
};
