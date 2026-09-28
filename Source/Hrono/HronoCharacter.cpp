// Copyright Epic Games, Inc. All Rights Reserved.

#include "HronoCharacter.h"
#include "Audio/HronoAudioPolicy.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Sound/SoundAttenuation.h"
#include "UObject/ConstructorHelpers.h"
#include "HronoPlayerController.h"
#include "Items/HidingWardrobe.h"
#include "HronoCollisionChannels.h"
#include "EngineUtils.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PrimitiveComponentUtilities.h"
#include "Hrono.h"
#include "Net/UnrealNetwork.h"
#include "Components/Drag_Component.h"
#include "Items/Drag_Item.h"
#include "Items/Chair.h"
#include "Components/SpotLightComponent.h"
#include "Interface/Enviroment_Interface.h"
#include "Items/Base_Item.h"
#include "Items/Clock.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Engine.h"
#include "Enviroment/PlayerVisibilityZone.h"
#include "Enviroment/OuijaBoard.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "UI/HronoMenuSettingsSaveGame.h"
#include "UI/HronoFpsWidget.h"
#include "UI/HronoTutorialWidget.h"
#include "Items/AxeItem.h"
#include "Items/Dozimetr.h"
#include "Items/PaintItem.h"
#include "Items/RitualGoatSkull.h"
#include "Ritual/TableRitualGate.h"

namespace
{
	FText GetTutorialStepText(EHronoTutorialStep Step)
	{
		switch (Step)
		{
		case EHronoTutorialStep::PickUpMonocle:
			return FText::FromString(TEXT("1/10  •  Take the monocle from the gray box."));
		case EHronoTutorialStep::FindPaintingAnomaly:
			return FText::FromString(TEXT("2/10  •  View an anomalous painting through the monocle."));
		case EHronoTutorialStep::InteractWithClock:
			return FText::FromString(TEXT("3/10  •  Interact with any clock."));
		case EHronoTutorialStep::DetectDosimeterAnomaly:
			return FText::FromString(TEXT("4/10  •  Follow the dosimeter's faster beeps."));
		case EHronoTutorialStep::StartCorrectRoomRitual:
			return FText::FromString(TEXT("5/10  •  Bring the skulls to the most anomalous room."));
		case EHronoTutorialStep::PickUpKey:
			return FText::FromString(TEXT("6/10  •  Pick up the key."));
		case EHronoTutorialStep::UnlockOfficeDoor:
			return FText::FromString(TEXT("7/10  •  Unlock the office door."));
		case EHronoTutorialStep::SeatAllPlayers:
			return FText::FromString(TEXT("8/10  •  Sit in the kitchen ritual chairs."));
		case EHronoTutorialStep::EnterDemonName:
			return FText::FromString(TEXT("9/10  •  Enter the demon's name: LEON."));
		case EHronoTutorialStep::UnitePlayerTimelines:
			return FText::FromString(TEXT("10/10  •  Insert the runes into the mirror."));
		case EHronoTutorialStep::Completed:
		default:
			return FText::FromString(TEXT("Tutorial complete."));
		}
	}

	void CompleteSeatedPlayersTutorial(UWorld* World)
	{
		if (!IsValid(World) || !TableRitualGate::AreAllPlayersSeatedAtRitualTable(World))
		{
			return;
		}

		for (TActorIterator<AHronoCharacter> It(World); It; ++It)
		{
			if (AHronoCharacter* Character = *It;
				IsValid(Character) && IsValid(Character->GetController()))
			{
				Character->CompleteTutorialStep(EHronoTutorialStep::SeatAllPlayers);
			}
		}
	}

	void DisablePlayerShadows(UPrimitiveComponent* Component)
	{
		if (!IsValid(Component))
		{
			return;
		}

		Component->SetCastShadow(false);
		Component->SetCastHiddenShadow(false);
		Component->SetCastContactShadow(false);

		if (USkeletalMeshComponent* SkeletalMesh = Cast<USkeletalMeshComponent>(Component))
		{
			SkeletalMesh->SetCastCapsuleDirectShadow(false);
			SkeletalMesh->SetCastCapsuleIndirectShadow(false);
		}
	}
}

void AHronoCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHronoCharacter, CharacterTimeline);
	DOREPLIFETIME(AHronoCharacter, bSprinting);
	DOREPLIFETIME(AHronoCharacter, bRecovering);
	DOREPLIFETIME(AHronoCharacter, CurrentStamina);
	DOREPLIFETIME(AHronoCharacter, MaxStamina);
	DOREPLIFETIME(AHronoCharacter, CurrentChair);
	DOREPLIFETIME(AHronoCharacter, bIsSitting);
	DOREPLIFETIME(AHronoCharacter, ReservedRitualChair);
	DOREPLIFETIME(AHronoCharacter, bIsAtRitualPoint);
	DOREPLIFETIME(AHronoCharacter, RitualDestinationActor);
	DOREPLIFETIME(AHronoCharacter, bIsSafeInHidingWardrobe);
	DOREPLIFETIME(AHronoCharacter, bTimelineMirrorRequested);
	DOREPLIFETIME(AHronoCharacter, CurrentHeldItem);
	DOREPLIFETIME_CONDITION(AHronoCharacter, TutorialStep, COND_OwnerOnly);
}

void AHronoCharacter::NotifyVoiceReceiverReady()
{
	bVoiceReceiverConfigured = true;
	if (AHronoPlayerController* RadioController = Cast<AHronoPlayerController>(GetController()))
	{
		RadioController->NotifyVoiceReceiverReady(this);
	}
}

void AHronoCharacter::SetSafeInHidingWardrobe(bool bNewSafe)
{
	// Compatibility wrapper. Protection is derived from actual registered volumes.
	RefreshWardrobeSafetySources();
}

void AHronoCharacter::UpdateWardrobeSafetySource(AHidingWardrobe* Source, bool bInside)
{
	if (!HasAuthority() || IsActorBeingDestroyed() || !Source) return;
	if (bInside && IsValid(Source) && Source->IsCharacterInsideSafetyVolume(this))
	{
		WardrobeSafetySources.Add(Source);
		Source->OnDestroyed.AddUniqueDynamic(this, &AHronoCharacter::OnWardrobeSafetySourceDestroyed);
	}
	else
	{
		WardrobeSafetySources.Remove(Source);
		if (IsValid(Source)) Source->OnDestroyed.RemoveDynamic(this, &AHronoCharacter::OnWardrobeSafetySourceDestroyed);
	}
	RefreshWardrobeSafetySources();
}

void AHronoCharacter::OnWardrobeSafetySourceDestroyed(AActor* DestroyedActor)
{
	UpdateWardrobeSafetySource(Cast<AHidingWardrobe>(DestroyedActor), false);
}

void AHronoCharacter::RefreshWardrobeSafetySources()
{
	if (!HasAuthority() || IsActorBeingDestroyed() || bApplyingTimelineTransition) return;
	if (bRefreshingWardrobeSafetySources)
	{
		bWardrobeSafetyRefreshPending = true;
		return;
	}
	TGuardValue<bool> RefreshGuard(bRefreshingWardrobeSafetySources, true);
	do
	{
		bWardrobeSafetyRefreshPending = false;
		bool bAnySourceSafe = false;
		for (auto It = WardrobeSafetySources.CreateIterator(); It; ++It)
		{
			AHidingWardrobe* Source = It->Get();
			if (!IsValid(Source) || !Source->IsCharacterInsideSafetyVolume(this))
			{
				if (IsValid(Source)) Source->OnDestroyed.RemoveDynamic(this, &AHronoCharacter::OnWardrobeSafetySourceDestroyed);
				It.RemoveCurrent();
				continue;
			}
			bAnySourceSafe |= Source->CanProvideSafetyFor(this);
		}
		ApplyDerivedWardrobeSafety(bAnySourceSafe);
		// Delegate callbacks can remove a source. Recompute after dispatch, not
		// recursively while a prior notification is still being delivered.
	} while (bWardrobeSafetyRefreshPending && !IsActorBeingDestroyed());
}

void AHronoCharacter::ApplyDerivedWardrobeSafety(bool bNewSafe)
{
	if (!HasAuthority() || bIsSafeInHidingWardrobe == bNewSafe)
	{
		return;
	}

	const bool bPreviousSafe = bIsSafeInHidingWardrobe;
	bIsSafeInHidingWardrobe = bNewSafe;
	UE_LOG(LogTemp, Log, TEXT("[WardrobeSafety] Character=%s Safe=%s"),
		*GetNameSafe(this),
		bIsSafeInHidingWardrobe ? TEXT("true") : TEXT("false"));
	OnRep_IsSafeInHidingWardrobe(bPreviousSafe);
	ForceNetUpdate();
}

void AHronoCharacter::OnRep_IsSafeInHidingWardrobe(bool bPreviousSafe)
{
	OnHidingSafetyChanged.Broadcast(bIsSafeInHidingWardrobe);
	if (bPreviousSafe && !bIsSafeInHidingWardrobe)
	{
		OnHidingWardrobeSafetyLost.Broadcast(this);
	}
}

AHronoCharacter::AHronoCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	static ConstructorHelpers::FObjectFinder<USoundBase> WoodStep(
		TEXT("/Game/HorrorEngine/Audio/Footsteps/S_FT_Wood_Cue.S_FT_Wood_Cue"));
	static ConstructorHelpers::FObjectFinder<USoundBase> WoodJump(
		TEXT("/Game/HorrorEngine/Audio/Footsteps/S_FT_Wood_Jump_Cue.S_FT_Wood_Jump_Cue"));
	static ConstructorHelpers::FObjectFinder<USoundBase> RockStep(
		TEXT("/Game/HorrorEngine/Audio/Footsteps/S_FT_Rock_Cue.S_FT_Rock_Cue"));
	static ConstructorHelpers::FObjectFinder<USoundBase> GrassStep(
		TEXT("/Game/HorrorEngine/Audio/Footsteps/S_FT_Grass_Cue.S_FT_Grass_Cue"));
	static ConstructorHelpers::FObjectFinder<USoundBase> MetalStep(
		TEXT("/Game/HorrorEngine/Audio/Footsteps/S_FT_Metal_Cue.S_FT_Metal_Cue"));
	static ConstructorHelpers::FObjectFinder<USoundBase> CarpetStep(
		TEXT("/Game/HorrorEngine/Audio/Footsteps/S_FT_Carpet_Cue.S_FT_Carpet_Cue"));
	static ConstructorHelpers::FObjectFinder<USoundAttenuation> GeneralAttenuation(
		TEXT("/Game/HorrorEngine/Audio/_SoundSettings/ATT_General.ATT_General"));
	NativeFootstepFallback = WoodStep.Object;
	NativeJumpFallback = WoodJump.Object;
	NativeSurfaceFootstepFallbacks.Add(SurfaceType1, WoodStep.Object);
	NativeSurfaceFootstepFallbacks.Add(SurfaceType2, RockStep.Object);
	NativeSurfaceFootstepFallbacks.Add(SurfaceType3, GrassStep.Object);
	NativeSurfaceFootstepFallbacks.Add(SurfaceType4, MetalStep.Object);
	NativeSurfaceFootstepFallbacks.Add(SurfaceType5, CarpetStep.Object);
	MovementSoundAttenuation = GeneralAttenuation.Object;
	TutorialGameStageText = NSLOCTEXT("HronoTutorial", "CharacterDefaultStage", "STAGE  •  INVESTIGATION");

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));
	DisablePlayerShadows(FirstPersonMesh);
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_CHANNEL_ITEM, ECR_Ignore);

	// Mount the first-person camera on the upper chest so locomotion animation
	// naturally produces a body-camera feel.
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("spine_03"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	// WorldSpaceRepresentation uses UE's combined first-person RT mask, which
	// MegaLights traces even when CastShadow is disabled. Keep this as a regular
	// primitive so reflection and shadow visibility can be controlled separately.
	GetMesh()->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::None);
	DisablePlayerShadows(GetMesh());

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;

	InteractionPoint = CreateDefaultSubobject<USceneComponent>(TEXT("InteractionPoint"));
	InteractionPoint->SetupAttachment(GetFirstPersonCameraComponent());

	// The authored Future point remains the base pose. Past gets a separate child
	// point so its offset can be tuned in HE_CharacterHrono1 without duplicating
	// the existing camera-relative setup.
	PastInteractionPoint = CreateDefaultSubobject<USceneComponent>(TEXT("PastInteractionPoint"));
	PastInteractionPoint->SetupAttachment(InteractionPoint);
	PastInteractionPoint->SetRelativeLocation(FVector(10.0f, -20.0f, -8.0f));

	MonocleInteractionPoint = CreateDefaultSubobject<USceneComponent>(TEXT("MonocleInteractionPoint"));
	MonocleInteractionPoint->SetupAttachment(GetFirstPersonCameraComponent());
	// The former Future monocle root was 80 - 30 = 50 cm ahead of the camera.
	// Keeping that depth avoids changing its apparent size; zero Y/Z centres it.
	MonocleInteractionPoint->SetRelativeLocation(FVector(50.0f, 0.0f, 0.0f));

	SpotLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotLight1"));
	SpotLight->SetupAttachment(GetFirstPersonCameraComponent());

	SpotLight->SetRelativeLocationAndRotation(FVector(30.0f, 17.5f, -5.0f), FRotator(-18.6f, -1.3f, 5.26f));
	SpotLight->Intensity = 0.5;
	SpotLight->SetIntensityUnits(ELightUnits::Lumens);
	SpotLight->AttenuationRadius = 1050.0f;
	SpotLight->InnerConeAngle = 18.7f;
	SpotLight->OuterConeAngle = 45.24f;
	// Keep the local flashlight in the main view, but exclude its direct-light
	// contribution from Hardware Lumen / ray-traced mirror reflections.
	SpotLight->SetAffectReflection(false);


	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

}

