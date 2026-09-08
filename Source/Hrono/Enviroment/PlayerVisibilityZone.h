#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayerVisibilityZone.generated.h"

class AHronoCharacter;
class UBoxComponent;
class UPrimitiveComponent;
class USceneComponent;

/**
 * A replicated volume that reveals players inside it to other local players
 * without changing either player's timeline.
 */
UCLASS(BlueprintType, Blueprintable)
class HRONO_API APlayerVisibilityZone : public AActor
{
	GENERATED_BODY()

public:
	APlayerVisibilityZone();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Visibility Zone|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	/** Resize this box in a placed Blueprint or directly in the level. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Visibility Zone|Components")
	TObjectPtr<UBoxComponent> VisibilityVolume;

	/**
	 * False: a player inside is revealed to the other player wherever the viewer is.
	 * True: the viewer and the revealed player must both be inside this same zone.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Visibility Zone")
	bool bRequireViewerInside = false;

	/**
	 * Periodically repairs missed startup overlaps and reapplies visibility after
	 * late client possession/replication. BeginOverlap and EndOverlap remain immediate.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Visibility Zone",
		meta = (ClampMin = "0.1", Units = "s"))
	float OccupantCheckInterval = 1.0f;

	UPROPERTY(ReplicatedUsing = OnRep_PlayersInside, VisibleInstanceOnly, BlueprintReadOnly,
		Category = "Player Visibility Zone")
	TArray<TObjectPtr<AHronoCharacter>> PlayersInside;

	UFUNCTION(BlueprintPure, Category = "Player Visibility Zone")
	bool IsPlayerInside(const AHronoCharacter* Player) const;

	/** Local visibility query used by AHronoCharacter. It never changes timeline state. */
	static bool ShouldRevealToViewer(
		const UObject* WorldContextObject,
		const AHronoCharacter* Viewer,
		const AHronoCharacter* OtherPlayer);

protected:
	UFUNCTION()
	void HandleVolumeBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void HandleVolumeEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex);

	UFUNCTION()
	void OnRep_PlayersInside();

private:
	void AddPlayer(AHronoCharacter* Player);
	void RemovePlayer(AHronoCharacter* Player);
	void CheckZoneState();
	void ReconcileServerOccupants();
	void RefreshLocalPlayerVisibility() const;

	FTimerHandle OccupantCheckTimerHandle;
};
