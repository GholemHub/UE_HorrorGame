// Fill out your copyright notice in the Description page of Project Settings.


#include "Enviroment/Switcher_Env.h"
#include "Net/UnrealNetwork.h"
#include "Enviroment/Light_Env.h"


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
}


void ASwitcher_Env::OnRep_Switch()
{
	UE_LOG(LogTemp, Log, TEXT("OnRep_Switch"));
	if (LightActor)
	{
		LightActor->OnSwith(bIsLightOn);
	}
}

void ASwitcher_Env::ServerInteract_Implementation(AActor* Interactor)
{
	UE_LOG(LogTemp, Log, TEXT("ServerInteract_Implementation"));


	// 1. Change the variable
	bIsLightOn = !bIsLightOn;

	// 2. Manually trigger the update on the Server
	OnRep_Switch();
}