void AHronoCharacter::OnRep_CharacterTimeline(EItemTimeline PreviousTimeline)
{
	RefreshHeldItemsInteractionPoint();
	ApplyTimelineCollision();
	ApplyMirrorFromCharacterTimeline();
	RefreshTimelineVisibilityForLocalPlayer();

	if (PreviousTimeline != CharacterTimeline)
	{
		OnCharacterTimelineChanged.Broadcast(PreviousTimeline, CharacterTimeline);
	}
}

USceneComponent* AHronoCharacter::GetActiveInteractionPoint() const
{
	if (CharacterTimeline == EItemTimeline::Past && IsValid(PastInteractionPoint))
	{
		return PastInteractionPoint;
	}

	return InteractionPoint;
}

USceneComponent* AHronoCharacter::GetHeldItemInteractionPoint(const ABase_Item* Item) const
{
	if (IsValid(Item) && Item->bUseCenteredInteractionPoint)
	{
		return MonocleInteractionPoint;
	}
	return GetActiveInteractionPoint();
}

void AHronoCharacter::OnRep_TimelineMirrorRequested()
{
	SetMirroredViewEnabled(bTimelineMirrorRequested);
}

void AHronoCharacter::SwitchPlayerTimeline()
{
	if (!HasAuthority()) return;
	const EItemTimeline Target = CharacterTimeline == EItemTimeline::Past
		? EItemTimeline::Future : EItemTimeline::Past;
	TrySetPlayerTimelineOnAuthority(Target);
}

void AHronoCharacter::SetPlayerTimeline(EItemTimeline NewTimeline)
{
	TrySetPlayerTimelineOnAuthority(NewTimeline);
}

bool AHronoCharacter::TrySetPlayerTimelineOnAuthority(EItemTimeline NewTimeline)
{
	if (!HasAuthority() || IsActorBeingDestroyed() || bApplyingTimelineTransition
		|| (NewTimeline != EItemTimeline::Past && NewTimeline != EItemTimeline::Future)
		|| (CharacterTimeline != EItemTimeline::Past && CharacterTimeline != EItemTimeline::Future)) return false;
	// A different authoritative transition supersedes an unfinished death.
	if (NewTimeline != CharacterTimeline) CancelDeathTimelineTransition();
	const bool bApplied = ApplyPlayerTimelineOnAuthority(NewTimeline);
	// Overlap notifications during collision changes register their sources but
	// defer the aggregate/event until the transition guard has been released.
	RefreshWardrobeSafetySources();
	return bApplied && !IsActorBeingDestroyed() && CharacterTimeline == NewTimeline;
}

void AHronoCharacter::ServerSetPlayerTimeline_Implementation(EItemTimeline NewTimeline)
{
	// Retained wire signature for older Blueprints/clients. A client's chosen
	// timeline is not a gameplay authorization and can never mutate server state.
	UE_LOG(LogTemp, Verbose, TEXT("[TimelineSwitch] Ignored legacy client request for %s"), *GetNameSafe(this));
}

bool AHronoCharacter::BeginDeathTimelineTransition(USkeletalMeshComponent* DeathMesh)
{
	if (IsActorBeingDestroyed()) return false;
	if (!HasAuthority()) return true; // Presentation only; no server mutation/RPC.
	if (bDeathTimelineTransitionPending || bApplyingTimelineTransition
		|| (CharacterTimeline != EItemTimeline::Past && CharacterTimeline != EItemTimeline::Future)
		|| (DeathMesh && (!IsValid(DeathMesh) || DeathMesh->GetOwner() != this))) return false;
	DeathOriginalTimeline = CharacterTimeline;
	DeathTargetTimeline = CharacterTimeline == EItemTimeline::Past ? EItemTimeline::Future : EItemTimeline::Past;
	bDeathTimelineTransitionPending = true;
	if (DeathMesh)
	{
		DeathTransitionMesh = DeathMesh;
		DeathMeshPreviousTickOption = static_cast<uint8>(DeathMesh->VisibilityBasedAnimTickOption);
		// Montage completion must run even on a server that renders no character.
		DeathMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;
	}
	return true;
}

void AHronoCharacter::CancelDeathTimelineTransition()
{
	if (!HasAuthority()) return;
	bDeathTimelineTransitionPending = false;
	DeathTargetTimeline = EItemTimeline::Both;
	if (USkeletalMeshComponent* DeathMesh = DeathTransitionMesh.Get())
		DeathMesh->VisibilityBasedAnimTickOption = static_cast<EVisibilityBasedAnimTickOption>(DeathMeshPreviousTickOption);
	DeathTransitionMesh.Reset();
}

bool AHronoCharacter::CompleteDeathTimelineTransition()
{
	if (!HasAuthority() || !bDeathTimelineTransitionPending || IsActorBeingDestroyed()) return false;
	const EItemTimeline Target = DeathTargetTimeline;
	CancelDeathTimelineTransition(); // Consume before any transition callbacks.
	return TrySetPlayerTimelineOnAuthority(Target);
}

void AHronoCharacter::ClientApplyTimelineMirror_Implementation(EItemTimeline NewTimeline)
{
	SetMirroredViewEnabled(NewTimeline == EItemTimeline::Past);
}

bool AHronoCharacter::ApplyPlayerTimelineOnAuthority(EItemTimeline NewTimeline)
{
	if (!HasAuthority() || bApplyingTimelineTransition || (NewTimeline != EItemTimeline::Past && NewTimeline != EItemTimeline::Future))
	{
		return false;
	}

	TGuardValue<bool> TransitionGuard(bApplyingTimelineTransition, true);
	if (NewTimeline == CharacterTimeline)
	{
		// A duplicate client/server death notification may request the target that
		// was already applied. Reinforce presentation without changing state again.
		bTimelineMirrorRequested = CharacterTimeline == EItemTimeline::Past;
		RefreshHeldItemsInteractionPoint();
		OnRep_TimelineMirrorRequested();
		ClientApplyTimelineMirror(CharacterTimeline);
		UE_LOG(LogTemp, Log,
			TEXT("[TimelineSwitch] Duplicate target ignored for %s: already %s"),
			*GetNameSafe(this),
			*StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(CharacterTimeline)));
		return true;
	}

	const EItemTimeline PreviousTimeline = CharacterTimeline;
	CharacterTimeline = NewTimeline;
	bTimelineMirrorRequested = NewTimeline == EItemTimeline::Past;

	MoveCarriedItemsToTimeline(NewTimeline);
	RefreshHeldItemsInteractionPoint();
	ApplyTimelineCollision();
	OnRep_TimelineMirrorRequested();
	ClientApplyTimelineMirror(NewTimeline);
	RefreshTimelineVisibilityForLocalPlayer();

	if (PreviousTimeline != CharacterTimeline)
	{
		OnCharacterTimelineChanged.Broadcast(PreviousTimeline, CharacterTimeline);
	}

	UE_LOG(LogTemp, Log,
		TEXT("[TimelineSwitch] Character=%s %s -> %s HeldItem=%s Mirrored=%s"),
		*GetNameSafe(this),
		*StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(PreviousTimeline)),
		*StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(CharacterTimeline)),
		*GetNameSafe(CurrentHeldItem),
		bTimelineMirrorRequested ? TEXT("true") : TEXT("false"));

	ForceNetUpdate();
	return !IsActorBeingDestroyed() && CharacterTimeline == NewTimeline;
}

void AHronoCharacter::MoveCarriedItemsToTimeline(EItemTimeline NewTimeline)
{
	if (!HasAuthority() || !IsValid(CurrentHeldItem))
	{
		return;
	}

	CurrentHeldItem->SetItemTimeline(NewTimeline);
}

void AHronoCharacter::RefreshHeldItemsInteractionPoint()
{
	if (IsValid(CurrentHeldItem) && CurrentHeldItem->OwningCharacter == this)
	{
		CurrentHeldItem->RefreshHeldAttachmentPoint();
	}
}

