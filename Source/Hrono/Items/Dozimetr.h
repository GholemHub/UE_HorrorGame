// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Base_Item.h"
#include "Dozimetr.generated.h"

class AHotDot;
class USoundBase;

/**
 * 
 */
UCLASS()
class HRONO_API ADozimetr : public ABase_Item
{
	GENERATED_BODY()

public:
	ADozimetr();
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Called from server-authoritative C++ (OnPickedUp / Drop)
	void On();
	void Off();

	/** Returns the nearest detectable HotDot for this dosimeter's timeline. */
	UFUNCTION(BlueprintPure, Category = "Dosimeter")
	AHotDot* FindClosestHotDot() const;

	UFUNCTION(BlueprintPure, Category = "Dosimeter")
	bool IsDosimeterActive() const { return bDosimeterActive; }

	UFUNCTION(BlueprintPure, Category = "Dosimeter")
	bool IsLocalDetectionRunning() const { return bLocalDetectionRunning; }

	/** Played when the dosimeter is switched on. Assign any sound in Blueprint. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> TurnOnSound;

	/** Played when the dosimeter is switched off. Assign any sound in Blueprint. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> TurnOffSound;

	/** Repeating beep played while an owning local player is holding the dosimeter. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dosimeter|Audio")
	TObjectPtr<USoundBase> BeepSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dosimeter|Detection",
		meta = (ClampMin = "0.0", Units = "cm"))
	float MinimumBeepDistance = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dosimeter|Detection",
		meta = (ClampMin = "0.0", Units = "cm"))
	float MaximumBeepDistance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dosimeter|Audio",
		meta = (ClampMin = "0.01", Units = "s"))
	float MinimumBeepInterval = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dosimeter|Audio",
		meta = (ClampMin = "0.01", Units = "s"))
	float MaximumBeepInterval = 2.0f;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnRep_OwningCharacter(AHronoCharacter* PreviousOwningCharacter) override;

	UFUNCTION()
	void OnRep_DosimeterActive();

	// Legacy Blueprint hooks kept so existing BP_Dozimetr assets continue to compile.
	// Native On/Off no longer invokes them; detection and beeping are fully handled above.
	UFUNCTION(BlueprintImplementableEvent, Category = "Chair")
	void ReceiveOn();

	UFUNCTION(BlueprintImplementableEvent, Category = "Chair")
	void ReceiveOff();

private:
	void RefreshLocalDetectionState(bool bPlayTransitionSound = true);
	void StartLocalDetection();
	void StopLocalDetection();
	void HandleBeep();
	float CalculateBeepInterval(float Distance) const;
	bool IsHeldByLocalPlayer() const;

	/** Authoritative on/off state. Local detection starts after both this and ownership replicate. */
	UPROPERTY(ReplicatedUsing = OnRep_DosimeterActive)
	bool bDosimeterActive = false;

	bool bLocalDetectionRunning = false;
	FTimerHandle BeepTimerHandle;
};
