#pragma once

#include "CoreMinimal.h"
#include "Items/Base_Item.h"
#include "RitualGoatSkull.generated.h"

class UGeometryCollectionComponent;
class UAudioComponent;
class USoundBase;

/** Server-authored audio lifecycle replicated by each skull to every client. */
UENUM(BlueprintType)
enum class ERitualSkullAudioState : uint8
{
	Silent,
	Playing,
	Finished
};

/** One shared skull class; ItemTimeline selects whether an instance belongs to Past or Future. */
UCLASS(BlueprintType, Blueprintable)
class HRONO_API ARitualGoatSkull : public ABase_Item
{
	GENERATED_BODY()

public:
	ARitualGoatSkull();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool TryPickUp(AHronoCharacter* Character) override;
	virtual void Drop() override;
	virtual void UpdateVisibilityForLocalPlayer(EItemTimeline ViewerTimeline) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cursed Room Ritual|Skull")
	TObjectPtr<UGeometryCollectionComponent> DestructibleSkull;

	/** Attached, locally controlled component used for the sound that lasts for the ritual. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cursed Room Ritual|Audio")
	TObjectPtr<UAudioComponent> RitualAudioComponent;

	/** One-shot sound played when this skull changes to its Chaos destruction mesh. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Audio")
	TObjectPtr<USoundBase> SkullBreakSound;

	/** Optional one-shot accent played when the skull begins rising after preparation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Audio")
	TObjectPtr<USoundBase> RitualStartSound;

	/** Sound kept playing from Rising until the ritual reaches Idle, Completed, or Failed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Audio")
	TObjectPtr<USoundBase> RitualLoopSound;

	/** Optional one-shot accent played when the ritual reaches Completed or Failed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Audio")
	TObjectPtr<USoundBase> RitualEndSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Audio",
		meta = (ClampMin = "0.0"))
	float RitualSoundVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Audio",
		meta = (ClampMin = "0.0", Units = "s"))
	float RitualSoundFadeInDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Audio",
		meta = (ClampMin = "0.0", Units = "s"))
	float RitualSoundFadeOutDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursed Room Ritual|Skull",
		meta = (ClampMin = "0.0", Units = "s"))
	float DebrisLifetime = 10.0f;

	void SetRitualLocked(bool bLocked);
	void SetRitualKinematic();
	void SetRitualPhysics(bool bReverseGravity);
	void RestoreNormalGravity();
	void ExplodeFromRitual();
	void StartRitualSound(bool bPlayStartSound);
	void StopRitualSound(bool bPlayEndSound);
	void SetRitualAudioState(ERitualSkullAudioState NewState);

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursed Room Ritual|Skull")
	void BP_OnWrongRoomReaction();

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursed Room Ritual|Skull")
	void BP_OnSkullStartFloating();

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursed Room Ritual|Skull")
	void BP_OnSkullStartScratching();

	UFUNCTION(BlueprintImplementableEvent, Category = "Cursed Room Ritual|Skull")
	void BP_OnRitualExplosion();

	UFUNCTION(BlueprintPure, Category = "Cursed Room Ritual|Skull")
	bool IsRitualLocked() const { return bRitualLocked; }

	UFUNCTION(BlueprintPure, Category = "Cursed Room Ritual|Skull")
	bool WasDestroyedByRitual() const { return bDestroyedByRitual; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_DestroyedByRitual();

	UFUNCTION()
	void HandleRitualAudioFinished();

	UFUNCTION()
	void OnRep_RitualAudioState();

private:
	void ActivateChaosDestruction();
	bool CanPlayLocalSkullAudio() const;
	void ApplyRitualAudioState();
	void StopLocalRitualLoopForTimelineVisibility();

	UPROPERTY(ReplicatedUsing = OnRep_DestroyedByRitual)
	bool bDestroyedByRitual = false;

	UPROPERTY(ReplicatedUsing = OnRep_RitualAudioState)
	ERitualSkullAudioState RitualAudioState = ERitualSkullAudioState::Silent;

	bool bRitualLocked = false;
	bool bChaosDestructionActivated = false;
	bool bRitualSoundRequested = false;
	bool bRitualSoundShouldBeActive = false;
	bool bPendingRitualStartSound = false;
};