void AHronoCharacter::RefreshTimelineVisibilityForLocalPlayer()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APlayerController* LocalPlayerController = World->GetFirstPlayerController();
	AHronoCharacter* LocalViewer = LocalPlayerController
		? Cast<AHronoCharacter>(LocalPlayerController->GetPawn())
		: nullptr;
	if (!LocalViewer || !LocalViewer->IsLocallyControlled())
	{
		return;
	}

	const EItemTimeline ViewerTimeline = LocalViewer->CharacterTimeline;
	auto& PrimitiveOwnerNoSeeStates = LocalViewer->TimelinePrimitiveOwnerNoSeeStates;
	auto& HiddenCharacters = LocalViewer->TimelineHiddenCharacters;

	for (auto StateIt = PrimitiveOwnerNoSeeStates.CreateIterator(); StateIt; ++StateIt)
	{
		if (!StateIt.Key().IsValid())
		{
			StateIt.RemoveCurrent();
		}
	}
	for (auto CharacterIt = HiddenCharacters.CreateIterator(); CharacterIt; ++CharacterIt)
	{
		if (!CharacterIt->IsValid())
		{
			CharacterIt.RemoveCurrent();
		}
	}

	for (TActorIterator<ABase_Item> It(World); It; ++It)
	{
		if (ABase_Item* TimelineActor = *It)
		{
			TimelineActor->UpdateVisibilityForLocalPlayer(ViewerTimeline);
		}
	}
	for (TActorIterator<AOuijaBoard> It(World); It; ++It)
	{
		It->RefreshPastReadability(ViewerTimeline);
	}

	for (TActorIterator<AHronoCharacter> It(World); It; ++It)
	{
		AHronoCharacter* OtherCharacter = *It;
		if (!IsValid(OtherCharacter))
		{
			continue;
		}

		// The first-person arms remain owner-only. The inherited Character mesh is
		// the network/world representation and must never be OnlyOwnerSee.
		if (USkeletalMeshComponent* WorldCharacterMesh = OtherCharacter->GetMesh())
		{
			WorldCharacterMesh->SetOnlyOwnerSee(false);
			WorldCharacterMesh->SetOwnerNoSee(true);
			WorldCharacterMesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::None);
		}
		if (USkeletalMeshComponent* ArmsMesh = OtherCharacter->GetFirstPersonMesh())
		{
			ArmsMesh->SetOnlyOwnerSee(true);
			ArmsMesh->SetOwnerNoSee(false);
		}

		TInlineComponentArray<UPrimitiveComponent*> RenderComponents(OtherCharacter);

		// Player shadows are disabled on every client. The local player does not
		// need any RT representation because it is neither reflected nor shadowed.
		if (OtherCharacter == LocalViewer)
		{
			for (UPrimitiveComponent* RenderComponent : RenderComponents)
			{
				DisablePlayerShadows(RenderComponent);
				if (IsValid(RenderComponent))
				{
					RenderComponent->SetVisibleInRayTracing(false);
				}
			}
			continue;
		}

		// Remote player shadows are also disabled. Hidden remote players retain
		// only the RT representation required for their Lumen mirror reflection.
		for (UPrimitiveComponent* RenderComponent : RenderComponents)
		{
			if (IsValid(RenderComponent))
			{
				if (RenderComponent->FirstPersonPrimitiveType == EFirstPersonPrimitiveType::WorldSpaceRepresentation)
				{
					RenderComponent->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::None);
				}
				DisablePlayerShadows(RenderComponent);
			}
		}

		const EItemTimeline OtherTimeline = OtherCharacter->CharacterTimeline;
		const bool bSameTimeline = ViewerTimeline == EItemTimeline::Both
			|| OtherTimeline == EItemTimeline::Both
			|| ViewerTimeline == OtherTimeline;
		const bool bRevealedByVisibilityZone =
			APlayerVisibilityZone::ShouldRevealToViewer(World, LocalViewer, OtherCharacter);
		const TWeakObjectPtr<AHronoCharacter> OtherKey(OtherCharacter);

		if (!bSameTimeline && !bRevealedByVisibilityZone)
		{
			const bool bWasAlreadyHidden = HiddenCharacters.Contains(OtherKey);
			for (UPrimitiveComponent* RenderComponent : RenderComponents)
			{
				if (!IsValid(RenderComponent))
				{
					continue;
				}

				// First-person arms are never the remote world representation.
				if (RenderComponent == OtherCharacter->GetFirstPersonMesh())
				{
					RenderComponent->SetVisibleInRayTracing(false);
					continue;
				}

				const TWeakObjectPtr<UPrimitiveComponent> ComponentKey(RenderComponent);
				if (!PrimitiveOwnerNoSeeStates.Contains(ComponentKey))
				{
					PrimitiveOwnerNoSeeStates.Add(ComponentKey, RenderComponent->bOwnerNoSee);
				}
				// Additional visibility ownership makes OwnerNoSee local to this
				// viewer. Keep the primitive in the hardware RT reflection mask; its
				// shadow mask was disabled above because this is a remote player.
				RenderComponent->SetOwnerNoSee(true);
				UPrimitiveComponentUtilities::AddVisibilityOwner(RenderComponent, LocalViewer);
				RenderComponent->SetVisibleInRayTracing(true);
			}
			HiddenCharacters.Add(OtherKey);

			if (!bWasAlreadyHidden)
			{
				UE_LOG(LogTemp, Log,
					TEXT("[TimelinePlayerVisibility] Viewer=%s(%s) hides Player=%s(%s)"),
					*GetNameSafe(LocalViewer),
					*StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(ViewerTimeline)),
					*GetNameSafe(OtherCharacter),
					*StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(OtherTimeline)));
			}
			continue;
		}

		const bool bWasHidden = HiddenCharacters.Remove(OtherKey) > 0;
		for (UPrimitiveComponent* RenderComponent : RenderComponents)
		{
			if (!IsValid(RenderComponent))
			{
				continue;
			}

			UPrimitiveComponentUtilities::RemoveVisibilityOwner(RenderComponent, LocalViewer);
			const TWeakObjectPtr<UPrimitiveComponent> ComponentKey(RenderComponent);
			if (const bool* PreviousOwnerNoSee = PrimitiveOwnerNoSeeStates.Find(ComponentKey))
			{
				RenderComponent->SetOwnerNoSee(*PreviousOwnerNoSee);
				PrimitiveOwnerNoSeeStates.Remove(ComponentKey);
			}
			// A same-timeline player may be visible directly, but must not appear
			// in the Lumen mirror.
			RenderComponent->SetVisibleInRayTracing(false);
		}

		if (bWasHidden)
		{
			UE_LOG(LogTemp, Log,
				TEXT("[TimelinePlayerVisibility] Viewer=%s(%s) shows Player=%s(%s)"),
				*GetNameSafe(LocalViewer),
				*StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(ViewerTimeline)),
				*GetNameSafe(OtherCharacter),
				*StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(OtherTimeline)));
		}
	}
}

bool AHronoCharacter::EnsureMirrorPostProcessInstance()
{
	if (MirrorPostProcessInstance)
	{
		return true;
	}

	if (!IsLocallyControlled() || !FirstPersonCameraComponent)
	{
		return false;
	}

	if (!MirrorPostProcessMaterial)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[MirrorView] %s has no MirrorPostProcessMaterial. Assign M_PP_MirrorPast in the character Blueprint."),
			*GetNameSafe(this));
		return false;
	}

	MirrorPostProcessInstance = UMaterialInstanceDynamic::Create(
		MirrorPostProcessMaterial,
		this,
		TEXT("MirrorPostProcessInstance"));

	if (!MirrorPostProcessInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("[MirrorView] Failed to create a dynamic material instance for %s"),
			*GetNameSafe(this));
		return false;
	}

	// Avoid applying the base material and its dynamic instance as two passes.
	// Two horizontal mirror passes would cancel one another.
	FirstPersonCameraComponent->RemoveBlendable(MirrorPostProcessMaterial);
	FirstPersonCameraComponent->AddOrUpdateBlendable(MirrorPostProcessInstance, 1.0f);
	return true;
}

void AHronoCharacter::SetMirroredViewEnabled(bool bEnabled)
{
	SetMirrorAmount(bEnabled ? 1.0f : 0.0f);
}

void AHronoCharacter::SetMirrorAmount(float NewMirrorAmount)
{
	MirrorAmount = FMath::Clamp(NewMirrorAmount, 0.0f, 1.0f);
	bMirrorViewActive = false;

	if (!IsLocallyControlled())
	{
		return;
	}

	if (!EnsureMirrorPostProcessInstance())
	{
		return;
	}

	MirrorPostProcessInstance->SetScalarParameterValue(MirrorParameterName, MirrorAmount);
	bMirrorViewActive = MirrorAmount >= 0.5f;

	UE_LOG(LogTemp, Log, TEXT("[MirrorView] Character=%s Amount=%.2f InputScale=%.0f"),
		*GetNameSafe(this),
		MirrorAmount,
		GetMirroredHorizontalInputScale());
}

void AHronoCharacter::ApplyMirrorFromCharacterTimeline()
{
	SetMirroredViewEnabled(CharacterTimeline == EItemTimeline::Past);
}

float AHronoCharacter::GetMirroredHorizontalInputScale() const
{
	return IsMirroredViewEnabled() ? -1.0f : 1.0f;
}


void AHronoCharacter::ServerSetSprinting_Implementation(bool bNewSprint)
{
	SetSprintingState(bNewSprint);
}

void AHronoCharacter::ApplyTimelineCollision()
{
	// Blueprint collision presets can override the constructor response. Reapply
	// this at runtime and after every timeline change so Base_Item actors never
	// physically block the character capsule.
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	USkeletalMeshComponent* CharacterMesh = GetMesh();

	Capsule->SetCollisionResponseToChannel(COLLISION_CHANNEL_ITEM, ECR_Ignore);

	if (CharacterTimeline == EItemTimeline::Past)
	{
		Capsule->SetCollisionObjectType(COLLISION_CHANNEL_PAWN_PAST);
		Capsule->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, ECR_Block);
		Capsule->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, ECR_Ignore);
		Capsule->SetCollisionResponseToChannel(COLLISION_CHANNEL_DOOR_PAST, ECR_Block);
		Capsule->SetCollisionResponseToChannel(COLLISION_CHANNEL_DOOR_FUTURE, ECR_Ignore);

		CharacterMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, ECR_Block);
		CharacterMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, ECR_Ignore);
	}
	else
	{
		Capsule->SetCollisionObjectType(COLLISION_CHANNEL_PAWN_FUTURE);
		Capsule->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, ECR_Block);
		Capsule->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, ECR_Ignore);
		Capsule->SetCollisionResponseToChannel(COLLISION_CHANNEL_DOOR_FUTURE, ECR_Block);
		Capsule->SetCollisionResponseToChannel(COLLISION_CHANNEL_DOOR_PAST, ECR_Ignore);

		CharacterMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_FUTURE, ECR_Block);
		CharacterMesh->SetCollisionResponseToChannel(COLLISION_CHANNEL_PAWN_PAST, ECR_Ignore);
	}
}



void AHronoCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AHronoCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AHronoCharacter::DoJumpEnd);
		//Drag
		EnhancedInputComponent->BindAction(
			DragAction,
			ETriggerEvent::Started,
			this,
			&AHronoCharacter::DoDrag
		);

		EnhancedInputComponent->BindAction(
			DragAction,
			ETriggerEvent::Completed,
			this,
			&AHronoCharacter::DoUnDrag
		);

		EnhancedInputComponent->BindAction(
			DragAction,
			ETriggerEvent::Canceled,
			this,
			&AHronoCharacter::DoUnDrag
		);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AHronoCharacter::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHronoCharacter::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AHronoCharacter::MouseLookInput);

		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AHronoCharacter::DoInteract);

		// Sprinting
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AHronoCharacter::DoStartSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AHronoCharacter::DoEndSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Canceled, this, &AHronoCharacter::DoEndSprint);

		// Crouch
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AHronoCharacter::DoCrouchStart);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AHronoCharacter::DoCrouchEnd);

		//StandAction;
		EnhancedInputComponent->BindAction(StandAction, ETriggerEvent::Completed, this, &AHronoCharacter::DoStandUp);

		// Drop held item
		if (DropAction)
		{
			EnhancedInputComponent->BindAction(DropAction, ETriggerEvent::Triggered, this, &AHronoCharacter::DoDrop);
		}
	}
	else
	{
		UE_LOG(LogHrono, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}

	// Keep this as a direct key binding so the journal is always available and
	// does not require modifying every existing Enhanced Input Mapping Context.
	FInputKeyBinding& TutorialBinding = PlayerInputComponent->BindKey(
		EKeys::Tab, IE_Pressed, this, &AHronoCharacter::ToggleTutorialMenu);
	TutorialBinding.bExecuteWhenPaused = true;
}

void AHronoCharacter::DoStandUp()
{
	if (HasAuthority())
	{
		StandUp();
	}
	else
	{
		ServerStandUp();
	}
}

void AHronoCharacter::ServerStandUp_Implementation()
{
	StandUp();
}

void AHronoCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	UE_LOG(LogTemp, Warning, TEXT("OnStartCrouch"));
}

#include "Kismet/GameplayStatics.h"
void AHronoCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Blueprint-authored and dynamically inherited component defaults can
	// override constructor values. Enforce the no-player-shadows rule on both
	// server and clients after every character component has been created.
	TInlineComponentArray<UPrimitiveComponent*> PlayerComponents(this);
	for (UPrimitiveComponent* Component : PlayerComponents)
	{
		DisablePlayerShadows(Component);
	}

	// Server assigns timeline: its own local player = Future, remote players = Past.
	// On a dedicated server (no local player), all characters default to Past
	// unless overridden by GameMode logic.
	if (HasAuthority())
	{
		MaxStamina = FMath::Max(MaxStamina, 1.0f);
		CurrentStamina = MaxStamina;
		bSprinting = false;
		bRecovering = false;
		StaminaRegenerationDelayRemaining = 0.0f;
		OnRep_StaminaData();
		ApplySprintMovementSpeed();
		ForceNetUpdate();

		if (IsLocallyControlled())
		{
			CharacterTimeline = EItemTimeline::Future;
			
		}
		else
		{
			CharacterTimeline = EItemTimeline::Past;
		}
	}

	if (!IsLocallyControlled())
	{
		SpotLight->DestroyComponent();
		SpotLight = nullptr;
	}

	ApplyTimelineCollision();
	RefreshHeldItemsInteractionPoint();
	// Past always renders as the mirror world, including the initial spawn.
	bTimelineMirrorRequested = CharacterTimeline == EItemTimeline::Past;
	OnRep_TimelineMirrorRequested();
	RefreshTimelineVisibilityForLocalPlayer();
	ApplySprintMovementSpeed();
	LoadLocalPlayerSettings();
	EnsureTutorialWidget();

	// DEBUG: Print timeline
	const char* TimelineStr = (CharacterTimeline == EItemTimeline::Future) ? "FUTURE" : "PAST";
	UE_LOG(LogTemp, Warning, TEXT("Character Timeline: %s"), ANSI_TO_TCHAR(TimelineStr));
}

void AHronoCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	// Possession can become local after BeginPlay. Reapply the local-only camera
	// material here so an initially Past client is always mirrored.
	ApplyMirrorFromCharacterTimeline();
	RefreshHeldItemsInteractionPoint();
	RefreshTimelineVisibilityForLocalPlayer();
	LoadLocalPlayerSettings();
	EnsureTutorialWidget();
	HandleHeldItemTutorial(CurrentHeldItem);
	UpdateRitualChairGuidance(0.0f, true);
}

void AHronoCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (const TWeakObjectPtr<AHidingWardrobe>& Source : WardrobeSafetySources)
		if (Source.IsValid()) Source->OnDestroyed.RemoveDynamic(this, &AHronoCharacter::OnWardrobeSafetySourceDestroyed);
	WardrobeSafetySources.Empty();
	CancelDeathTimelineTransition();
	if (ABase_Item* HighlightedItem = HighlightedInteractionItem.Get())
	{
		HighlightedItem->SetInteractionHighlighted(false);
		HighlightedInteractionItem.Reset();
	}
	ClearRitualChairGuidance();

	if (IsValid(FpsCounterWidget))
	{
		FpsCounterWidget->RemoveFromParent();
		FpsCounterWidget = nullptr;
	}

	if (IsValid(TutorialWidget))
	{
		TutorialWidget->CloseMenu();
		TutorialWidget->RemoveFromParent();
		TutorialWidget = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void AHronoCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// UCameraComponent applies bUsePawnControlRotation while producing the local
	// player's camera view. That path is not evaluated for a remote pawn on the
	// server (or on another client), even though CharacterMovement sends control
	// rotation to the server and ACharacter replicates its remote view pitch.
	// Explicitly consume those rotations for non-local copies so camera-attached
	// InteractionPoint components, and in turn every held item, follow the owner.
	if (!IsLocallyControlled() && IsValid(FirstPersonCameraComponent))
	{
		const FRotator ReplicatedViewRotation = HasAuthority()
			? GetControlRotation()
			: GetBaseAimRotation();
		FirstPersonCameraComponent->SetWorldRotation(ReplicatedViewRotation);
	}

	if (HasAuthority())
	{
		UpdateStamina(DeltaTime);
		UpdateMovementAudio(DeltaTime);
	}

	if (IsLocallyControlled())
	{
		UpdateInteractionHighlight();
		UpdateRitualChairGuidance(DeltaTime);
		UpdateTutorialPaintingLook(DeltaTime);
	}
}

void AHronoCharacter::UpdateTutorialPaintingLook(float DeltaTime)
{
	if (TutorialStep != EHronoTutorialStep::FindPaintingAnomaly
		|| ResolveTutorialItem(CurrentHeldItem) != EHronoTutorialItem::Monocle
		|| !IsValid(FirstPersonCameraComponent))
	{
		TutorialObservedPainting.Reset();
		TutorialPaintingLookTime = 0.0f;
		return;
	}

	const FVector TraceStart = FirstPersonCameraComponent->GetComponentLocation();
	const FVector TraceEnd = TraceStart
		+ FirstPersonCameraComponent->GetForwardVector() * TutorialPaintingLookDistance;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(TutorialMonoclePainting), false, this);
	QueryParams.AddIgnoredActor(CurrentHeldItem);
	FHitResult Hit;
	GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

	APaintItem* Painting = Cast<APaintItem>(Hit.GetActor());
	const bool bValidAnomaly = IsValid(Painting)
		&& Painting->GetPaintAnomalyType() != EPaintAnomalyType::None
		&& (Painting->ItemTimeline == EItemTimeline::Both
			|| Painting->ItemTimeline == CharacterTimeline);
	if (!bValidAnomaly)
	{
		TutorialObservedPainting.Reset();
		TutorialPaintingLookTime = 0.0f;
		return;
	}

	if (TutorialObservedPainting.Get() != Painting)
	{
		TutorialObservedPainting = Painting;
		TutorialPaintingLookTime = 0.0f;
	}
	TutorialPaintingLookTime += DeltaTime;
	if (TutorialPaintingLookTime >= TutorialPaintingLookDuration)
	{
		TutorialPaintingLookTime = 0.0f;
		ServerNotifyTutorialPaintingFound(Painting);
	}
}

void AHronoCharacter::ServerNotifyTutorialPaintingFound_Implementation(APaintItem* Painting)
{
	if (!IsValid(Painting)
		|| Painting->GetPaintAnomalyType() == EPaintAnomalyType::None
		|| ResolveTutorialItem(CurrentHeldItem) != EHronoTutorialItem::Monocle
		|| (Painting->ItemTimeline != EItemTimeline::Both
			&& Painting->ItemTimeline != CharacterTimeline)
		|| FVector::DistSquared(Painting->GetActorLocation(), GetActorLocation())
			> FMath::Square(TutorialPaintingLookDistance + 250.0f))
	{
		return;
	}

	CompleteTutorialStep(EHronoTutorialStep::FindPaintingAnomaly);
}

void AHronoCharacter::NotifyTutorialDosimeterFastBeep()
{
	if (TutorialStep != EHronoTutorialStep::DetectDosimeterAnomaly)
	{
		return;
	}

	if (HasAuthority())
	{
		ServerNotifyTutorialDosimeterFastBeep_Implementation();
	}
	else
	{
		ServerNotifyTutorialDosimeterFastBeep();
	}
}

void AHronoCharacter::ServerNotifyTutorialDosimeterFastBeep_Implementation()
{
	if (!IsValid(Cast<ADozimetr>(CurrentHeldItem)))
	{
		return;
	}

	CompleteTutorialStep(EHronoTutorialStep::DetectDosimeterAnomaly);
}

void AHronoCharacter::UpdateRitualChairGuidance(float DeltaTime, bool bForceRefresh)
{
	if (!IsLocallyControlled() || !GetWorld())
	{
		return;
	}

	constexpr float RefreshInterval = 0.2f;
	RitualChairGuidanceRefreshAccumulator += DeltaTime;
	if (!bForceRefresh && RitualChairGuidanceRefreshAccumulator < RefreshInterval)
	{
		return;
	}
	RitualChairGuidanceRefreshAccumulator = 0.0f;

	TSet<TWeakObjectPtr<AChair>> NewHighlightedChairs;
	for (TActorIterator<AChair> It(GetWorld()); It; ++It)
	{
		AChair* Chair = *It;
		if (!IsValid(Chair))
		{
			continue;
		}

		const bool bSameTimeline = Chair->ItemTimeline == EItemTimeline::Both
			|| Chair->ItemTimeline == CharacterTimeline;
		const bool bFreeChair = !Chair->bIsSit;
		const bool bReservedForThisPlayer = !bIsSitting && Chair->GetSitter() == this;
		const bool bCanSitHere = !IsValid(CurrentChair)
			|| (bIsAtRitualPoint && Chair == ReservedRitualChair
				&& CurrentChair == Chair && Chair->GetSitter() == this);
		const bool bShouldHighlight = Chair->IsRitualGuidanceUnlocked()
			&& !bIsSitting
			&& bCanSitHere
			&& bSameTimeline
			&& !Chair->IsHidden()
			&& (bFreeChair || bReservedForThisPlayer);

		Chair->SetInteractionContextHighlighted(bShouldHighlight);
		if (bShouldHighlight)
		{
			NewHighlightedChairs.Add(Chair);
		}
	}

	for (const TWeakObjectPtr<AChair>& PreviousChair : RitualGuidanceHighlightedChairs)
	{
		if (AChair* Chair = PreviousChair.Get();
			IsValid(Chair) && !NewHighlightedChairs.Contains(PreviousChair))
		{
			Chair->SetInteractionContextHighlighted(false);
		}
	}
	RitualGuidanceHighlightedChairs = MoveTemp(NewHighlightedChairs);
}

void AHronoCharacter::RefreshRitualChairGuidanceNow()
{
	UpdateRitualChairGuidance(0.0f, true);
}

void AHronoCharacter::ClearRitualChairGuidance()
{
	for (const TWeakObjectPtr<AChair>& HighlightedChair : RitualGuidanceHighlightedChairs)
	{
		if (AChair* Chair = HighlightedChair.Get())
		{
			Chair->SetInteractionContextHighlighted(false);
		}
	}
	RitualGuidanceHighlightedChairs.Reset();
}

FHitResult AHronoCharacter::GetInteractionPresentationHit()
{
	UCameraComponent* Camera = GetFirstPersonCameraComponent();
	UWorld* World = GetWorld();
	if (!IsLocallyControlled() || !IsValid(Camera) || !IsValid(World)) return FHitResult();
	const FVector Start = Camera->GetComponentLocation();
	const FVector End = Start + Camera->GetForwardVector() * InteractTraceDistance;
	if (PresentationTraceFrame == GFrameCounter && PresentationTraceStart.Equals(Start)
		&& PresentationTraceEnd.Equals(End) && PresentationTraceTimeline == CharacterTimeline)
	{
		return CachedPresentationHit;
	}
	PresentationTraceFrame = GFrameCounter;
	PresentationTraceStart = Start;
	PresentationTraceEnd = End;
	PresentationTraceTimeline = CharacterTimeline;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(InteractionPresentation), false, this);
	const ECollisionChannel TraceChannel = CharacterTimeline == EItemTimeline::Future
		? ECC_GameTraceChannel3 : ECC_GameTraceChannel2;
	World->LineTraceSingleByChannel(CachedPresentationHit, Start, End, TraceChannel, Params);
	ABase_Item* PrimaryItem = Cast<ABase_Item>(CachedPresentationHit.GetActor());
	if (!IsValid(PrimaryItem))
	{
		FHitResult VisibilityHit;
		if (World->LineTraceSingleByChannel(VisibilityHit, Start, End, ECC_Visibility, Params)
			&& (!CachedPresentationHit.bBlockingHit || VisibilityHit.Distance <= CachedPresentationHit.Distance + 1.0f))
		{
			CachedPresentationHit = VisibilityHit;
		}
	}
	return CachedPresentationHit;
}

bool AHronoCharacter::IsFocusedItemUsable()
{
	ABase_Item* Item = Cast<ABase_Item>(GetInteractionPresentationHit().GetActor());
	return IsValid(Item) && Item->UsableValid
		&& Item->CanBePickedUp()
		&& (Item->ItemTimeline == EItemTimeline::Both || Item->ItemTimeline == CharacterTimeline);
}

void AHronoCharacter::UpdateInteractionHighlight()
{
	ABase_Item* NewHighlightedItem = nullptr;
	AActor* HitActor = GetInteractionPresentationHit().GetActor();
	for (int32 Depth = 0; IsValid(HitActor) && Depth < 8; ++Depth)
	{
		if (ABase_Item* Item = Cast<ABase_Item>(HitActor))
		{
			NewHighlightedItem = Item->CanHighlightFor(this) ? Item : nullptr;
			break;
		}
		HitActor = HitActor->GetAttachParentActor();
	}

	ABase_Item* PreviousItem = HighlightedInteractionItem.Get();
	if (PreviousItem == NewHighlightedItem)
	{
		if (IsValid(NewHighlightedItem))
		{
			// Revalidate every frame so Blueprint mesh/material changes cannot leave a
			// focused item visually unhighlighted until the player looks away.
			NewHighlightedItem->SetInteractionHighlighted(true);
		}
		return;
	}

	if (IsValid(PreviousItem))
	{
		PreviousItem->SetInteractionHighlighted(false);
	}
	if (IsValid(NewHighlightedItem))
	{
		NewHighlightedItem->SetInteractionHighlighted(true);
	}
	HighlightedInteractionItem = NewHighlightedItem;
}

