// Fill out your copyright notice in the Description page of Project Settings.


#include "Enviroment/Switcher_Env.h"
#include "Components/LightComponentBase.h"
#include "Net/UnrealNetwork.h"
#include "Enviroment/Light_Env.h"
#include "Kismet/GameplayStatics.h"
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

	return AppliedLightCount;
}

void ASwitcher_Env::ServerInteract_Implementation(AActor* Interactor)
{
	UE_LOG(LogTemp, Log, TEXT("ServerInteract_Implementation"));

	SetLightState(!bIsLightOn);
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
