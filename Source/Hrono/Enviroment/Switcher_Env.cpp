// Fill out your copyright notice in the Description page of Project Settings.


#include "Enviroment/Switcher_Env.h"
#include "Components/LightComponentBase.h"
#include "Components/MeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Enviroment/Light_Env.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "ScareDirector.h"
#include "Sound/SoundBase.h"


// Sets default values
ASwitcher_Env::ASwitcher_Env()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

}

// Called when the game starts or when spawned
void ASwitcher_Env::BeginPlay()
{
	Super::BeginPlay();

	// Placed switches start enabled by default. Applying the initial state here is
	// important because an initial replicated value equal to the C++ default does
	// not necessarily invoke RepNotify on clients.
	const int32 InitialLightCount = ApplyLightStateToLinkedActors(bIsLightOn);
	UE_LOG(LogTemp, Log,
		TEXT("[%s] Initial switch state is %s; applied to %d linked light components/actors"),
		*GetName(), bIsLightOn ? TEXT("ON") : TEXT("OFF"), InitialLightCount);
}

// Called every frame
void ASwitcher_Env::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASwitcher_Env::Interact_Implementation(AActor* Interactor)
{
	UE_LOG(LogTemp, Log, TEXT("Interact_Implementation"));
	
		UE_LOG(LogTemp, Log, TEXT("Interact_Implementation2"));
		ServerInteract(Interactor);
}


void ASwitcher_Env::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASwitcher_Env, bIsLightOn);
	DOREPLIFETIME(ASwitcher_Env, LightActors);
}


void ASwitcher_Env::OnRep_Switch()
{
	// Runs on clients (RepNotify) and is called manually on the server, so this is
	// the single place every machine reacts to the light state change.
	if (USoundBase* SwitchSound = bIsLightOn ? SwitchOnSound : SwitchOffSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SwitchSound, GetActorLocation());
	}

	const int32 AppliedLightCount = ApplyLightStateToLinkedActors(bIsLightOn);
	UE_LOG(LogTemp, Log,
		TEXT("[%s] Switch state changed to %s; applied to %d linked light components/actors"),
		*GetName(), bIsLightOn ? TEXT("ON") : TEXT("OFF"), AppliedLightCount);

	OnSwitchToggled(bIsLightOn);
}

int32 ASwitcher_Env::ApplyLightStateToLinkedActors(bool bNewIsLightOn)
{
	int32 AppliedLightCount = 0;

	for (AActor* LightActor : LightActors)
	{
		if (!IsValid(LightActor))
		{
			continue;
		}

		if (ALight_Env* EnvironmentLight = Cast<ALight_Env>(LightActor))
		{
			EnvironmentLight->SetLightEnabled(bNewIsLightOn);
			++AppliedLightCount;
			continue;
		}

		// The current house uses BP_LightActor, whose native parent is AActor and
		// whose actual light is a PointLightComponent. Handle that Blueprint and
		// any similar lamp without requiring it to inherit from ALight_Env.
		TInlineComponentArray<ULightComponentBase*> LightComponents;
		LightActor->GetComponents(LightComponents, true);
		for (ULightComponentBase* LightComponent : LightComponents)
		{
			if (IsValid(LightComponent))
			{
				LightComponent->SetVisibility(bNewIsLightOn, true);
				++AppliedLightCount;
			}
		}
	}

	return AppliedLightCount + ApplyEmissiveStateToLinkedActors(bNewIsLightOn);
}

