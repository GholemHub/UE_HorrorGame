#include "Enviroment/PianoActor.h"

#include "Audio/HronoAudioPolicy.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "HronoCharacter.h"
#include "HronoCollisionChannels.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"

APianoActor::APianoActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(false);

	PianoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PianoMesh"));
	RootComponent = PianoMesh;
	PianoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	InteractionBox->SetupAttachment(PianoMesh);
	InteractionBox->SetBoxExtent(FVector(100.0f, 60.0f, 70.0f));
	InteractionBox->SetCollisionObjectType(ECC_WorldDynamic);
	InteractionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	InteractionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	InteractionBox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	InteractionBox->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
	InteractionBox->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, ECR_Block);
	InteractionBox->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, ECR_Block);
	InteractionBox->SetGenerateOverlapEvents(false);
}

void APianoActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (!bAutoSizeInteractionBox || !PianoMesh || !PianoMesh->GetStaticMesh() || !InteractionBox)
	{
		return;
	}

	FVector Min;
	FVector Max;
	PianoMesh->GetLocalBounds(Min, Max);
	InteractionBox->SetRelativeLocation((Min + Max) * 0.5f);
	InteractionBox->SetBoxExtent((Max - Min) * 0.5f + FVector(4.0f), true);
}

void APianoActor::Interact_Implementation(AActor* Interactor)
{
	const AHronoCharacter* Character = Cast<AHronoCharacter>(Interactor);
	if (!HasAuthority() || !IsValid(Character) || !IsValid(PianoSound)
		|| !IsValid(Character->GetController())
		|| (PianoTimeline != EItemTimeline::Both && Character->GetTimeline() != PianoTimeline))
	{
		return;
	}

	const double Now = GetWorld()->GetTimeSeconds();
	if (Now - LastPlayTimeSeconds < FMath::Max(0.0f, InteractionCooldownSeconds))
	{
		return;
	}
	LastPlayTimeSeconds = Now;
	MulticastPlayPiano(InteractionBox->GetComponentLocation(), PianoTimeline);
}

void APianoActor::MulticastPlayPiano_Implementation(FVector_NetQuantize SoundLocation,
	EItemTimeline EventTimeline)
{
	if (IsValid(PianoSound) && HronoAudioPolicy::CanHear(this, EventTimeline))
	{
		UGameplayStatics::PlaySoundAtLocation(this, PianoSound, SoundLocation,
			FRotator::ZeroRotator, 1.0f, 1.0f, 0.0f, SoundAttenuation);
	}
}
