#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HronoAudioLibrary.generated.h"
class UAudioComponent;
class USoundBase;
class USceneComponent;
class AActor;

UCLASS()
class HRONO_API UHronoAudioLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    /** Replace a local effect source, stopping the previous source before allocating another. */
    UFUNCTION(BlueprintCallable, Category="Hrono|Audio")
    static UAudioComponent* ReplaceAttachedSound(UAudioComponent* Previous,
        USoundBase* Sound, USceneComponent* AttachToComponent);
    UFUNCTION(BlueprintCallable, Category="Hrono|Audio")
    static UAudioComponent* ReplaceSoundOnActor(UAudioComponent* Previous,
        USoundBase* Sound, AActor* Actor);
    UFUNCTION(BlueprintCallable, Category="Hrono|Audio")
    static void StopManagedSound(UAudioComponent* Component);
};
