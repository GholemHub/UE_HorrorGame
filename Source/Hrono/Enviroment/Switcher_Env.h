// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/Enviroment_Interface.h"
#include "Enviroment/Light_Env.h"

#include "Switcher_Env.generated.h"

UCLASS()
class HRONO_API ASwitcher_Env : public AActor, public IEnviroment_Interface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASwitcher_Env();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	virtual void Interact_Implementation(AActor* Interactor) override;
	UFUNCTION(Server, Reliable)
	void ServerInteract(AActor* Interactor);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_Switch)
	bool bIsLightOn = true;
	
	UPROPERTY(EditInstanceOnly, Replicated, BlueprintReadWrite, Category = "Light Test")
	TArray<ALight_Env*> LightActors;

	UFUNCTION()
	void OnRep_Switch();
};
