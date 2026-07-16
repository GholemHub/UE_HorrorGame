#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemRedactor.generated.h"

/**
 * A Blueprint-placeable actor that stores references to other actors.
 */
UCLASS(Blueprintable)
class HRONO_API AItemRedactor : public AActor
{
	GENERATED_BODY()

public:
	AItemRedactor();

	/** Actors assigned to this redactor. Editable per placed instance and available in Blueprints. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Redactor")
	TArray<TObjectPtr<AActor>> Actors;
};
