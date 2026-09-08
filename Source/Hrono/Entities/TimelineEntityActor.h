#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HronoSharedTools.h"
#include "TimelineEntityActor.generated.h"

class AHronoCharacter;
class UAudioComponent;
class USceneComponent;
class USkeletalMeshComponent;
class UStaticMeshComponent;
#if WITH_EDITOR
struct FPropertyChangedEvent;
#endif

/** Selects which native visual component represents the entity. */
UENUM(BlueprintType)
enum class ETimelineEntityVisualType : uint8
{
	StaticMesh UMETA(DisplayName = "Static Mesh"),
	SkeletalMesh UMETA(DisplayName = "Skeletal Mesh")
};

/** Describes why the entity was hidden. */
UENUM(BlueprintType)
enum class ETimelineEntityDisappearReason : uint8
{
	Blueprint UMETA(DisplayName = "Blueprint"),
	PlayerTooClose UMETA(DisplayName = "Player Too Close"),
	WatchedTooLong UMETA(DisplayName = "Watched Too Long")
};

/**
 * A collision-free apparition that is visible only to a local player in the
 * same timeline. It supports either a static or skeletal mesh and reports
 * proximity/continuous-gaze milestones to Blueprint.
 *
 * Disappearance is local by design: two players can perceive the same entity
 * independently. Call Disappear Entity from a Blueprint event to hide it, or
 * leave the corresponding automatic option enabled.
 */
UCLASS(BlueprintType, Blueprintable)
class HRONO_API ATimelineEntityActor : public AActor
{
	GENERATED_BODY()

public:
	ATimelineEntityActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	/** Root used to position the visual and audio components. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Entity|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	/** Used when Visual Type is Static Mesh. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Entity|Components")
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	/** Used when Visual Type is Skeletal Mesh. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Entity|Components")
	TObjectPtr<USkeletalMeshComponent> SkeletalMesh;

	/** Spatial sound source. Assign its sound and attenuation in Blueprint. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Entity|Components")
	TObjectPtr<UAudioComponent> AudioComponent;

	/** Chooses which of the two mesh components is shown. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_VisualConfiguration,
		Category = "Entity|Visual")
	ETimelineEntityVisualType VisualType = ETimelineEntityVisualType::StaticMesh;

	/** The entity is visible only to players in this timeline. Both is universal. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_VisualConfiguration,
		Category = "Entity|Timeline")
	EItemTimeline EntityTimeline = EItemTimeline::Both;

	/** Distance at which On Player Too Close fires. Zero disables this test. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity|Detection|Proximity",
		meta = (ClampMin = "0.0", Units = "cm"))
	float TooCloseDistance = 200.0f;

	/** Automatically calls Disappear Entity after On Player Too Close. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity|Detection|Proximity")
	bool bAutomaticallyDisappearWhenTooClose = true;

	/** Half-angle of the cone that counts as looking at the entity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity|Detection|Gaze",
		meta = (ClampMin = "0.0", ClampMax = "90.0", Units = "deg"))
	float GazeHalfAngleDegrees = 10.0f;

	/** Local offset from the actor origin used as the gaze target. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity|Detection|Gaze")
	FVector GazeTargetOffset = FVector(0.0f, 0.0f, 90.0f);

	/** Continuous gaze time that fires On Watched For Disappear Duration. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity|Detection|Gaze",
		meta = (ClampMin = "0.0", Units = "s"))
	float GazeDisappearDuration = 3.0f;

	/** Automatically calls Disappear Entity after the first gaze milestone. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity|Detection|Gaze")
	bool bAutomaticallyDisappearAfterGaze = true;

	/** Continuous gaze time for the additional long-look Blueprint event. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity|Detection|Gaze",
		meta = (ClampMin = "0.0", Units = "s"))
	float LongGazeDuration = 5.0f;

	/** Walls and other Visibility-blocking geometry interrupt the gaze timer. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity|Detection|Gaze")
	bool bRequireUnobstructedView = true;

	/** Seconds after a real disappearance before this entity may return. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity|Respawn",
		meta = (ClampMin = "0.0", Units = "s"))
	float RespawnCooldown = 60.0f;

	/** The local player must be at least this far away when the cooldown finishes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity|Respawn",
		meta = (ClampMin = "0.0", Units = "cm"))
	float MinimumRespawnDistance = 600.0f;

	/** Turns the whole actor so its authored front faces the local player. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity|Facing")
	bool bFaceLocalPlayer = true;

	/** Keeps the apparition upright while it turns toward the player. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity|Facing")
	bool bUseYawOnly = true;

	/** Corrects meshes whose authored forward axis does not point along actor +X. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity|Facing")
	FRotator FacingRotationOffset = FRotator::ZeroRotator;

	/** Zero snaps immediately; positive values smoothly interpolate the facing rotation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity|Facing",
		meta = (ClampMin = "0.0"))
	float FacingRotationInterpSpeed = 0.0f;

	/** Hides the entity and stops its audio for this local player. */
	UFUNCTION(BlueprintCallable, Category = "Entity",
		meta = (DisplayName = "Disappear Entity"))
	void DisappearEntity(
		AHronoCharacter* TriggeringCharacter = nullptr,
		ETimelineEntityDisappearReason Reason = ETimelineEntityDisappearReason::Blueprint);

