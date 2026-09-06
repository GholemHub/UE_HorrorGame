#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/HeldItemInertiaProfile.h"
#include "HeldItemInertiaComponent.generated.h"

class AHronoCharacter;
class UHeldItemInertiaProfile;

/**
 * Local cosmetic spring for an ABase_Item attached to a character interaction point.
 * It never replicates offsets; every visible copy derives them from existing movement.
 */
UCLASS(ClassGroup = (Items), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class HRONO_API UHeldItemInertiaComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHeldItemInertiaComponent();

	/** Optional reusable profile. When set, it overrides InlineSettings. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Held Item Inertia")
	TObjectPtr<UHeldItemInertiaProfile> Profile;

	/** Per-item tuning used when Profile is not assigned. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Held Item Inertia", meta = (ShowOnlyInnerProperties))
	FHeldItemInertiaSettings InlineSettings;

	/** Changes the reusable profile and safely clears accumulated motion. */
	UFUNCTION(BlueprintCallable, Category = "Held Item Inertia")
	void SetProfile(UHeldItemInertiaProfile* NewProfile);

	/** Clears offsets and velocities while preserving the current held base pose. */
	UFUNCTION(BlueprintCallable, Category = "Held Item Inertia")
	void ResetInertia();

	UFUNCTION(BlueprintPure, Category = "Held Item Inertia")
	FVector GetCurrentPositionOffset() const { return PositionOffset; }

	UFUNCTION(BlueprintPure, Category = "Held Item Inertia")
	FRotator GetCurrentRotationOffset() const;

	/**
	 * Adds an item-authored local pose on top of the normal held-item inertia.
	 * This is intended for short procedural actions such as an axe swing. The
	 * offset is cosmetic and should be driven on every machine that displays it.
	 */
	UFUNCTION(BlueprintCallable, Category = "Held Item Inertia|Action")
	void SetActionPoseOffset(FVector LocationOffset, FRotator RotationOffset);

	/** Restores the normal held pose without disturbing accumulated inertia. */
	UFUNCTION(BlueprintCallable, Category = "Held Item Inertia|Action")
	void ClearActionPoseOffset();

	/** Native lifecycle hooks used by ABase_Item. */
	void BeginHeld(AHronoCharacter* Character, const FTransform& InBaseRelativeTransform);
	void EndHeld(bool bRestoreBaseTransform);

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

private:
	const FHeldItemInertiaSettings& GetSettings() const;
	FRotator GetViewRotation(const AHronoCharacter* Character) const;
	void ResetSamples();
	void ApplyCurrentOffset();
	void IntegrateSpring(const FVector& TargetPosition, const FVector& TargetRotation,
		float DeltaTime, const FHeldItemInertiaSettings& Settings);
	static FVector ClampVectorAxes(const FVector& Value, const FVector& AbsoluteLimits);
	static bool IsFiniteVector(const FVector& Value);

	TWeakObjectPtr<AHronoCharacter> HeldCharacter;
	FTransform BaseRelativeTransform = FTransform::Identity;
	FVector PositionOffset = FVector::ZeroVector;
	FVector PositionVelocity = FVector::ZeroVector;
	/** Pitch, Yaw, Roll in X, Y, Z. */
	FVector RotationOffset = FVector::ZeroVector;
	FVector RotationVelocity = FVector::ZeroVector;
	FVector ActionPositionOffset = FVector::ZeroVector;
	FRotator ActionRotationOffset = FRotator::ZeroRotator;
	FVector PreviousCharacterVelocity = FVector::ZeroVector;
	FVector PreviousCharacterLocation = FVector::ZeroVector;
	FRotator PreviousViewRotation = FRotator::ZeroRotator;
	bool bHeld = false;
	bool bHaveSamples = false;
	bool bLoggedFirstAppliedFrame = false;
};
