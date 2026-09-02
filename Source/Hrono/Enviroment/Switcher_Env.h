// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/Enviroment_Interface.h"
#include "Enviroment/Light_Env.h"

#include "Switcher_Env.generated.h"

class UMaterialInstanceDynamic;

/** Runtime-only cache used to restore the real material values after a flicker pulse. */
struct FSwitcherEmissiveMaterialState
{
	TWeakObjectPtr<UMaterialInstanceDynamic> Material;
	TMap<FName, float> OriginalScalarValues;
	TMap<FName, FLinearColor> OriginalVectorValues;
};

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

	/**
	 * Applies only the emissive-material part of the visual state. Ritual flicker
	 * uses this without changing the replicated switch state or replaying audio.
	 */
	UFUNCTION(BlueprintCallable, Category = "Switch|Emissive")
	int32 ApplyEmissiveStateToLinkedActors(bool bNewIsLightOn);

	/** Automatically fires in this actor's Blueprint whenever the switch state changes. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Switch", meta = (DisplayName = "On Switch Toggled"))
	void OnSwitchToggled(bool bNewIsLightOn);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_Switch)
	bool bIsLightOn = true;

	UPROPERTY(EditInstanceOnly, Replicated, BlueprintReadWrite, Category = "Light Test")
	TArray<AActor*> LightActors;

	/**
	 * Additional fixture/bulb actors whose mesh materials should follow this switch.
	 * Meshes inside LightActors are detected automatically, so only add actors here
	 * when the visible lamp mesh is separate from its Point/Spot/Rect light actor.
	 */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Switch|Emissive")
	TArray<TObjectPtr<AActor>> EmissiveActors;

	/** Scalar material parameters treated as emissive strength and set to zero while off. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switch|Emissive")
	TArray<FName> EmissiveScalarParameterNames = {
		TEXT("EmissiveIntensity"),
		TEXT("Emissive Intensity"),
		TEXT("EmissiveStrength"),
		TEXT("Emissive Strength"),
		TEXT("EmissiveMultiplier"),
		TEXT("GlowIntensity")
	};

	/** Vector material parameters treated as emissive colour and set to black while off. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switch|Emissive")
	TArray<FName> EmissiveVectorParameterNames = {
		TEXT("EmissiveColor"),
		TEXT("Emissive Color"),
		TEXT("EmissiveTint"),
		TEXT("GlowColor")
	};

	/** Played when the switch is flipped on. Assign any sound in Blueprint. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> SwitchOnSound;

	/** Played when the switch is flipped off. Assign any sound in Blueprint. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> SwitchOffSound;

	/**
	 * Server Threat added when a player changes this switch from OFF to ON.
	 * Set to zero for switches which should not affect aggression. Direct system
	 * calls and ritual flicker do not use this value.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Switch|Threat",
		meta = (ClampMin = "0.0"))
	float ThreatIncreaseWhenTurnedOn = 5.0f;

	UFUNCTION()
	void OnRep_Switch();

private:
	void InitializeEmissiveMaterials();

	bool bEmissiveMaterialsInitialized = false;
	TArray<FSwitcherEmissiveMaterialState> EmissiveMaterialStates;
};