FHitResult AHronoCharacter::PerformInteractTrace(bool bIsDrag)
{
	FHitResult HitResult;

	UCameraComponent* Camera = GetFirstPersonCameraComponent();
	if (!Camera)
	{
		UE_LOG(LogTemp, Error, TEXT("PerformInteractTrace failed: Camera is nullptr"));
		return HitResult;
	}

	FVector Start = Camera->GetComponentLocation();
	FVector End = Start + Camera->GetForwardVector() * InteractTraceDistance;

	/*DrawDebugLine(
		GetWorld(),
		Start,
		End,
		FColor::Green,
		false,
		0.2f,
		0,
		2.0f
	);*/

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit;
	if (CharacterTimeline == EItemTimeline::Future) {
		bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			Start,
			End,
			ECC_GameTraceChannel3,
			Params
		);
	}
	else {
		bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			Start,
			End,
			ECC_GameTraceChannel2,
			Params
		);
	}
	if (bHit)
	{
		if (!bIsDrag)
		{
			OnEnyInteractTrace(HitResult);
		}

		/*DrawDebugSphere(
			GetWorld(),
			HitResult.ImpactPoint,
			12.0f,
			12,
			FColor::Red,
			false,
			2.0f
		);*/

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("LineTrace HIT: %s"),
			HitResult.GetActor() ? *HitResult.GetActor()->GetName() : TEXT("Unknown")
		);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("LineTrace missed"));
	}
	auto Door = Cast<ADrag_Item>(HitResult.GetActor());

	if (Door)
	{
		const char* DoorTimelineStr = (Door->ItemTimeline == EItemTimeline::Future) ? "FUTURE" : "PAST";
		const char* CharTimelineStr = (CharacterTimeline == EItemTimeline::Future) ? "FUTURE" : "PAST";

		UE_LOG(LogTemp, Warning, TEXT("Door Timeline: %s, Character Timeline: %s"),
			ANSI_TO_TCHAR(DoorTimelineStr), ANSI_TO_TCHAR(CharTimelineStr));

		if (Door->ItemTimeline != EItemTimeline::Both
			&& Door->ItemTimeline != CharacterTimeline)
		{
			UE_LOG(LogTemp, Warning, TEXT("Timeline mismatch! Clearing HitResult"));
			HitResult = FHitResult();
		}
	}

	return HitResult;
}
#include "Items/Drag_Item.h"
void AHronoCharacter::HandleInteraction(const FHitResult& HitResult)
{
	AActor* HitActor = HitResult.GetActor();
	if (!HitActor)
	{
		return;
	}

	if (auto Item = Cast<ABase_Item>(HitActor))
	{
		if (!Item->CanBePickedUp())
		{
			return;
		}
		UE_LOG(LogTemp, Warning, TEXT("Valid item found: %s"), *Item->GetName());
		auto Draggable = Cast<ADrag_Item>(Item);
		if (Draggable) return;


		if (HasAuthority())
		{
			PickupItem(Item);
		}
		else
		{
			ServerPickupItem(Item);
		}
	}
}


void AHronoCharacter::DoInteract()
{
	if (!IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("DoInteract aborted: Character is not locally controlled"));
		return;
	}

	UE_LOG(LogTemp, Warning,
		TEXT("[InteractionDebug] E PRESSED Player=%s Authority=%d Timeline=%s CurrentItem=%s"),
		*GetName(), HasAuthority(),
		*StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(CharacterTimeline)),
		*GetNameSafe(GetHeldItem()));

	FHitResult HitResult = PerformInteractTrace(false);

	if (HitResult.bBlockingHit)
	{
		const FString HitDebug = FString::Printf(
			TEXT("E HIT: %s | Component: %s | Interface: %s"),
			*GetNameSafe(HitResult.GetActor()),
			*GetNameSafe(HitResult.GetComponent()),
			HitResult.GetActor() && HitResult.GetActor()->Implements<UEnviroment_Interface>()
				? TEXT("YES") : TEXT("NO"));
		UE_LOG(LogTemp, Warning, TEXT("[InteractionDebug] %s"), *HitDebug);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, HitDebug);
		}
		HandleInteraction(HitResult);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[InteractionDebug] E TRACE MISSED Player=%s"), *GetName());
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("E TRACE MISSED"));
		}
	}
}

void AHronoCharacter::ServerPickupItem_Implementation(ABase_Item* Item)
{
	UE_LOG(LogTemp, Warning, TEXT("[Item] ServerPickupItem on %s"), *GetName());

	PickupItem(Item);
}

bool AHronoCharacter::CanInteractWithActorOnServer(const AActor* Target,
	const UPrimitiveComponent* TargetComponent, float ExtraDistance) const
{
	if (!HasAuthority() || IsActorBeingDestroyed() || !IsValid(Target) || Target == this
		|| Target->IsActorBeingDestroyed() || Target->GetWorld() != GetWorld()
		|| !Target->GetActorEnableCollision()
		|| (CharacterTimeline != EItemTimeline::Past && CharacterTimeline != EItemTimeline::Future)
		|| !FMath::IsFinite(InteractTraceDistance) || InteractTraceDistance <= 0.0f
		|| !FMath::IsFinite(ExtraDistance) || ExtraDistance < 0.0f
		|| (TargetComponent && TargetComponent->GetOwner() != Target))
	{
		return false;
	}
	if (const ABase_Item* Item = Cast<ABase_Item>(Target);
		Item && Item->ItemTimeline != EItemTimeline::Both && Item->ItemTimeline != CharacterTimeline)
	{
		return false;
	}

	const FVector ViewLocation = FirstPersonCameraComponent
		? FirstPersonCameraComponent->GetComponentLocation() : GetPawnViewLocation();
	const ECollisionChannel Channel = CharacterTimeline == EItemTimeline::Past
		? COLLISION_CHANNEL_PAWN_PAST : COLLISION_CHANNEL_PAWN_FUTURE;
	const float MaximumDistance = InteractTraceDistance + ExtraDistance;
	if (ViewLocation.ContainsNaN() || !FMath::IsFinite(MaximumDistance)) return false;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(ServerInteraction), false, this);
	if (IsValid(CurrentHeldItem) && CurrentHeldItem != Target)
	{
		Params.AddIgnoredActor(CurrentHeldItem);
	}
	TInlineComponentArray<UPrimitiveComponent*> Primitives;
	Target->GetComponents(Primitives);
	for (const UPrimitiveComponent* Primitive : Primitives)
	{
		if (!IsValid(Primitive) || !Primitive->IsRegistered()
			|| (TargetComponent && Primitive != TargetComponent)
			|| !Primitive->IsQueryCollisionEnabled()
			|| Primitive->GetCollisionResponseToChannel(Channel) != ECR_Block)
		{
			continue;
		}
		FVector Point;
		if (Primitive->GetClosestPointOnCollision(ViewLocation, Point) < 0.0f)
		{
			// Complex-only meshes may not support closest-point queries.
			Point = Primitive->Bounds.GetBox().GetClosestPointTo(ViewLocation);
		}
		if (Point.ContainsNaN() || FVector::DistSquared(ViewLocation, Point) > FMath::Square(MaximumDistance))
		{
			continue;
		}
		FHitResult Obstruction;
		if (!GetWorld()->LineTraceSingleByChannel(Obstruction, ViewLocation, Point, Channel, Params)
			|| Obstruction.GetActor() == Target)
		{
			return true;
		}
	}
	return false;
}

void AHronoCharacter::OnRep_CurrentChair(AChair* PreviousChair)
{
	UpdateChairState(PreviousChair);
}

void AHronoCharacter::UpdateChairState(AChair* PreviousChair)
{
	// Run on the authority (so the server-owned position is updated and replicated)
	// and on the owning client (so the local player reacts immediately). Simulated
	// proxies are skipped because their transform follows replicated movement.
	// Doing the teleport on the server too keeps positions in sync and prevents the
	// CharacterMovement correction that caused the double teleport on stand up.
	if (!HasAuthority() && !IsLocallyControlled())
		return;

	if (CurrentChair)
	{
		HandleSitStarted(CurrentChair);
	}
	else if (PreviousChair)
	{
		// CurrentChair has already been cleared, so the chair we left is passed in
		// as the previous value of the replicated property.
		HandleSitEnded(PreviousChair);
	}
}

void AHronoCharacter::HandleSitStarted(AChair* Chair)
{
	if (!Chair)
		return;

	// Freeze the character in place while sitting.
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}

	// Snap to the chair's sit point.
	if (USceneComponent* SitPoint = Chair->GetSitPoint())
	{
		SetActorLocation(SitPoint->GetComponentLocation(), false, nullptr, ETeleportType::TeleportPhysics);
	}

	// Optional cosmetic hook (animation, sound). The gameplay logic now lives in C++.
	OnSitStarted(Chair);
	Chair->NotifyCharacterSat(this);
}

void AHronoCharacter::HandleSitEnded(AChair* Chair)
{
	// Snap to the chair's stand-up point before restoring movement.
	if (Chair)
	{
		if (USceneComponent* StandUpPoint = Chair->GetStandUpPoint())
		{
			SetActorLocation(StandUpPoint->GetComponentLocation(), false, nullptr, ETeleportType::TeleportPhysics);
		}
	}

	// Restore normal walking movement.
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Walking);
	}

	// Optional cosmetic hook (animation, sound). The gameplay logic now lives in C++.
	OnSitEnded();
}

void AHronoCharacter::StandUp()
{
	if (!HasAuthority())
		return;
	
	AChair* PreviousChair = CurrentChair;

	if (CurrentChair)
	{
		if (CurrentChair->IsRitualStarted == true) {
			return;
		}
		CurrentChair->bIsSit = false;
		CurrentChair->SetSitter(nullptr);
	}



	CurrentChair = nullptr;
	bIsSitting = false;
	ReservedRitualChair = nullptr;
	bIsAtRitualPoint = false;
	RitualDestinationActor = nullptr;
	bLocalRitualPositionApplied = false;

	// RepNotify is not called on the authority, so invoke it manually to keep the
	// server (listen-server host) in sync with clients.
	OnRep_CurrentChair(PreviousChair);
}

bool AHronoCharacter::SitOnChair(AChair* Chair)
{
	if (!HasAuthority())
		return false;

	if (!IsValid(Chair) || !TableRitualGate::CanUseChair(*Chair))
		return false;

	// The victim still owns this chair while walking back from the bottle ritual.
	// CurrentChair and the chair's occupied flag deliberately remain set during
	// the teleport, so the ordinary free-chair path cannot handle the return.
	if (bIsAtRitualPoint && !bIsSitting
		&& CurrentChair == Chair && ReservedRitualChair == Chair
		&& Chair->GetSitter() == this)
	{
		return ForceSitOnChair(Chair);
	}

	if (bIsSitting || CurrentChair)
		return false;

	if (Chair->bIsSit)
		return false;

	AChair* PreviousChair = CurrentChair;

	CurrentChair = Chair;
	bIsSitting = true;
	ReservedRitualChair = Chair;
	bIsAtRitualPoint = false;
	RitualDestinationActor = nullptr;
	bLocalRitualPositionApplied = false;

	Chair->bIsSit = true;
	Chair->SetSitter(this);

	// RepNotify is not called on the authority, so invoke it manually to keep the
	// server (listen-server host) in sync with clients.
	OnRep_CurrentChair(PreviousChair);
	Chair->ForceNetUpdate();
	ForceNetUpdate();
	CompleteSeatedPlayersTutorial(GetWorld());
	return true;
}

bool AHronoCharacter::ForceSitOnChair(AChair* Chair)
{
	if (!HasAuthority() || !IsValid(Chair) || !TableRitualGate::CanUseChair(*Chair))
	{
		return false;
	}

	// This operation is intentionally forceful: the selected ritual character must
	// own the target chair even if stale state or another sitter currently occupies it.
	if (AHronoCharacter* PreviousSitter = Chair->GetSitter();
		IsValid(PreviousSitter) && PreviousSitter != this)
	{
		AChair* PreviousSitterChair = PreviousSitter->CurrentChair;
		if (IsValid(PreviousSitterChair))
		{
			PreviousSitterChair->bIsSit = false;
			PreviousSitterChair->SetSitter(nullptr);
			PreviousSitterChair->ForceNetUpdate();
		}

		PreviousSitter->CurrentChair = nullptr;
		PreviousSitter->bIsSitting = false;
		PreviousSitter->OnRep_CurrentChair(PreviousSitterChair);
		PreviousSitter->ForceNetUpdate();
	}

	AChair* PreviousChair = CurrentChair;
	if (IsValid(PreviousChair) && PreviousChair != Chair)
	{
		PreviousChair->bIsSit = false;
		if (PreviousChair->GetSitter() == this)
		{
			PreviousChair->SetSitter(nullptr);
		}
		PreviousChair->ForceNetUpdate();
	}

	CurrentChair = Chair;
	bIsSitting = true;
	ReservedRitualChair = Chair;
	bIsAtRitualPoint = false;
	RitualDestinationActor = nullptr;
	bLocalRitualPositionApplied = false;
	Chair->bIsSit = true;
	Chair->SetSitter(this);

	// RepNotify does not execute automatically on the authority. This performs the
	// teleport and invokes the existing Blueprint OnSitStarted hook immediately.
	OnRep_CurrentChair(PreviousChair);
	Chair->ForceNetUpdate();
	ForceNetUpdate();
	CompleteSeatedPlayersTutorial(GetWorld());
	return true;
}

