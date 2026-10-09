#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RitualCandleActor.generated.h"

class URitualCandleComponent;

/** Native ritual candle. Blueprint children supply meshes, flames and lights only. */
UCLASS(Blueprintable)
class HRONO_API ARitualCandleActor : public AActor
{
	GENERATED_BODY()

public:
	ARitualCandleActor();

	/** Server-owned sequence. Legacy LightAll Blueprint entry forwards here. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Ritual Candle")
	void StartRitualLighting();

	/** Server-owned mistake. Legacy OnMistake Blueprint entry forwards here. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Ritual Candle")
	void ReportRitualMistake();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ritual Candle")
	TObjectPtr<URitualCandleComponent> RitualCandle;
};
