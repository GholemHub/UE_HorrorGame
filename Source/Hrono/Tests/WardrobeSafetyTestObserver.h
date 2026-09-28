#pragma once

#include "CoreMinimal.h"
#include "HronoCharacter.h"
#include "Items/HidingWardrobe.h"
#include "WardrobeSafetyTestObserver.generated.h"

/** Transient delegate receiver for wardrobe regression tests; never spawned in gameplay. */
UCLASS(Transient, NotBlueprintable)
class UWardrobeSafetyTestObserver : public UObject
{
	GENERATED_BODY()
public:
	TArray<bool> Changes;
	int32 LostCount = 0;
	bool bBeginDeathOnLoss = false;
	bool bDeathAccepted = false;
	TWeakObjectPtr<AHidingWardrobe> DestroyOnGain;

	UFUNCTION()
	void OnSafetyChanged(bool bSafe)
	{
		Changes.Add(bSafe);
		if (bSafe && DestroyOnGain.IsValid())
		{
			AHidingWardrobe* Source = DestroyOnGain.Get();
			DestroyOnGain.Reset();
			Source->Destroy();
		}
	}

	UFUNCTION()
	void OnSafetyLost(AHronoCharacter* Player)
	{
		++LostCount;
		if (bBeginDeathOnLoss) bDeathAccepted = Player->BeginDeathTimelineTransition(nullptr);
	}
};