bool AHronoCharacter::MoveFromChairToRitualPoint(AActor* RitualPoint)
{
	if (!HasAuthority() || !IsValid(RitualPoint))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[ChairRitual] Move failed for %s: authority=%d RitualPoint=%s"),
			*GetName(),
			HasAuthority(),
			*GetNameSafe(RitualPoint));
		return false;
	}

	AChair* ChairToReserve = IsValid(CurrentChair)
		? CurrentChair
		: ReservedRitualChair.Get();
	if (!IsValid(ChairToReserve) || ChairToReserve->GetSitter() != this)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[ChairRitual] Move failed for %s: character is not registered on a chair"),
			*GetName());
		return false;
	}

	ReservedRitualChair = ChairToReserve;
	bIsSitting = false;
	bIsAtRitualPoint = true;
	RitualDestinationActor = RitualPoint;
	bLocalRitualPositionApplied = true;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->SetMovementMode(MOVE_Walking);
	}

	// Leave the sitting animation/state without using HandleSitEnded, because that
	// helper would first teleport the pawn to the chair's StandUpPoint.
	OnSitEnded();

	SetActorLocationAndRotation(
		RitualPoint->GetActorLocation(),
		RitualPoint->GetActorRotation(),
		false,
		nullptr,
		ETeleportType::TeleportPhysics);

	UE_LOG(LogTemp, Log,
		TEXT("[ChairRitual] Moved %s from reserved chair %s to %s without possession"),
		*GetName(),
		*GetNameSafe(ReservedRitualChair),
		*GetNameSafe(RitualPoint));

	ForceNetUpdate();
	return true;
}

bool AHronoCharacter::ReturnToReservedRitualChair()
{
	if (!HasAuthority() || !IsValid(ReservedRitualChair))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[ChairRitual] Return failed for %s: authority=%d ReservedChair=%s"),
			*GetName(),
			HasAuthority(),
			*GetNameSafe(ReservedRitualChair));
		return false;
	}

	return ForceSitOnChair(ReservedRitualChair);
}

void AHronoCharacter::OnRep_RitualPositionState()
{
	if (!HasAuthority() && !IsLocallyControlled())
	{
		return;
	}

	if (bIsAtRitualPoint)
	{
		if (IsValid(RitualDestinationActor))
		{
			if (UCharacterMovementComponent* Movement = GetCharacterMovement())
			{
				Movement->StopMovementImmediately();
				Movement->SetMovementMode(MOVE_Walking);
			}

			if (!bLocalRitualPositionApplied)
			{
				OnSitEnded();
			}
			bLocalRitualPositionApplied = true;

			SetActorLocationAndRotation(
				RitualDestinationActor->GetActorLocation(),
				RitualDestinationActor->GetActorRotation(),
				false,
				nullptr,
				ETeleportType::TeleportPhysics);
		}
		return;
	}

	if (bLocalRitualPositionApplied
		&& IsValid(ReservedRitualChair)
		&& CurrentChair == ReservedRitualChair)
	{
		bLocalRitualPositionApplied = false;
		HandleSitStarted(ReservedRitualChair);
	}
}

void AHronoCharacter::DoDrop()
{
	UE_LOG(LogTemp, Log, TEXT("[DropLog] 1. DoDrop Input Triggered. IsLocallyControlled: %s, HasAuthority: %s"),
		IsLocallyControlled() ? TEXT("True") : TEXT("False"),
		HasAuthority() ? TEXT("True") : TEXT("False"));

	if (!IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("[DropLog] DoDrop aborted: Character is not locally controlled"));
		return;
	}

	if (HasAuthority())
	{
		UE_LOG(LogTemp, Log, TEXT("[DropLog] Local Player is Server. Calling DropCurrentItem directly."));
		DropCurrentItem();
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[DropLog] Local Player is Client. Sending ServerDropCurrentItem RPC..."));
		ServerDropCurrentItem();
	}
}


void AHronoCharacter::HandleDrag(const FHitResult& HitResult)
{
	AActor* HitActor = HitResult.GetActor();
	UE_LOG(LogTemp, Warning, TEXT("HandleDrag: Hit actor = %s"), HitActor ? *HitActor->GetName() : TEXT("None"));

	auto Item = Cast<ADrag_Item>(HitResult.GetActor());
	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("HandleDrag FAILED: Not an ADrag_Item!"));
		return;
	}

	if (Item->bUseAutomaticOpenClose)
	{
		UE_LOG(LogTemp, Log,
			TEXT("HandleDrag ignored for %s because automatic E interaction is enabled"),
			*GetNameSafe(Item));
		return;
	}

	// Multi-door actors own more than one UDrag_Component. Select the one whose
	// interaction primitive was actually hit instead of always taking the first.
	auto DragComponent = Item->FindDragComponentForHit(HitResult.GetComponent());
	if (!DragComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("HandleDrag FAILED: No DragComponent found!"));
		return;
	}

	if (Item->IsLockedByTrigger())
	{
		UE_LOG(LogTemp, Log,
			TEXT("[DoorTriggerLock] %s cannot drag %s while its trigger lock is active"),
			*GetNameSafe(this),
			*GetNameSafe(Item));
		return;
	}

	if (Item->bNeedKeyActor)
	{
		if (!Item->CanUnlockWithItem(CurrentHeldItem))
		{
			UE_LOG(LogTemp, Log,
				TEXT("[DoorLock] %s requires %s; held item %s does not match"),
				*GetNameSafe(Item),
				*Item->GetRequiredKeyTag().ToString(),
				*GetNameSafe(CurrentHeldItem));
			return;
		}

		if (HasAuthority())
		{
			ServerUnlockWithHeldKey_Implementation(Item);
		}
		else
		{
			ServerUnlockWithHeldKey(Item);
		}
	}

	if (Item->IsDoorBlockedForTimeline(CharacterTimeline))
	{
		UE_LOG(LogTemp, Log,
			TEXT("[DoorBarricade] %s cannot drag %s in timeline %s"),
			*GetNameSafe(this),
			*GetNameSafe(Item),
			*StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(CharacterTimeline)));
		return;
	}

	CurrentDraggedComponent = DragComponent;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("HandleDrag FAILED: No PlayerController!"));
		return;
	}

	DragComponent->StartDrag(PC, HitResult.ImpactPoint);

	if (DragComponent->bIsCupBoard)
	{
		UE_LOG(LogTemp, Log, TEXT("Started dragging cupboard door"));
	}
	else if (DragComponent->bIsShelf)
	{
		UE_LOG(LogTemp, Log, TEXT("Started dragging shelf"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Started dragging door"));
	}
}

void AHronoCharacter::DoDrag()
{
	UE_LOG(LogTemp, Log, TEXT("DoDrag()"));

	FHitResult HitResult = PerformInteractTrace(true);

	if (HitResult.bBlockingHit)
	{
		HandleDrag(HitResult);
	}
}

void AHronoCharacter::DoUnDrag()
{
	UE_LOG(LogTemp, Log, TEXT("DoUnDrag()"));
	if (!CurrentDraggedComponent) return;

	CurrentDraggedComponent->StopDrag();
	CurrentDraggedComponent = nullptr;
}

// Resolve only authored drag panels. A component name is a selector, never permission.
UDrag_Component* AHronoCharacter::GetAllowedDragPanel(ADrag_Item* Item, FName Name, bool bLinear) const
{
	if (!IsValid(Item) || Item->bUseAutomaticOpenClose || Item->bNeedKeyActor
		|| Item->IsLockedByTrigger() || Item->IsDoorBlockedForTimeline(CharacterTimeline)) return nullptr;
	const USceneComponent* Movement = bLinear
		? Item->FindShelfMovementComponent(Name) : Item->FindDoorMovementComponent(Name);
	if (!Movement) return nullptr;
	TInlineComponentArray<UDrag_Component*> Panels;
	Item->GetComponents(Panels);
	for (UDrag_Component* Panel : Panels)
	{
		if (Panel->GetTargetMovementComponent() == Movement
			&& (Panel->bIsShelf || Panel->bIsCupBoard) == bLinear
			&& Panel->GetInteractionPrimitive()
			&& CanInteractWithActorOnServer(Item, Panel->GetInteractionPrimitive(), 200.0f)) return Panel;
	}
	return nullptr;
}

void AHronoCharacter::Server_SetDoorRotation_Implementation(ADrag_Item* Door, FRotator NewRotation)
{
	Server_SetDoorPanelRotation_Implementation(Door, NAME_None, NewRotation);
}

void AHronoCharacter::Server_CommitDragPanelPose_Implementation(ADrag_Item* Item,
	FName ComponentName, bool bLinear, FVector Location, FRotator Rotation)
{
	if (!GetAllowedDragPanel(Item, ComponentName, bLinear)) return;
	if (bLinear) Server_SetShelfPanelPosition_Implementation(Item, ComponentName, Location);
	else Server_SetDoorPanelRotation_Implementation(Item, ComponentName, Rotation);
	// Locked/out-of-range releases expire through the actor's bounded audio timeout.
	if (IsValid(Item)) Item->FinishManualPanelMovement(ComponentName);
}

void AHronoCharacter::Server_SetDoorPanelRotation_Implementation(
	ADrag_Item* Door, FName DoorComponentName, FRotator NewRotation)
{
	if (NewRotation.ContainsNaN()) return;
	UDrag_Component* Panel = GetAllowedDragPanel(Door, DoorComponentName, false);
	if (!Panel) return;
	float MinYaw = Door->ItemType == EItemType::DraggableInvertLeft ? 0.0f : -90.0f;
	float MaxYaw = MinYaw + 90.0f;
	if (Panel->bUseCustomDoorAngleLimits)
	{
		MinYaw = FMath::Min(Panel->MinimumDoorYaw, Panel->MaximumDoorYaw);
		MaxYaw = FMath::Max(Panel->MinimumDoorYaw, Panel->MaximumDoorYaw);
	}
	if (!FMath::IsFinite(MinYaw) || !FMath::IsFinite(MaxYaw)) return;
	FRotator Allowed = Panel->GetTargetMovementComponent()->GetRelativeRotation();
	Allowed.Yaw = FMath::Clamp(NewRotation.Yaw, MinYaw, MaxYaw);
	Door->ApplyDoorRotationFromServer(DoorComponentName, Allowed);
}

void AHronoCharacter::ServerDropCurrentItem_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("[DropLog] 2. RPC Received on Server from Character: %s"), *GetName());
	DropCurrentItem();
}

void AHronoCharacter::DropCurrentItem()
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("[DropLog] DropCurrentItem aborted: Executing without Server Authority!"));
		return;
	}

	ABase_Item* ItemToDrop = CurrentHeldItem;

	if (!IsValid(ItemToDrop) || ItemToDrop->OwningCharacter != this)
	{
		// A stale hand reference must never drop another player's item.
		CurrentHeldItem = nullptr;
		ForceNetUpdate();
		UE_LOG(LogTemp, Log, TEXT("[Item] Drop ignored for %s: no valid owned item"), *GetName());
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[DropLog] 4. Valid item found: %s. Initiating Item->Drop()."), *ItemToDrop->GetName());
	ItemToDrop->Drop();
	CurrentHeldItem = nullptr;
	ForceNetUpdate();
}

void AHronoCharacter::ServerUnlockWithHeldKey_Implementation(ADrag_Item* Item)
{
	if (!CanInteractWithActorOnServer(Item)
		|| Item->IsLockedByTrigger()
		|| Item->IsDoorBlockedForTimeline(CharacterTimeline)
		|| !IsValid(CurrentHeldItem)
		|| CurrentHeldItem->OwningCharacter != this
		|| CurrentHeldItem->GetOwner() != this
		|| !CurrentHeldItem->bIsPickedUp
		|| !Item->bNeedKeyActor
		|| !Item->CanUnlockWithItem(CurrentHeldItem))
	{
		return;
	}

	ABase_Item* Key = CurrentHeldItem;
	CurrentHeldItem = nullptr;
	Item->bNeedKeyActor = false;
	Key->OnHeldStateChanged(false, this);
	Key->Destroy();
	CompleteTutorialStep(EHronoTutorialStep::UnlockOfficeDoor);
	ForceNetUpdate();
	Item->ForceNetUpdate();
}

ABase_Item* AHronoCharacter::GetHeldItem() const
{
	return CurrentHeldItem;
}

bool AHronoCharacter::ReleaseHeldItemForPlacement(ABase_Item* Item)
{
	if (!HasAuthority() || !IsValid(Item))
	{
		return false;
	}

	if (CurrentHeldItem != Item || Item->OwningCharacter != this)
	{
		return false;
	}

	// Release the hand before item Blueprint callbacks can run. Placement has no
	// intermediate dropped state, physics impulse, or drop sound.
	CurrentHeldItem = nullptr;
	if (!Item->ReleaseForPlacement(this))
	{
		CurrentHeldItem = Item;
		return false;
	}

	ForceNetUpdate();
	return true;
}

