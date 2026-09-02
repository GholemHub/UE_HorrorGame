#include "Items/RitualGoatSkull.h"

#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "GameplayTagContainer.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Ritual/CursedRoomRitual.h"
#include "Sound/SoundBase.h"

DEFINE_LOG_CATEGORY_STATIC(LogRitualSkull, Log, All);

ARitualGoatSkull::ARitualGoatSkull()
{
	ItemTimeline = EItemTimeline::Both;
	ItemTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Item.Ritual.GoatSkull"), false));

	// Actor physics replication follows the root rigid body. This also prevents
	// this skull's mesh from detaching from a non-physical scene root on drop.
	ItemMesh->SetupAttachment(nullptr);
	SetRootComponent(ItemMesh);
	DefaultSceneRoot->SetupAttachment(ItemMesh);
	ItemMesh->SetIsReplicated(true);

	DestructibleSkull = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("DestructibleSkull"));
	DestructibleSkull->SetupAttachment(ItemMesh);
	DestructibleSkull->SetVisibility(false, true);
	DestructibleSkull->SetHiddenInGame(true, true);
	DestructibleSkull->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DestructibleSkull->SetGenerateOverlapEvents(false);
	DestructibleSkull->SetSimulatePhysics(false);

	RitualAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("RitualAudio"));
	RitualAudioComponent->SetupAttachment(ItemMesh);
	RitualAudioComponent->bAutoActivate = false;
	RitualAudioComponent->bStopWhenOwnerDestroyed = true;
}

void ARitualGoatSkull::BeginPlay()
{
	Super::BeginPlay();
	if (RitualAudioComponent)
	{
		RitualAudioComponent->OnAudioFinished.AddUniqueDynamic(
			this,
			&ARitualGoatSkull::HandleRitualAudioFinished);
	}
}

void ARitualGoatSkull::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ARitualGoatSkull, bDestroyedByRitual);
	DOREPLIFETIME(ARitualGoatSkull, RitualAudioState);
}

bool ARitualGoatSkull::TryPickUp(AHronoCharacter* Character)
{
	return !bRitualLocked && !bDestroyedByRitual && Super::TryPickUp(Character);
}

void ARitualGoatSkull::Drop()
{
	const bool bWasHeld = bIsPickedUp && OwningCharacter != nullptr;
	Super::Drop();

	if (bWasHeld && HasAuthority() && !bIsPickedUp)
	{
		if (ACursedRoomRitual* Ritual = ACursedRoomRitual::FindRitual(this))
		{
			Ritual->NotifySkullDropped(this);
		}
	}
}

void ARitualGoatSkull::UpdateVisibilityForLocalPlayer(EItemTimeline ViewerTimeline)
{
	Super::UpdateVisibilityForLocalPlayer(ViewerTimeline);
	const bool bVisibleInTimeline =
		ItemTimeline == EItemTimeline::Both || ItemTimeline == ViewerTimeline;
	if (bVisibleInTimeline && RitualAudioState == ERitualSkullAudioState::Playing)
	{
		// ItemTimeline can become locally visible after the audio RepNotify.
		// Retry here so the correct client cannot permanently miss the sound.
		StartRitualSound(false);
	}
	else if (!bVisibleInTimeline)
	{
		StopLocalRitualLoopForTimelineVisibility();
	}

	if (!bDestroyedByRitual)
	{
		return;
	}

	if (ItemMesh)
	{
		ItemMesh->SetVisibility(false, true);
	}
	if (DestructibleSkull)
	{
		const bool bShowDebris = bChaosDestructionActivated && bVisibleInTimeline;
		DestructibleSkull->SetVisibility(bShowDebris, true);
		DestructibleSkull->SetHiddenInGame(!bShowDebris, true);
	}
}

void ARitualGoatSkull::SetRitualLocked(bool bLocked)
{
	bRitualLocked = bLocked;
	SetReplicateMovement(true);
}

