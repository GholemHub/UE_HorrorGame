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

	/**
	 * Changes this switch on the authoritative server and lets RepNotify apply the
	 * result to every connected machine. Returns true only when the state changed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Switch")
	bool SetLightState(bool bNewIsLightOn);

	/**
	 * Applies a visual state directly to every light assigned in LightActors.
	 * Besides Light_Env, this supports Blueprint actors containing Point/Spot/Rect
	 * light components. It is intentionally local so RepNotify can call it on clients.
	 * Returns the number of light components/actors that received the state.
	 */
	UFUNCTION(BlueprintCallable, Category = "Switch")
	int32 ApplyLightStateToLinkedActors(bool bNewIsLightOn);

	/** Automatically fires in this actor's Blueprint whenever the switch state changes. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Switch", meta = (DisplayName = "On Switch Toggled"))
	void OnSwitchToggled(bool bNewIsLightOn);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_Switch)
	bool bIsLightOn = true;

	UPROPERTY(EditInstanceOnly, Replicated, BlueprintReadWrite, Category = "Light Test")
	TArray<AActor*> LightActors;

	/** Played when the switch is flipped on. Assign any sound in Blueprint. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> SwitchOnSound;

	/** Played when the switch is flipped off. Assign any sound in Blueprint. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> SwitchOffSound;

	UFUNCTION()
	void OnRep_Switch();
};