bool AHronoCharacter::TransferHeldItemTo(AHronoCharacter* TargetCharacter, ABase_Item* Item)
{
	if (!HasAuthority()
		|| !IsValid(TargetCharacter)
		|| TargetCharacter == this
		|| !IsValid(Item)
		|| CurrentHeldItem != Item
		|| Item->OwningCharacter != this
		|| IsValid(TargetCharacter->CurrentHeldItem)
		|| !IsValid(TargetCharacter->GetHeldItemInteractionPoint(Item)))
	{
		return false;
	}

	CurrentHeldItem = nullptr;
	Item->OnHeldStateChanged(false, this);

	Item->OwningCharacter = TargetCharacter;
	Item->SetOwner(TargetCharacter);
	TargetCharacter->CurrentHeldItem = Item;

	if (!Item->AttachToCharacter())
	{
		TargetCharacter->CurrentHeldItem = nullptr;
		Item->OwningCharacter = this;
		Item->SetOwner(this);
		CurrentHeldItem = Item;
		Item->AttachToCharacter();
		ForceNetUpdate();
		TargetCharacter->ForceNetUpdate();
		Item->ForceNetUpdate();
		return false;
	}

	ForceNetUpdate();
	TargetCharacter->ForceNetUpdate();
	Item->ForceNetUpdate();
	if (TargetCharacter->IsLocallyControlled())
	{
		TargetCharacter->HandleHeldItemTutorial(Item);
	}
	return true;
}

void AHronoCharacter::Server_SetShelfPosition_Implementation(ADrag_Item* Shelf, const FVector& NewPosition)
{
	Server_SetShelfPanelPosition_Implementation(Shelf, NAME_None, NewPosition);
}

bool AHronoCharacter::Server_SetShelfPosition_Validate(ADrag_Item* Shelf, const FVector& NewPosition)
{
	// Stale targets and invalid requests are rejected without disconnecting players.
	return true;
}

void AHronoCharacter::Server_SetShelfPanelPosition_Implementation(
	ADrag_Item* Shelf, FName ShelfComponentName, FVector NewPosition)
{
	if (NewPosition.ContainsNaN() || !GetAllowedDragPanel(Shelf, ShelfComponentName, true)) return;
	Shelf->ApplyShelfPositionFromServer(ShelfComponentName, NewPosition);
}

void AHronoCharacter::OnEnyInteractTrace(FHitResult HitResult)
{
	if (AActor* HitActor = HitResult.GetActor())
	{
		ADrag_Item* DragItem = Cast<ADrag_Item>(HitActor);
		const FName InteractionComponentName = HitResult.GetComponent()
			? HitResult.GetComponent()->GetFName()
			: NAME_None;
		if (DragItem
			&& DragItem->ShouldUseAutomaticOpenClose(this)
			&& DragItem->FindDragComponentForInteractionName(InteractionComponentName))
		{
			if (HasAuthority())
			{
				PerformAutomaticDragItemInteraction(DragItem, InteractionComponentName);
			}
			else
			{
				Server_ToggleAutomaticDragItem(DragItem, InteractionComponentName);
			}
			return;
		}

		const bool bImplementsInterface = HitActor->Implements<UEnviroment_Interface>();
		UE_LOG(LogTemp, Warning,
			TEXT("[InteractionDebug] TRACE CALLBACK Player=%s Actor=%s Component=%s Interface=%d Authority=%d"),
			*GetName(), *GetNameSafe(HitActor), *GetNameSafe(HitResult.GetComponent()),
			bImplementsInterface, HasAuthority());
		if (bImplementsInterface)
		{
			if (HasAuthority())
			{
				Server_InteractWithEnvironment_Implementation(HitActor);
			}
			else
			{
				// Client MUST ask the server to do the interaction
				Server_InteractWithEnvironment(HitActor);
			}
		}
	}
}

void AHronoCharacter::PerformAutomaticDragItemInteraction(
	ADrag_Item* Item,
	FName InteractionComponentName)
{
	if (!HasAuthority()
		|| !IsValid(Item)
		|| !Item->ShouldUseAutomaticOpenClose(this)
		|| InteractionComponentName.IsNone()
		|| !Item->FindDragComponentForInteractionName(InteractionComponentName)
		|| !CanInteractWithActorOnServer(Item, Item->FindDragComponentForInteractionName(InteractionComponentName)->GetInteractionPrimitive())
		|| Item->IsLockedByTrigger()
		|| Item->IsDoorBlockedForTimeline(CharacterTimeline))
	{
		return;
	}

	if (Item->bNeedKeyActor)
	{
		if (!Item->CanUnlockWithItem(CurrentHeldItem))
		{
			return;
		}
		ServerUnlockWithHeldKey_Implementation(Item);
		if (Item->bNeedKeyActor)
		{
			return;
		}
	}

	Item->ToggleAutomaticOpenClose(InteractionComponentName);
}

void AHronoCharacter::Server_ToggleAutomaticDragItem_Implementation(
	ADrag_Item* Item,
	FName InteractionComponentName)
{
	PerformAutomaticDragItemInteraction(Item, InteractionComponentName);
}

void AHronoCharacter::PickupItem(ABase_Item* Item)
{
	UE_LOG(LogTemp, Warning, TEXT("PickupItem Authority=%d"), HasAuthority());


	if (!HasAuthority())
	{
		return;
	}
	if (!IsValid(Item) || !Item->CanBePickedUp())
	{
		return;
	}

	if (!CanInteractWithActorOnServer(Item))
	{
		return;
	}

	if (Item->ItemType == EItemType::Clock)
	{
		if (AClock* Clock = Cast<AClock>(Item))
		{
			Clock->ResetClock(this);
			CompleteTutorialStep(EHronoTutorialStep::InteractWithClock);
		}
		else
		{
			Item->Use(this);
		}

		UE_LOG(LogTemp, Log, TEXT("RESET"));
		return;
	}

	// A chair is a world interaction, not an item carried in the player's hand.
	// A returning ritual victim may still be holding an item when they sit down.
	if (Cast<AChair>(Item))
	{
		Item->TryPickUp(this);
		return;
	}

	// One-hand rule: the server rejects every additional pickup until the held
	// item is dropped, consumed, or placed into an interaction socket. World
	// interactions above do not occupy the hand and remain usable while carrying.
	if (IsValid(CurrentHeldItem))
	{
		UE_LOG(LogTemp, Log,
			TEXT("[Item] Pickup rejected for %s: already holding %s"),
			*GetNameSafe(this), *GetNameSafe(CurrentHeldItem));
		return;
	}

	// Reserve before attachment/OnHeldStateChanged can re-enter gameplay code.
	// A failed acquisition releases only this reservation, not a later transfer.
	CurrentHeldItem = Item;
	const bool bPickedUp = Item->TryPickUp(this);
	if (!bPickedUp || !IsValid(Item) || Item->OwningCharacter != this || !Item->bIsPickedUp)
	{
		if (CurrentHeldItem == Item)
		{
			CurrentHeldItem = nullptr;
			ForceNetUpdate();
		}
		return;
	}

	if (CurrentHeldItem == Item)
	{
		ForceNetUpdate();
		if (ResolveTutorialItem(Item) == EHronoTutorialItem::Monocle)
		{
			CompleteTutorialStep(EHronoTutorialStep::PickUpMonocle);
		}
		else if (Item->ItemType == EItemType::Key)
		{
			CompleteTutorialStep(EHronoTutorialStep::PickUpKey);
		}
		if (IsLocallyControlled())
		{
			HandleHeldItemTutorial(Item);
		}
	}
}

void AHronoCharacter::OnRep_CurrentHeldItem(ABase_Item* PreviousHeldItem)
{
	if (CurrentHeldItem != PreviousHeldItem)
	{
		HandleHeldItemTutorial(CurrentHeldItem);
	}
}

void AHronoCharacter::EnsureTutorialWidget()
{
	if (!IsLocallyControlled() || IsValid(TutorialWidget))
	{
		return;
	}

	APlayerController* LocalController = Cast<APlayerController>(GetController());
	if (!LocalController)
	{
		return;
	}

	TSubclassOf<UHronoTutorialWidget> WidgetClass = TutorialWidgetClass;
	if (!WidgetClass)
	{
		WidgetClass = UHronoTutorialWidget::StaticClass();
	}
	TutorialWidget = CreateWidget<UHronoTutorialWidget>(LocalController, WidgetClass);
	if (TutorialWidget)
	{
		// Keep the modal tutorial above every gameplay HUD layer so invisible HUD
		// widgets cannot intercept its navigation buttons.
		TutorialWidget->AddToPlayerScreen(100000);
		TutorialWidget->SetGameStageText(TutorialGameStageText);
		ApplyTutorialStep(TutorialStep);
		for (EHronoTutorialItem Item : TutorialPromptedItems)
		{
			TutorialWidget->SetItemDiscovered(Item);
		}
	}
}

void AHronoCharacter::ToggleTutorialMenu()
{
	if (!IsLocallyControlled())
	{
		return;
	}
	EnsureTutorialWidget();
	if (TutorialWidget)
	{
		TutorialWidget->ToggleMenu();
	}
}

void AHronoCharacter::HandleHeldItemTutorial(ABase_Item* Item)
{
	if (!IsLocallyControlled() || !IsValid(Item))
	{
		return;
	}

	const EHronoTutorialItem TutorialItem = ResolveTutorialItem(Item);
	if (TutorialItem == EHronoTutorialItem::None || TutorialPromptedItems.Contains(TutorialItem))
	{
		return;
	}

	TutorialPromptedItems.Add(TutorialItem);
	EnsureTutorialWidget();
	if (TutorialWidget)
	{
		TutorialWidget->ShowPickupPrompt(TutorialItem);
	}
}

EHronoTutorialItem AHronoCharacter::ResolveTutorialItem(const ABase_Item* Item) const
{
	if (!IsValid(Item))
	{
		return EHronoTutorialItem::None;
	}
	if (TableRitualGate::IsCursedImage(*Item))
	{
		return EHronoTutorialItem::TableRitual;
	}
	if (Item->TutorialItem != EHronoTutorialItem::None)
	{
		return Item->TutorialItem;
	}
	if (Item->IsA<ADozimetr>()) return EHronoTutorialItem::Dosimeter;
	if (Item->IsA<AClock>()) return EHronoTutorialItem::Clock;
	if (Item->IsA<ARitualGoatSkull>()) return EHronoTutorialItem::Skull;
	if (Item->IsA<AAxeItem>()) return EHronoTutorialItem::Axe;

	const FString SearchName = Item->GetClass()->GetName() + TEXT(" ")
		+ Item->GetName() + TEXT(" ") + Item->ItemName.ToString();
	return SearchName.Contains(TEXT("Monocle"), ESearchCase::IgnoreCase)
		? EHronoTutorialItem::Monocle
		: EHronoTutorialItem::None;
}

void AHronoCharacter::SetTutorialGameStageText(const FText& NewStageText)
{
	TutorialGameStageText = NewStageText;
	if (TutorialWidget)
	{
		TutorialWidget->SetGameStageText(NewStageText);
	}
}

void AHronoCharacter::ApplyTutorialStep(EHronoTutorialStep NewStep)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	EnsureTutorialWidget();
	if (TutorialWidget)
	{
		TutorialWidget->SetTodoText(GetTutorialStepText(NewStep));
	}
}

bool AHronoCharacter::CompleteTutorialStep(EHronoTutorialStep CompletedStep)
{
	if (!HasAuthority()
		|| TutorialStep == EHronoTutorialStep::Completed
		|| CompletedStep != TutorialStep)
	{
		return false;
	}

	TutorialStep = static_cast<EHronoTutorialStep>(
		FMath::Min(
			static_cast<int32>(EHronoTutorialStep::Completed),
			static_cast<int32>(TutorialStep) + 1));
	OnRep_TutorialStep();
	ForceNetUpdate();
	return true;
}

void AHronoCharacter::OnRep_TutorialStep()
{
	ApplyTutorialStep(TutorialStep);
}

