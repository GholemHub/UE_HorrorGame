#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RitualCandleComponent.generated.h"

/** Server-owned, late-join-safe state for the three flames on BP_Item_Candle. */
UCLASS(ClassGroup=(Ritual), meta=(BlueprintSpawnableComponent))
class HRONO_API URitualCandleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URitualCandleComponent();

	/** Compatibility wrapper for older Blueprint callers. */
	UFUNCTION(BlueprintCallable, Category="Ritual Candle", meta=(DefaultToSelf="CandleActor"))
	static void StartLightingForActor(AActor* CandleActor);

	/** Compatibility wrapper for older Blueprint callers. */
	UFUNCTION(BlueprintCallable, Category="Ritual Candle", meta=(DefaultToSelf="CandleActor"))
	static void ReportMistakeForActor(AActor* CandleActor);

	/** Called by the native candle actor on the server. */
	void StartLighting();
	void ReportMistake();

	UFUNCTION(BlueprintPure, Category="Ritual Candle")
	uint8 GetLitMask() const { return static_cast<uint8>(VisualState & 0xff); }

	UFUNCTION(BlueprintPure, Category="Ritual Candle")
	uint8 GetHiddenBodyMask() const { return static_cast<uint8>(VisualState >> 8); }

	/** Flame component names in ignition order; each flame may own a light child. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ritual Candle|Setup")
	TArray<FName> FlameComponentNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ritual Candle|Setup",
		meta=(ClampMin="0.05", Units="s"))
	float IgnitionInterval = 1.0f;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(ReplicatedUsing=OnRep_VisualState, VisibleInstanceOnly, BlueprintReadOnly,
		Category="Ritual Candle|Runtime")
	int32 VisualState = 0;

	UFUNCTION()
	void OnRep_VisualState();

private:
	FTimerHandle IgnitionTimerHandle;
	uint8 NextIgnitionIndex = 0;

	void IgniteNextFlame();
	void SetVisualState(uint8 NewLitMask, uint8 NewHiddenBodyMask);
	void ApplyVisualState();
	USceneComponent* FindFlame(FName ComponentName) const;
};
