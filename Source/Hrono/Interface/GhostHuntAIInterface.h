#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Hunt/GhostHuntTypes.h"
#include "GhostHuntAIInterface.generated.h"

UINTERFACE(BlueprintType)
class HRONO_API UGhostHuntAIInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Implement this on the existing Demon pawn or AI Controller Blueprint.
 * Calls are made only by the authoritative Hunt Director.
 */
class HRONO_API IGhostHuntAIInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hunt|AI")
	void HandleHuntStateChanged(
		EGhostHuntState NewState,
		EItemTimeline TargetTimeline,
		FVector SearchOrigin,
		bool bHasLastKnownPlayerPosition,
		FVector LastKnownPlayerPosition);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hunt|AI")
	void HandleHuntStimulus(
		EGhostHuntStimulus Stimulus,
		AActor* SubjectActor,
		AActor* InterestActor,
		FVector StimulusLocation,
		EItemTimeline StimulusTimeline);
};