void ASwitcher_Env::InitializeEmissiveMaterials()
{
	if (bEmissiveMaterialsInitialized)
	{
		return;
	}

	bEmissiveMaterialsInitialized = true;
	TSet<AActor*> CandidateActors;
	for (AActor* LightActor : LightActors)
	{
		if (IsValid(LightActor))
		{
			CandidateActors.Add(LightActor);
		}
	}
	for (AActor* EmissiveActor : EmissiveActors)
	{
		if (IsValid(EmissiveActor))
		{
			CandidateActors.Add(EmissiveActor);
		}
	}

	int32 ControlledParameterCount = 0;
	for (AActor* CandidateActor : CandidateActors)
	{
		TInlineComponentArray<UMeshComponent*> MeshComponents;
		CandidateActor->GetComponents(MeshComponents, true);
		for (UMeshComponent* MeshComponent : MeshComponents)
		{
			if (!IsValid(MeshComponent))
			{
				continue;
			}

			const int32 MaterialCount = MeshComponent->GetNumMaterials();
			for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
			{
				UMaterialInterface* SourceMaterial = MeshComponent->GetMaterial(MaterialIndex);
				if (!IsValid(SourceMaterial))
				{
					continue;
				}

				TArray<FMaterialParameterInfo> ScalarInfos;
				TArray<FMaterialParameterInfo> VectorInfos;
				TArray<FGuid> ParameterIds;
				SourceMaterial->GetAllScalarParameterInfo(ScalarInfos, ParameterIds);
				ParameterIds.Reset();
				SourceMaterial->GetAllVectorParameterInfo(VectorInfos, ParameterIds);

				TArray<FName> MatchingScalarNames;
				TArray<FName> MatchingVectorNames;
				for (const FMaterialParameterInfo& ParameterInfo : ScalarInfos)
				{
					if (EmissiveScalarParameterNames.Contains(ParameterInfo.Name))
					{
						MatchingScalarNames.AddUnique(ParameterInfo.Name);
					}
				}
				for (const FMaterialParameterInfo& ParameterInfo : VectorInfos)
				{
					if (EmissiveVectorParameterNames.Contains(ParameterInfo.Name))
					{
						MatchingVectorNames.AddUnique(ParameterInfo.Name);
					}
				}

				if (MatchingScalarNames.IsEmpty() && MatchingVectorNames.IsEmpty())
				{
					continue;
				}

				UMaterialInstanceDynamic* DynamicMaterial =
					MeshComponent->CreateDynamicMaterialInstance(MaterialIndex, SourceMaterial);
				if (!IsValid(DynamicMaterial))
				{
					continue;
				}

				FSwitcherEmissiveMaterialState& State = EmissiveMaterialStates.AddDefaulted_GetRef();
				State.Material = DynamicMaterial;
				for (const FName ParameterName : MatchingScalarNames)
				{
					State.OriginalScalarValues.Add(
						ParameterName,
						DynamicMaterial->K2_GetScalarParameterValue(ParameterName));
					++ControlledParameterCount;
				}
				for (const FName ParameterName : MatchingVectorNames)
				{
					State.OriginalVectorValues.Add(
						ParameterName,
						DynamicMaterial->K2_GetVectorParameterValue(ParameterName));
					++ControlledParameterCount;
				}
			}
		}
	}

	UE_LOG(LogTemp, Log,
		TEXT("[%s] Emissive control initialized: %d material(s), %d parameter(s), %d explicit actor(s)"),
		*GetName(), EmissiveMaterialStates.Num(), ControlledParameterCount, EmissiveActors.Num());
}

int32 ASwitcher_Env::ApplyEmissiveStateToLinkedActors(bool bNewIsLightOn)
{
	InitializeEmissiveMaterials();

	int32 AppliedParameterCount = 0;
	for (FSwitcherEmissiveMaterialState& State : EmissiveMaterialStates)
	{
		UMaterialInstanceDynamic* DynamicMaterial = State.Material.Get();
		if (!IsValid(DynamicMaterial))
		{
			continue;
		}

		for (const TPair<FName, float>& Pair : State.OriginalScalarValues)
		{
			DynamicMaterial->SetScalarParameterValue(Pair.Key, bNewIsLightOn ? Pair.Value : 0.0f);
			++AppliedParameterCount;
		}
		for (const TPair<FName, FLinearColor>& Pair : State.OriginalVectorValues)
		{
			DynamicMaterial->SetVectorParameterValue(
				Pair.Key,
				bNewIsLightOn ? Pair.Value : FLinearColor::Black);
			++AppliedParameterCount;
		}
	}

	return AppliedParameterCount;
}

void ASwitcher_Env::ServerInteract_Implementation(AActor* Interactor)
{
	UE_LOG(LogTemp, Log, TEXT("ServerInteract_Implementation"));

	const bool bTurningOn = !bIsLightOn;
	const bool bStateChanged = SetLightState(bTurningOn);
	if (bStateChanged && bTurningOn && ThreatIncreaseWhenTurnedOn > 0.0f)
	{
		if (AScareDirector* Director = AScareDirector::GetHuntDirector(this))
		{
			Director->AddThreatWithReason(
				ThreatIncreaseWhenTurnedOn,
				FString::Printf(
					TEXT("Player %s turned on switch %s"),
					*GetNameSafe(Interactor),
					*GetName()));
		}
		else
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[%s] Light turned ON, but no ScareDirector was found; Threat was not changed."),
				*GetName());
		}
	}
}

bool ASwitcher_Env::SetLightState(bool bNewIsLightOn)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("SetLightState ignored for %s because it was not called on the server"),
			*GetName());
		return false;
	}

	if (bIsLightOn == bNewIsLightOn)
	{
		// An aggression transition must enforce OFF even when the replicated bool
		// was already false but a Blueprint or sequencer re-enabled a component.
		ApplyLightStateToLinkedActors(bNewIsLightOn);
		return false;
	}

	bIsLightOn = bNewIsLightOn;

	// RepNotify runs on clients; call it manually so the server reacts too.
	OnRep_Switch();
	ForceNetUpdate();
	return true;
}
