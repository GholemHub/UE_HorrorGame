#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Enviroment_Interface.generated.h"

UINTERFACE(Blueprintable)
class HRONO_API UEnviroment_Interface : public UInterface
{
    GENERATED_BODY()
};

class HRONO_API IEnviroment_Interface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void Interact(AActor* Interactor);
};