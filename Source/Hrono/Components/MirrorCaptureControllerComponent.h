#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MirrorCaptureControllerComponent.generated.h"

class USceneCaptureComponent2D;
class USceneComponent;

/** Drives a SceneCapture2D as a planar mirror of the local player's camera. */
UCLASS(ClassGroup = (Rendering), meta = (BlueprintSpawnableComponent))
class HRONO_API UMirrorCaptureControllerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMirrorCaptureControllerComponent();

	/** Capture that writes to the mirror render target. Auto-finds one on the owner when unset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mirror")
	TObjectPtr<USceneCaptureComponent2D> SceneCapture;

	/** Component whose location and +X axis define the mirror plane. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mirror")
	TObjectPtr<USceneComponent> MirrorPlane;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mirror")
	int32 PlayerIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mirror")
	bool bMatchPlayerFOV = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mirror", meta = (EditCondition = "bMatchPlayerFOV"))
	float FOVOffset = 0.0f;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	FVector PlaneOrigin = FVector::ZeroVector;
	FVector PlaneNormal = FVector::ForwardVector;
	bool bHasMirrorPlane = false;
};
