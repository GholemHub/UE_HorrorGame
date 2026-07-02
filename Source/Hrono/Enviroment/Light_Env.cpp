// Fill out your copyright notice in the Description page of Project Settings.


#include "Enviroment/Light_Env.h"
#include "Light_Env.h"

// Sets default values
ALight_Env::ALight_Env()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Light = CreateDefaultSubobject<USpotLightComponent>(TEXT("Light"));
	Light->SetupAttachment(RootComponent);

	Light->Intensity = 5000.f;
	Light->LightColor = FColor::White;
	bReplicates = true;
}

// Called when the game starts or when spawned
void ALight_Env::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALight_Env::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALight_Env::OnSwith(bool NewState)
{
	Light->SetVisibility(NewState);
	UE_LOG(LogTemp, Log, TEXT("OnSwith"));
	GEngine->AddOnScreenDebugMessage(
		1,
		10.f,
		FColor::Yellow,
		FString::Printf(TEXT("OnSwith :: %i"), NewState)
	);

}

