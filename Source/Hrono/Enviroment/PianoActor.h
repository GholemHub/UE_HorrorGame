#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HronoSharedTools.h"
#include "Interface/Enviroment_Interface.h"
#include "PianoActor.generated.h"

class UBoxComponent;
class USoundAttenuation;
class USoundBase;
class UStaticMeshComponent;

/** A world piano played through the character's existing, server-validated E interaction. */
UCLASS(Blueprintable)
class HRONO_API APianoActor : public AActor, public IEnviroment_Interface
{
	GENERATED_BODY()

public:
	APianoActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	double GetLastPlayTimeSeconds() const { return LastPlayTimeSeconds; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Piano")
	TObjectPtr<UStaticMeshComponent> PianoMesh;

	/** The target of the Past/Future interaction trace (E). Also blocks players physically. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Piano")
	TObjectPtr<UBoxComponent> InteractionBox;

	/** Size the box from the assigned mesh. Disable to edit its extent and position manually. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Piano|Interaction")
	bool bAutoSizeInteractionBox = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Piano|Audio")
	TObjectPtr<USoundBase> PianoSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Piano|Audio")
	TObjectPtr<USoundAttenuation> SoundAttenuation;

	/** Who can hear and use this piano. Both is the default for a shared-world prop. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Piano|Interaction")
	EItemTimeline PianoTimeline = EItemTimeline::Both;

	/** Minimum time between accepted presses on the server. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Piano|Interaction", meta = (ClampMin = "0.0"))
	float InteractionCooldownSeconds = 0.25f;

private:
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayPiano(FVector_NetQuantize SoundLocation, EItemTimeline EventTimeline);

	double LastPlayTimeSeconds = -1.0e9;
};