	/** Makes the entity perceptible again and rearms all detection events. */
	UFUNCTION(BlueprintCallable, Category = "Entity",
		meta = (DisplayName = "Reset Entity"))
	void ResetEntity();

	/** Re-evaluates mesh/audio visibility using the current local player's timeline. */
	UFUNCTION(BlueprintCallable, Category = "Entity|Timeline")
	void RefreshVisibilityForLocalPlayer();

	/** Immediately reapplies Visual Type to both native mesh components. */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Entity|Visual")
	void RefreshVisualRepresentation();

	UFUNCTION(BlueprintPure, Category = "Entity")
	bool HasLocallyDisappeared() const { return bHasLocallyDisappeared; }

	UFUNCTION(BlueprintPure, Category = "Entity|Detection|Gaze")
	float GetCurrentGazeDuration() const { return CurrentGazeDuration; }

	/** Used by ScareDirector. This never emits On Entity Disappeared. */
	void SetDirectorVisibility(bool bVisible);

	/** Stops director arbitration and restores this actor's standalone behavior. */
	void ClearDirectorVisibilityControl();

	bool IsAvailableForCharacter(AHronoCharacter& Character) const;

	/** Fired once when the local player enters Too Close Distance. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Entity|Events",
		meta = (DisplayName = "On Player Too Close"))
	void OnPlayerTooClose(AHronoCharacter* Character, float Distance);

	/** Fired once after the local player continuously watches for the short duration. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Entity|Events",
		meta = (DisplayName = "On Watched For Disappear Duration"))
	void OnWatchedForDisappearDuration(AHronoCharacter* Character);

	/**
	 * Fired at the five-second milestone. With automatic three-second disappearance
	 * enabled this cannot be reached; disable that option when this event is needed.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Entity|Events",
		meta = (DisplayName = "On Watched For Long Duration"))
	void OnWatchedForLongDuration(AHronoCharacter* Character);

	/** Fired after Disappear Entity has applied the hidden state. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Entity|Events",
		meta = (DisplayName = "On Entity Disappeared"))
	void OnEntityDisappeared(
		AHronoCharacter* TriggeringCharacter,
		ETimelineEntityDisappearReason Reason);

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnRep_VisualConfiguration();

	AHronoCharacter* GetLocalCharacter() const;
	bool IsCharacterInEntityTimeline(AHronoCharacter& Character) const;
	bool IsCharacterLookingAtEntity(AHronoCharacter& Character) const;
	void TryRespawnAfterCooldown(AHronoCharacter* Character, float DeltaSeconds);
	void FaceCharacter(AHronoCharacter& Character, float DeltaSeconds);
	void EnforceCollisionlessComponents();
	void ApplyVisualComponentState(bool bActorVisible);
	void ValidateSelectedVisualAsset() const;
	void ApplyLocalVisibility(bool bVisible);
	void ResetGazeState();

	bool bHasLocallyDisappeared = false;
	bool bIsManagedByDirector = false;
	bool bDirectorAllowsVisibility = true;
	bool bIsLocallyVisible = false;
	bool bWasTooClose = false;
	bool bShortGazeEventSent = false;
	bool bLongGazeEventSent = false;
	bool bAudioPausedForTimeline = false;
	float CurrentGazeDuration = 0.0f;
	float CurrentRespawnCooldown = 0.0f;
};