void AHronoCharacter::Server_InteractWithEnvironment_Implementation(AActor* InteractableActor)
{
	UE_LOG(LogTemp, Warning,
		TEXT("[InteractionDebug] SERVER RPC Player=%s Actor=%s Valid=%d Interface=%d Distance=%.1f"),
		*GetName(), *GetNameSafe(InteractableActor), IsValid(InteractableActor),
		InteractableActor && InteractableActor->Implements<UEnviroment_Interface>(),
		InteractableActor ? FVector::Distance(GetActorLocation(), InteractableActor->GetActorLocation()) : -1.0f);
	// The server verifies the actor is valid and implements the interface, then interacts
	if (CanInteractWithActorOnServer(InteractableActor) && InteractableActor->Implements<UEnviroment_Interface>())
	{
		IEnviroment_Interface::Execute_Interact(InteractableActor, this);
	}
}

void AHronoCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();
	if (bCorrectMoveInputWhenMirrored && IsMirroredViewEnabled())
	{
		MovementVector.X *= -1.0f;
	}

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);

}
void AHronoCharacter::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (bCorrectLookInputWhenMirrored && IsMirroredViewEnabled())
	{
		LookAxisVector.X *= -1.0f;
	}

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

void AHronoCharacter::MouseLookInput(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (bCorrectLookInputWhenMirrored && IsMirroredViewEnabled())
	{
		LookAxisVector.X *= -1.0f;
	}

	DoAim(
		LookAxisVector.X * MouseSensitivity,
		LookAxisVector.Y * MouseSensitivity);
}

void AHronoCharacter::ApplyLocalPlayerSettings(
	float NewMouseSensitivity,
	float NewFieldOfView)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	MouseSensitivity = FMath::Clamp(NewMouseSensitivity, 0.1f, 3.0f);
	CameraFieldOfView = FMath::Clamp(NewFieldOfView, 70.0f, 120.0f);
	if (IsValid(FirstPersonCameraComponent))
	{
		FirstPersonCameraComponent->SetFieldOfView(CameraFieldOfView);
	}
}

void AHronoCharacter::SetFpsCounterEnabled(bool bEnabled)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	bShowFpsCounter = bEnabled;
	if (!bEnabled)
	{
		if (IsValid(FpsCounterWidget))
		{
			FpsCounterWidget->RemoveFromParent();
			FpsCounterWidget = nullptr;
		}
		return;
	}

	if (!IsValid(FpsCounterWidget))
	{
		APlayerController* LocalController = Cast<APlayerController>(GetController());
		if (!IsValid(LocalController))
		{
			return;
		}

		FpsCounterWidget = CreateWidget<UHronoFpsWidget>(
			LocalController, UHronoFpsWidget::StaticClass());
	}

	if (IsValid(FpsCounterWidget) && !FpsCounterWidget->IsInViewport())
	{
		FpsCounterWidget->AddToViewport(10000);
	}
}

void AHronoCharacter::LoadLocalPlayerSettings()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	float SavedMouseSensitivity = 1.0f;
	float SavedFieldOfView = 90.0f;
	bool bSavedShowFps = false;
	static const FString SettingsSlot(TEXT("HronoMenuSettings"));
	if (UGameplayStatics::DoesSaveGameExist(SettingsSlot, 0))
	{
		if (const UHronoMenuSettingsSaveGame* Save = Cast<UHronoMenuSettingsSaveGame>(
			UGameplayStatics::LoadGameFromSlot(SettingsSlot, 0)))
		{
			SavedMouseSensitivity = Save->MouseSensitivity;
			SavedFieldOfView = Save->FieldOfView;
			bSavedShowFps = Save->bShowFps;
		}
	}

	ApplyLocalPlayerSettings(SavedMouseSensitivity, SavedFieldOfView);
	SetFpsCounterEnabled(bSavedShowFps);
}

void AHronoCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AHronoCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void AHronoCharacter::DoJumpStart()
{
	// pass Jump to the character
	Jump();

}

void AHronoCharacter::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
}
void AHronoCharacter::DoCrouchStart()
{
	UE_LOG(LogTemp, Warning,
		TEXT("Role=%d RemoteRole=%d Local=%d Authority=%d"),
		(int32)GetLocalRole(),
		(int32)GetRemoteRole(),
		IsLocallyControlled(),
		HasAuthority());

	Crouch();

	
	UE_LOG(LogTemp, Warning,
		TEXT("MovementMode=%d"),
		(int32)GetCharacterMovement()->MovementMode);


}
void AHronoCharacter::DoCrouchEnd()
{
	UnCrouch();
}
void AHronoCharacter::OnRep_Sprinting()
{
	ApplySprintMovementSpeed();
	OnSprintStateChanged.Broadcast(bSprinting);
}

void AHronoCharacter::OnRep_StaminaRecovery()
{
	ApplySprintMovementSpeed();
}

void AHronoCharacter::OnRep_StaminaData()
{
	MaxStamina = FMath::Max(MaxStamina, 1.0f);
	CurrentStamina = FMath::Clamp(CurrentStamina, 0.0f, MaxStamina);
	ApplySprintMovementSpeed();
	BroadcastStaminaChanged();
}

void AHronoCharacter::DoStartSprint()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	if (HasAuthority())
	{
		SetSprintingState(true);
	}
	else
	{
		ServerSetSprinting(true);
	}
}

void AHronoCharacter::DoEndSprint()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	if (HasAuthority())
	{
		SetSprintingState(false);
	}
	else
	{
		ServerSetSprinting(false);
	}
}

void AHronoCharacter::UpdateStamina(float DeltaSeconds)
{
	if (!HasAuthority() || DeltaSeconds <= 0.0f)
	{
		return;
	}

	MaxStamina = FMath::Max(MaxStamina, 1.0f);
	if (CurrentStamina > MaxStamina)
	{
		SetCurrentStamina(MaxStamina);
	}

	const float HorizontalSpeed = GetVelocity().Size2D();
	const bool bIsActuallyRunning = bSprinting
		&& !bRecovering
		&& HorizontalSpeed > WalkSpeed + 1.0f;

	if (bIsActuallyRunning)
	{
		StaminaRegenerationDelayRemaining = FMath::Max(StaminaRegenerationDelay, 0.0f);
		SetCurrentStamina(CurrentStamina - FMath::Max(StaminaDrainRate, 0.0f) * DeltaSeconds);

		if (CurrentStamina <= KINDA_SMALL_NUMBER)
		{
			SetCurrentStamina(0.0f);
			SetStaminaRecoveryState(true);
			SetSprintingState(false);
		}
	}
	else
	{
		StaminaRegenerationDelayRemaining = FMath::Max(
			StaminaRegenerationDelayRemaining - DeltaSeconds,
			0.0f);

		if (StaminaRegenerationDelayRemaining <= 0.0f && CurrentStamina < MaxStamina)
		{
			SetCurrentStamina(
				CurrentStamina + FMath::Max(StaminaRegenerationRate, 0.0f) * DeltaSeconds);
		}
	}

	const float RecoveryThreshold = FMath::Clamp(
		StaminaRecoveryThreshold,
		0.0f,
		MaxStamina);
	if (bRecovering && CurrentStamina >= RecoveryThreshold)
	{
		SetStaminaRecoveryState(false);
	}
}

void AHronoCharacter::SetCurrentStamina(float NewStamina)
{
	if (!HasAuthority())
	{
		return;
	}

	const float ClampedStamina = FMath::Clamp(NewStamina, 0.0f, FMath::Max(MaxStamina, 1.0f));
	if (FMath::IsNearlyEqual(CurrentStamina, ClampedStamina))
	{
		return;
	}

	CurrentStamina = ClampedStamina;
	OnRep_StaminaData();
}

void AHronoCharacter::SetSprintingState(bool bNewSprint)
{
	if (!HasAuthority())
	{
		return;
	}

	const float RequiredStamina = FMath::Clamp(
		MinimumStaminaToStartSprint,
		0.0f,
		FMath::Max(MaxStamina, 1.0f));
	const bool bCanStartSprint = !bRecovering
		&& CurrentStamina > KINDA_SMALL_NUMBER
		&& CurrentStamina >= RequiredStamina;
	const bool bAcceptedSprint = bNewSprint && bCanStartSprint;
	if (bSprinting == bAcceptedSprint)
	{
		ApplySprintMovementSpeed();
		return;
	}

	bSprinting = bAcceptedSprint;
	OnRep_Sprinting();
	ForceNetUpdate();
}

void AHronoCharacter::SetStaminaRecoveryState(bool bNewRecovering)
{
	if (!HasAuthority() || bRecovering == bNewRecovering)
	{
		return;
	}

	bRecovering = bNewRecovering;
	OnRep_StaminaRecovery();
	ForceNetUpdate();
}

void AHronoCharacter::ApplySprintMovementSpeed()
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!Movement)
	{
		return;
	}

	if (bRecovering)
	{
		Movement->MaxWalkSpeed = RecoveringWalkSpeed;
	}
	else if (bSprinting && CurrentStamina > KINDA_SMALL_NUMBER)
	{
		Movement->MaxWalkSpeed = SprintSpeed;
	}
	else
	{
		Movement->MaxWalkSpeed = WalkSpeed;
	}
}

void AHronoCharacter::BroadcastStaminaChanged()
{
	const float Percentage = GetStaminaPercentage();
	OnSprintMeterUpdated.Broadcast(Percentage);
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina, Percentage);
}

void AHronoCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (HasAuthority()) MulticastMovementSound(2, CharacterTimeline, SurfaceType_Default);
}

void AHronoCharacter::PlayFootstepSound()
{
	// Legacy animation notifies are ignored when native cadence owns footsteps.
	if (bUseMovementFootsteps || !HasAuthority() || !GetCharacterMovement()->IsMovingOnGround()
		|| GetVelocity().SizeSquared2D() < 100.0f || bIsSitting || bDeathTimelineTransitionPending) return;
	const double Now = GetWorld()->GetTimeSeconds();
	if (Now - LastFootstepTime < 0.12) return;
	LastFootstepTime = Now;
	MulticastMovementSound(0, CharacterTimeline, SurfaceType_Default);
}

void AHronoCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();
	if (HasAuthority()) MulticastMovementSound(1, CharacterTimeline, SurfaceType_Default);
}

void AHronoCharacter::UpdateMovementAudio(float DeltaSeconds)
{
	const UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!bUseMovementFootsteps || !IsValid(Controller) || !Movement->IsMovingOnGround()
		|| bIsSitting || bDeathTimelineTransitionPending || GetVelocity().SizeSquared2D() < 100.0f)
	{
		AccumulatedFootstepDistance = 0.0f;
		return;
	}
	AccumulatedFootstepDistance += GetVelocity().Size2D() * FMath::Clamp(DeltaSeconds, 0.0f, 0.1f);
	const float StepLength = FMath::Max(30.0f, FootstepDistance);
	if (AccumulatedFootstepDistance < StepLength) return;
	const double Now = GetWorld()->GetTimeSeconds();
	if (Now - LastFootstepTime < 0.12) return;
	AccumulatedFootstepDistance = FMath::Fmod(AccumulatedFootstepDistance, StepLength);
	LastFootstepTime = Now;
	FHitResult Ground;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(FootstepSurface), false, this);
	Params.bReturnPhysicalMaterial = true;
	const FVector Start = GetActorLocation();
	GetWorld()->LineTraceSingleByChannel(Ground, Start,
		Start - FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 30.0f), ECC_Visibility, Params);
	const uint8 Surface = static_cast<uint8>(UPhysicalMaterial::DetermineSurfaceType(Ground.PhysMaterial.Get()));
	MulticastMovementSound(0, CharacterTimeline, Surface);
}

void AHronoCharacter::MulticastMovementSound_Implementation(uint8 Event,
	EItemTimeline EventTimeline, uint8 Surface)
{
	if (!HronoAudioPolicy::CanHear(this, EventTimeline)) return;
	USoundBase* Sound = Event == 1 ? JumpSound.Get() : Event == 2 ? LandSound.Get() : FootstepSound.Get();
	if (!IsValid(Sound)) Sound = Event == 0 ? NativeFootstepFallback.Get() : NativeJumpFallback.Get();
	if (Event == 0)
	{
		const auto SurfaceKey = static_cast<EPhysicalSurface>(Surface);
		const TObjectPtr<USoundBase>* Variant = SurfaceFootstepSounds.Find(SurfaceKey);
		if (!Variant || !IsValid(*Variant)) Variant = NativeSurfaceFootstepFallbacks.Find(SurfaceKey);
		if (Variant)
		{
			if (IsValid(*Variant)) Sound = Variant->Get();
		}
	}
	if (IsValid(Sound)) UGameplayStatics::PlaySoundAtLocation(this, Sound,
		GetActorLocation(), 1.0f, 1.0f, 0.0f, MovementSoundAttenuation);
}
