// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Base_Item.h"
#include "Clock.generated.h"

/**
 * 
 */
UCLASS()
class HRONO_API AClock : public ABase_Item
{
	GENERATED_BODY()
	
public:
	virtual void Use_Implementation(AActor* Character) override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeNow = 0.0;

	/** Played when the clock is used/read. Assign any sound in Blueprint. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> UseSound;
};
