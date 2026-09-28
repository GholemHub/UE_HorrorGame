// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Base_Item.h"
#include "Radio.generated.h"

class UAudioComponent;
class USoundBase;

/**
 * 
 */
UCLASS()
class HRONO_API ARadio : public ABase_Item
{
	GENERATED_BODY()
	public:
	UFUNCTION(BlueprintCallable, Category="Radio|Audio")
	UAudioComponent* ReplaceRadioSound(UAudioComponent* Previous, USoundBase* Sound);
};
