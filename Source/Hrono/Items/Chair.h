// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Base_Item.h"
#include "Chair.generated.h"

class AHronoCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterSatOnChair, AHronoCharacter*, Character);

/**
 * 
 */
UCLASS()
class HRONO_API AChair : public ABase_Item
{
	GENERATED_BODY()
	

public:
	AChair();
	virtual void Use_Implementation(AActor* Character) override;

	/** Fired after a character successfully sits on this chair. */
	UPROPERTY(BlueprintAssignable, Category = "Chair")
	FOnCharacterSatOnChair OnCharacterSat;

	void NotifyCharacterSat(AHronoCharacter* Character);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	bool bIsSit = false;

	/** Character currently occupying this chair, or null when the chair is empty. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "Chair")
	TObjectPtr<AHronoCharacter> CurrentSitter;

	/** Returns the character sitting on this chair, or null if it is empty. */
	UFUNCTION(BlueprintPure, Category = "Chair")
	AHronoCharacter* GetSitter() const { return CurrentSitter; }

	void SetSitter(AHronoCharacter* Character) { CurrentSitter = Character; }

	/** Played when a character sits down on this chair. Assign any sound in Blueprint. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> SitSound;

	/** Played when a character stands up from this chair. Assign any sound in Blueprint. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> StandUpSound;

	/** Where the character is placed while sitting on this chair */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chair")
	USceneComponent* SitPoint;

	/** Where the character is placed after standing up from this chair */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chair")
	USceneComponent* StandUpPoint;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chair")
	bool IsRitualStarted = false;



	USceneComponent* GetSitPoint() const { return SitPoint; }
	USceneComponent* GetStandUpPoint() const { return StandUpPoint; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

};
