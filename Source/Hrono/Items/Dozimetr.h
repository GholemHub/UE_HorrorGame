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