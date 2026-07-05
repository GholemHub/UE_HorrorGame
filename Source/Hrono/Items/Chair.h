// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Base_Item.h"
#include "Chair.generated.h"

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

	UPROPERTY(Replicated)
	bool bIsSit = false;

	/** Where the character is placed while sitting on this chair */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chair")
	USceneComponent* SitPoint;

	/** Where the character is placed after standing up from this chair */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chair")
	USceneComponent* StandUpPoint;

	USceneComponent* GetSitPoint() const { return SitPoint; }
	USceneComponent* GetStandUpPoint() const { return StandUpPoint; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

};
