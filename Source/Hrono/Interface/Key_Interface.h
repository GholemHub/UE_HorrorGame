// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Key_Interface.generated.h"

UINTERFACE(Blueprintable)
class HRONO_API UKey_Interface : public UInterface
{
	GENERATED_BODY()
};

class HRONO_API IKey_Interface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void Interact(AActor* Interactor);
};