void ARitualGoatSkull::SetRitualKinematic()
{
	SetRitualLocked(true);
	if (ItemMesh)
	{
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ARitualGoatSkull::SetRitualPhysics(bool bReverseGravity)
{
	SetRitualLocked(true);
	SetActorEnableCollision(true);
	if (ItemMesh)
	{
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ConfigureDroppedCollision(ItemMesh);
		ItemMesh->SetEnableGravity(!bReverseGravity);
		ItemMesh->SetSimulatePhysics(true);
		ItemMesh->WakeAllRigidBodies();
	}
}

void ARitualGoatSkull::RestoreNormalGravity()
{
	if (ItemMesh && !bDestroyedByRitual)
	{
		ItemMesh->SetEnableGravity(true);
		ItemMesh->SetSimulatePhysics(true);
		ItemMesh->WakeAllRigidBodies();
	}
}

void ARitualGoatSkull::ExplodeFromRitual()
{
	if (!HasAuthority() || bDestroyedByRitual)
	{
		return;
	}

	bDestroyedByRitual = true;
	ActivateChaosDestruction();
	ForceNetUpdate();

	UE_LOG(LogRitualSkull, Log, TEXT("[RitualSkull] %s exploded (Timeline=%s)"),
		*GetName(),
		*StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(ItemTimeline)));

	if (DebrisLifetime > 0.0f)
	{
		SetLifeSpan(DebrisLifetime);
	}
}

bool ARitualGoatSkull::CanPlayLocalSkullAudio() const
{
	return GetNetMode() != NM_DedicatedServer
		&& IsValid(ItemMesh)
		&& ItemMesh->IsVisible();
}

void ARitualGoatSkull::StartRitualSound(bool bPlayStartSound)
{
	bRitualSoundShouldBeActive = true;
	bPendingRitualStartSound |= bPlayStartSound;
	if (bRitualSoundRequested || !CanPlayLocalSkullAudio())
	{
		return;
	}
	bRitualSoundRequested = true;

	if (bPendingRitualStartSound && IsValid(RitualStartSound))
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			RitualStartSound,
			GetActorLocation(),
			FRotator::ZeroRotator,
			FMath::Max(0.0f, RitualSoundVolume));
	}
	bPendingRitualStartSound = false;

	if (IsValid(RitualAudioComponent) && IsValid(RitualLoopSound))
	{
		RitualAudioComponent->SetSound(RitualLoopSound);
		const float Volume = FMath::Max(0.0f, RitualSoundVolume);
		if (RitualSoundFadeInDuration > 0.0f)
		{
			RitualAudioComponent->FadeIn(RitualSoundFadeInDuration, Volume);
		}
		else
		{
			RitualAudioComponent->SetVolumeMultiplier(Volume);
			RitualAudioComponent->Play();
		}
	}

	UE_LOG(LogRitualSkull, Log,
		TEXT("[RitualAudio] Started on %s (Authority=%d NetMode=%d Start=%s Loop=%s)"),
		*GetName(), HasAuthority() ? 1 : 0, static_cast<int32>(GetNetMode()),
		*GetNameSafe(RitualStartSound), *GetNameSafe(RitualLoopSound));
}

void ARitualGoatSkull::StopRitualSound(bool bPlayEndSound)
{
	const bool bWasRequested = bRitualSoundRequested;
	bRitualSoundShouldBeActive = false;
	bPendingRitualStartSound = false;
	bRitualSoundRequested = false;

	if (IsValid(RitualAudioComponent) && RitualAudioComponent->IsPlaying())
	{
		if (RitualSoundFadeOutDuration > 0.0f)
		{
			RitualAudioComponent->FadeOut(RitualSoundFadeOutDuration, 0.0f);
		}
		else
		{
			RitualAudioComponent->Stop();
		}
	}

	if (bWasRequested && bPlayEndSound && IsValid(RitualEndSound))
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			RitualEndSound,
			GetActorLocation(),
			FRotator::ZeroRotator,
			FMath::Max(0.0f, RitualSoundVolume));
	}

	if (bWasRequested)
	{
		UE_LOG(LogRitualSkull, Log,
			TEXT("[RitualAudio] Stopped on %s (End=%s, FadeOut=%.2fs)"),
			*GetName(), *GetNameSafe(RitualEndSound), RitualSoundFadeOutDuration);
	}
}

