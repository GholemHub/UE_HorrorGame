// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Base_Item.h"
#include "Chair.generated.h"

/**
 * 
 */
UCLASS()
class HRONO_API AChair : public ABase_Item
{
	GENERATED_BODY()
	

public:
	AChair();
	virtual void Use_Implementation(AActor* Character) override;

	UPROPERTY(Replicated)
	bool bIsSit = false;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

};
