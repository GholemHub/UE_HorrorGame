#include "Ritual/RitualChairAudioReplication.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

namespace RitualChairAudio
{
	const TCHAR* CreakSoundPath =
		TEXT("/Game/HorrorEngine/Audio/Interactions/S_Creak_06.S_Creak_06");
	const FName RitualChairClassName(TEXT("BP_RitualChair_C"));

	AActor* FindClosestRitualChair(UWorld* World, const FVector& RitualOrigin)
	{
		AActor* ClosestChair = nullptr;
		double ClosestDistanceSquared = TNumericLimits<double>::Max();

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Candidate = *It;
			if (!IsValid(Candidate)
				|| !IsValid(Candidate->GetClass())
				|| Candidate->GetClass()->GetFName() != RitualChairClassName)
			{
				continue;
			}

			const double DistanceSquared = FVector::DistSquared(
				Candidate->GetActorLocation(),
				RitualOrigin);
			if (DistanceSquared < ClosestDistanceSquared)
			{
				ClosestDistanceSquared = DistanceSquared;
				ClosestChair = Candidate;
			}
		}

		return ClosestChair;
	}
}

void PlayRitualChairStartAudioForRemoteClient(
	const UObject* WorldContextObject,
	const FVector& RitualOrigin)
{
	if (!IsValid(WorldContextObject))
	{
		return;
	}

	UWorld* World = WorldContextObject->GetWorld();
	// The existing Blueprint plays the sound for standalone and the listen-server
	// host. This path supplies the missing local playback only to remote clients.
	if (!IsValid(World) || World->GetNetMode() != NM_Client)
	{
		return;
	}

	USoundBase* CreakSound = LoadObject<USoundBase>(
		nullptr,
		RitualChairAudio::CreakSoundPath);
	if (!IsValid(CreakSound))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[RitualChairAudio] Could not load ritual-start sound '%s'"),
			RitualChairAudio::CreakSoundPath);
		return;
	}

	AActor* RitualChair = RitualChairAudio::FindClosestRitualChair(
		World,
		RitualOrigin);
	const FVector SoundLocation = IsValid(RitualChair)
		? RitualChair->GetActorLocation()
		: RitualOrigin;

	UGameplayStatics::PlaySoundAtLocation(World, CreakSound, SoundLocation);
	UE_LOG(LogTemp, Log,
		TEXT("[RitualChairAudio] Client played ritual-start creak at %s"),
		*GetNameSafe(RitualChair));
}
