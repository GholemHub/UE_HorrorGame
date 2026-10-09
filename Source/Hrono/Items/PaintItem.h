#pragma once

#include "CoreMinimal.h"
#include "Items/Base_Item.h"
#include "PaintItem.generated.h"

class AHronoCharacter;
class UNiagaraComponent;
class UStaticMeshComponent;

/** Exactly one capture-only visual clue can be active on a painting. */
UENUM(BlueprintType)
enum class EPaintAnomalyType : uint8
{
	None UMETA(DisplayName = "None"),
	Eyes UMETA(DisplayName = "Watching Eyes"),
	Tentacles UMETA(DisplayName = "Tentacles"),
	TextureCube UMETA(DisplayName = "Texture Cube")
};

/**
 * Native replacement for BP_Paint_Item's gameplay graph.
 *
 * The frame remains an ordinary ABase_Item mesh. A painting can expose exactly
 * one anomaly at a time: eyes, a Niagara tentacle effect, or a capture-only
 * canvas for a cubemap material. ARoom assigns the replicated anomaly when it
 * applies its painting-evidence pattern.
 */
UCLASS(BlueprintType, Blueprintable)
class HRONO_API APaintItem : public ABase_Item
{
	GENERATED_BODY()

public:
	APaintItem();
	virtual bool CanBePickedUp() const override { return false; }

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void UpdateVisibilityForLocalPlayer(EItemTimeline ViewerTimeline) override;

	/** Authority-only evidence assignment. None hides and deactivates every anomaly. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Painting|Evidence")
	void SetPaintAnomalyType(EPaintAnomalyType NewType);

	UFUNCTION(BlueprintPure, Category = "Painting|Evidence")
	EPaintAnomalyType GetPaintAnomalyType() const { return PaintAnomalyType; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Painting|Components")
	TObjectPtr<UStaticMeshComponent> LeftEyeMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Painting|Components")
	TObjectPtr<UStaticMeshComponent> RightEyeMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Painting|Components")
	TObjectPtr<UNiagaraComponent> TentacleEffect;

	/** Position/scale this plane over the canvas in each painting Blueprint. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Painting|Components")
	TObjectPtr<UStaticMeshComponent> CubeAnomalyMesh;

	/** Zero follows the viewer immediately, matching the old Blueprint. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Painting|Eyes",
		meta = (ClampMin = "0.0", UIMin = "0.0"))
	float EyeRotationInterpSpeed = 0.0f;

	/** Replicated clue state. Its enum makes simultaneous eyes and tentacles impossible. */
	UPROPERTY(ReplicatedUsing = OnRep_PaintAnomalyType, VisibleInstanceOnly,
		BlueprintReadOnly, Category = "Painting|Evidence")
	EPaintAnomalyType PaintAnomalyType = EPaintAnomalyType::None;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_PaintAnomalyType();

private:
	void ApplyAnomalyVisibility();
	void HideCubeFromOtherSceneCaptures();
	AHronoCharacter* FindLocalViewer() const;
	void RotateEyeToward(UStaticMeshComponent* Eye, const FQuat& AuthoredRotationOffset,
		const FVector& TargetLocation, float DeltaSeconds) const;

	/** Last local timeline supplied by ABase_Item. This never replicates. */
	EItemTimeline LocalViewerTimeline = EItemTimeline::Both;

	/**
	 * Blueprint-authored relative rotations captured before tracking starts. They
	 * correct each imported eye mesh's own forward axis independently.
	 */
	FQuat LeftEyeAuthoredRotationOffset = FQuat::Identity;
	FQuat RightEyeAuthoredRotationOffset = FQuat::Identity;

	/**
	 * Prevents visibility refreshes from resetting Niagara and issuing its spawn
	 * burst again. Cleared only when the effect is deliberately deactivated.
	 */
	bool bTentacleActivationIssued = false;

};