void ARitualGoatSkull::StopLocalRitualLoopForTimelineVisibility()
{
	bRitualSoundRequested = false;
	if (IsValid(RitualAudioComponent) && RitualAudioComponent->IsPlaying())
	{
		RitualAudioComponent->Stop();
	}
}

void ARitualGoatSkull::SetRitualAudioState(ERitualSkullAudioState NewState)
{
	if (!HasAuthority())
	{
		UE_LOG(LogRitualSkull, Warning,
			TEXT("[RitualAudio] Non-authority tried to set audio state on %s"),
			*GetName());
		return;
	}

	if (RitualAudioState == NewState)
	{
		return;
	}

	RitualAudioState = NewState;
	ApplyRitualAudioState();
	ForceNetUpdate();
	UE_LOG(LogRitualSkull, Log,
		TEXT("[RitualAudio] Server state for %s -> %s"),
		*GetName(),
		*StaticEnum<ERitualSkullAudioState>()->GetNameStringByValue(
			static_cast<int64>(RitualAudioState)));
}

void ARitualGoatSkull::OnRep_RitualAudioState()
{
	ApplyRitualAudioState();
	UE_LOG(LogRitualSkull, Log,
		TEXT("[RitualAudio] Client received state for %s -> %s"),
		*GetName(),
		*StaticEnum<ERitualSkullAudioState>()->GetNameStringByValue(
			static_cast<int64>(RitualAudioState)));
}

void ARitualGoatSkull::ApplyRitualAudioState()
{
	switch (RitualAudioState)
	{
	case ERitualSkullAudioState::Playing:
		StartRitualSound(true);
		break;
	case ERitualSkullAudioState::Finished:
		StopRitualSound(true);
		break;
	case ERitualSkullAudioState::Silent:
	default:
		StopRitualSound(false);
		break;
	}
}

void ARitualGoatSkull::HandleRitualAudioFinished()
{
	// This makes an ordinary SoundWave usable as the ritual loop too. StopRitualSound
	// clears the request before Stop/FadeOut, so it cannot accidentally restart here.
	if (bRitualSoundShouldBeActive && bRitualSoundRequested && CanPlayLocalSkullAudio()
		&& IsValid(RitualAudioComponent) && IsValid(RitualLoopSound))
	{
		RitualAudioComponent->Play();
	}
}

void ARitualGoatSkull::OnRep_DestroyedByRitual()
{
	if (bDestroyedByRitual)
	{
		ActivateChaosDestruction();
	}
}

void ARitualGoatSkull::ActivateChaosDestruction()
{
	if (bChaosDestructionActivated)
	{
		return;
	}

	const bool bWasLocallyAudible = CanPlayLocalSkullAudio();
	bChaosDestructionActivated = true;
	SetRitualLocked(true);

	if (ItemMesh)
	{
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ItemMesh->SetVisibility(false, true);
	}

	if (DestructibleSkull && DestructibleSkull->GetRestCollection())
	{
		DestructibleSkull->SetHiddenInGame(false, true);
		DestructibleSkull->SetVisibility(true, true);
		DestructibleSkull->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ConfigureDroppedCollision(DestructibleSkull);
		DestructibleSkull->SetSimulatePhysics(true);
		DestructibleSkull->CrumbleActiveClusters();
	}
	else
	{
		UE_LOG(LogRitualSkull, Warning,
			TEXT("[RitualSkull] %s has no Geometry Collection assigned; using hidden-mesh fallback"),
			*GetName());
	}

	if (bWasLocallyAudible && IsValid(SkullBreakSound))
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			SkullBreakSound,
			GetActorLocation(),
			FRotator::ZeroRotator,
			FMath::Max(0.0f, RitualSoundVolume));
	}

	BP_OnRitualExplosion();
}
