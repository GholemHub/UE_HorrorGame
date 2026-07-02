// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SpotLightComponent.h"
#include "Light_Env.generated.h"

UCLASS()
class HRONO_API ALight_Env : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALight_Env();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Light")
	USpotLightComponent* Light;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void OnSwith(bool NewState);
};
