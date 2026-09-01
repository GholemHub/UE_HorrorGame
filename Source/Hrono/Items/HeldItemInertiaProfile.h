#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HeldItemInertiaProfile.generated.h"

/**
 * Cosmetic held-item inertia tuning. Values can live directly on an item's
 * component or in a reusable Data Asset shared by many item Blueprints.
 */
USTRUCT(BlueprintType)
struct HRONO_API FHeldItemInertiaSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inertia")
	bool bEnabled = true;

	/** Converts character acceleration (cm/s^2) into a camera-local position target. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (ClampMin = "0.0"))
	float CharacterAccelerationStrength = 0.0025f;

	/** Converts camera yaw speed (degrees/s) into sideways lag. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (ClampMin = "0.0"))
	float CameraHorizontalStrength = 0.015f;

	/** Converts camera pitch speed (degrees/s) into vertical lag. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (ClampMin = "0.0"))
	float CameraVerticalStrength = 0.012f;

	/** Spring restoring force. Larger values return to the held pose faster. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring", meta = (ClampMin = "0.0"))
	float SpringStiffness = 80.0f;

	/** Velocity damping. Lower values produce more overshoot. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring", meta = (ClampMin = "0.0"))
	float Damping = 14.0f;

	/** Absolute camera-local location limits in centimetres (X forward, Y right, Z up). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limits", meta = (ClampMin = "0.0"))
	FVector MaxPositionOffset = FVector(8.0f, 10.0f, 7.0f);

	/** Absolute rotation limits in degrees. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limits")
	FRotator MaxRotationOffset = FRotator(12.0f, 12.0f, 14.0f);

	/** Heavier values create more lag and lower the effective spring frequency. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ClampMin = "0.1"))
	float WeightMultiplier = 1.0f;

	/** Extra forward carry, in centimetres, when speed is lost abruptly. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (ClampMin = "0.0"))
	float SuddenStopSwingStrength = 3.0f;

	/** Mapped position/rotation targets below this value are ignored. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (ClampMin = "0.0"))
	float InputDeadZone = 0.08f;

	/** Converts positional lag into a small complementary tilt. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation", meta = (ClampMin = "0.0"))
	float PositionToRotation = 0.75f;

	/** Reduces cosmetic motion for non-local characters while retaining readable inertia. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multiplayer", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RemoteEffectScale = 0.65f;

	/** Input safety limit; also prevents a 180-degree reversal from destabilising the spring. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Safety", meta = (ClampMin = "1.0"))
	float MaxCharacterAcceleration = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Safety", meta = (ClampMin = "1.0"))
	float MaxCameraAngularSpeed = 720.0f;

	/** A larger one-frame owner displacement is treated as a teleport and resets the state. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Safety", meta = (ClampMin = "1.0", Units = "cm"))
	float TeleportDistance = 250.0f;

	/** A larger one-frame view discontinuity is treated as a view reset, not mouse input. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Safety", meta = (ClampMin = "1.0", ClampMax = "180.0"))
	float ViewDiscontinuityAngle = 120.0f;

	/** Frames longer than this reset samples instead of injecting a large impulse. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Safety", meta = (ClampMin = "0.016", ClampMax = "0.25", Units = "s"))
	float MaxDeltaTime = 0.1f;

	/** Maximum spring integration step. 1/120 keeps 30/60/120 FPS behaviour close. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Safety", meta = (ClampMin = "0.002", ClampMax = "0.033", Units = "s"))
	float MaxSubstep = 0.008333333f;
};

/** Create one asset per feel (Light, Heavy, Rigid, Pendant) and assign it on item Blueprints. */
UCLASS(BlueprintType)
class HRONO_API UHeldItemInertiaProfile : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Held Item Inertia", meta = (ShowOnlyInnerProperties))
	FHeldItemInertiaSettings Settings;
};

