// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Base_Item.h"
#include "Dozimetr.generated.h"

/**
 * 
 */
UCLASS()
class HRONO_API ADozimetr : public ABase_Item
{
	GENERATED_BODY()

public:

	// Called from server-authoritative C++ (OnPickedUp / Drop)
	void On();
	void Off();

	/** Played when the dosimeter is switched on. Assign any sound in Blueprint. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> TurnOnSound;

	/** Played when the dosimeter is switched off. Assign any sound in Blueprint. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> TurnOffSound;

protected:

	UFUNCTION(Client, Reliable)
	void Client_On();

	UFUNCTION(Client, Reliable)
	void Client_Off();

	// Implement these in the Blueprint event graph — this is where your Beep loop lives
	UFUNCTION(BlueprintImplementableEvent, Category = "Chair")
	void ReceiveOn();

	UFUNCTION(BlueprintImplementableEvent, Category = "Chair")
	void ReceiveOff();
};