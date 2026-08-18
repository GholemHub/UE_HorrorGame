// Copyright Epic Games, Inc. All Rights Reserved.

#include "HronoCharacter.h"
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
#include "Components/SpotLightComponent.h"
#include "Interface/Enviroment_Interface.h"
#include "Items/Base_Item.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Engine.h"

#include "Components/InventoryComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

void AHronoCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHronoCharacter, CharacterTimeline);
	DOREPLIFETIME(AHronoCharacter, bSprinting);
	DOREPLIFETIME(AHronoCharacter, CurrentChair);
	DOREPLIFETIME(AHronoCharacter, bIsSitting);
	DOREPLIFETIME(AHronoCharacter, bIsSafeInHidingWardrobe);
	DOREPLIFETIME(AHronoCharacter, bTimelineMirrorRequested);
}

void AHronoCharacter::SetSafeInHidingWardrobe(bool bNewSafe)
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
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_CHANNEL_ITEM, ECR_Ignore);

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

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

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));

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

void AHronoCharacter::OnRep_TimelineMirrorRequested()
{
	SetMirroredViewEnabled(bTimelineMirrorRequested);
}

void AHronoCharacter::SwitchPlayerTimeline()
{
	const EItemTimeline RequestedTimeline = CharacterTimeline == EItemTimeline::Past
		? EItemTimeline::Future
		: EItemTimeline::Past;

	if (HasAuthority())
	{
		ApplyPlayerTimelineOnAuthority(RequestedTimeline);
		return;
	}

	// Send the concrete target rather than another "toggle" command. If the same
	// death event runs on server and owning client, both requests now converge on
	// one timeline instead of toggling there and immediately back again.
	ServerSetPlayerTimeline(RequestedTimeline);
}

void AHronoCharacter::SetPlayerTimeline(EItemTimeline NewTimeline)
{
	if (NewTimeline == EItemTimeline::Both)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[TimelineSwitch] %s rejected Both: player timeline must be Past or Future"),
			*GetNameSafe(this));
		return;
	}

	if (HasAuthority())
	{
		ApplyPlayerTimelineOnAuthority(NewTimeline);
		return;
	}

	ServerSetPlayerTimeline(NewTimeline);
}

void AHronoCharacter::ServerSetPlayerTimeline_Implementation(EItemTimeline NewTimeline)
{
	if (NewTimeline != EItemTimeline::Both)
	{
		ApplyPlayerTimelineOnAuthority(NewTimeline);
	}
}

void AHronoCharacter::ClientApplyTimelineMirror_Implementation(EItemTimeline NewTimeline)
{
	SetMirroredViewEnabled(NewTimeline == EItemTimeline::Past);
}

void AHronoCharacter::ApplyPlayerTimelineOnAuthority(EItemTimeline NewTimeline)
{
	if (!HasAuthority() || NewTimeline == EItemTimeline::Both)
	{
		return;
	}

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
		return;
	}

	const double ServerTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
	if (LastTimelineSwitchServerTime >= 0.0
		&& ServerTime - LastTimelineSwitchServerTime < TimelineSwitchDuplicateGuardSeconds)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[TimelineSwitch] Duplicate toggle blocked for %s: requested=%s after %.3fs"),
			*GetNameSafe(this),
			*StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(NewTimeline)),
			ServerTime - LastTimelineSwitchServerTime);
		return;
	}
	LastTimelineSwitchServerTime = ServerTime;

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
		TEXT("[TimelineSwitch] Character=%s %s -> %s CarriedItems=%d Mirrored=%s"),
		*GetNameSafe(this),
		*StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(PreviousTimeline)),
		*StaticEnum<EItemTimeline>()->GetNameStringByValue(static_cast<int64>(CharacterTimeline)),
		InventoryComponent ? InventoryComponent->Items.Num() : 0,
		bTimelineMirrorRequested ? TEXT("true") : TEXT("false"));

	ForceNetUpdate();
}

void AHronoCharacter::MoveCarriedItemsToTimeline(EItemTimeline NewTimeline)
{
	if (!HasAuthority() || !InventoryComponent)
	{
		return;
	}

	for (ABase_Item* Item : InventoryComponent->Items)
	{
		if (IsValid(Item))
		{
			Item->SetItemTimeline(NewTimeline);
		}
	}

	InventoryComponent->UpdateItemVisibility();
}

void AHronoCharacter::RefreshHeldItemsInteractionPoint()
{
	if (!InventoryComponent)
	{
		return;
	}

	for (ABase_Item* Item : InventoryComponent->Items)
	{
		if (IsValid(Item) && Item->OwningCharacter == this)
		{
			Item->RefreshHeldAttachmentPoint();
		}
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
		}
		if (USkeletalMeshComponent* ArmsMesh = OtherCharacter->GetFirstPersonMesh())
		{
			ArmsMesh->SetOnlyOwnerSee(true);
			ArmsMesh->SetOwnerNoSee(false);
		}

		TInlineComponentArray<UPrimitiveComponent*> RenderComponents(OtherCharacter);

		// OwnerNoSee removes the local body from the raster camera. Explicitly
		// removing it from ray tracing also removes the local Lumen reflection.
		if (OtherCharacter == LocalViewer)
		{
			for (UPrimitiveComponent* RenderComponent : RenderComponents)
			{
				if (IsValid(RenderComponent))
				{
					RenderComponent->SetVisibleInRayTracing(false);
				}
			}
			continue;
		}

		const EItemTimeline OtherTimeline = OtherCharacter->CharacterTimeline;
		const bool bSameTimeline = ViewerTimeline == EItemTimeline::Both
			|| OtherTimeline == EItemTimeline::Both
			|| ViewerTimeline == OtherTimeline;
		const TWeakObjectPtr<AHronoCharacter> OtherKey(OtherCharacter);

		if (!bSameTimeline)
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
				// viewer. UE 5.8 still keeps the primitive in the hardware RT scene,
				// so it remains visible in the roughness-0 Lumen mirror.
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
	bSprinting = bNewSprint;

	if (bSprinting && !bRecovering)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
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

		//Inventory
		EnhancedInputComponent->BindAction(NextItemAction, ETriggerEvent::Triggered, this, &AHronoCharacter::NextItemInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHronoCharacter::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AHronoCharacter::LookInput);

		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AHronoCharacter::DoInteract);

		// Sprinting
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AHronoCharacter::DoStartSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AHronoCharacter::DoEndSprint);

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

	// Server assigns timeline: its own local player = Future, remote players = Past.
	// On a dedicated server (no local player), all characters default to Past
	// unless overridden by GameMode logic.
	if (HasAuthority())
	{
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
}

void AHronoCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
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

	DrawDebugLine(
		GetWorld(),
		Start,
		End,
		FColor::Green,
		false,
		0.2f,
		0,
		2.0f
	);

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
		if (!bIsDrag) {
			OnEnyInteractTrace(HitResult);
		}
		else
		{
			OnMakeInteractImpulse(HitResult);
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
		*GetNameSafe(GetCurrentInventoryItem()));

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
		UGameplayStatics::PlaySoundAtLocation(this, InteractSound, GetActorLocation());
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

#include "Items/Chair.h"

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

	// RepNotify is not called on the authority, so invoke it manually to keep the
	// server (listen-server host) in sync with clients.
	OnRep_CurrentChair(PreviousChair);
}

void AHronoCharacter::SitOnChair(AChair* Chair)
{
	if (!HasAuthority())
		return;

	if (bIsSitting || CurrentChair)
		return;

	if (Chair->bIsSit)
		return;

	AChair* PreviousChair = CurrentChair;

	CurrentChair = Chair;
	bIsSitting = true;

	Chair->bIsSit = true;
	Chair->SetSitter(this);

	// RepNotify is not called on the authority, so invoke it manually to keep the
	// server (listen-server host) in sync with clients.
	OnRep_CurrentChair(PreviousChair);
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

	// Multi-door actors own more than one UDrag_Component. Select the one whose
	// interaction primitive was actually hit instead of always taking the first.
	auto DragComponent = Item->FindDragComponentForHit(HitResult.GetComponent());
	if (!DragComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("HandleDrag FAILED: No DragComponent found!"));
		return;
	}

	if (Item->bNeedKeyActor)
	{
		const FGameplayTag BasementKeyTag =
			FGameplayTag::RequestGameplayTag(TEXT("Item.Key"));

		if (!InventoryComponent->FindItemByTag(BasementKeyTag))
		{
			return;
		}
		else {
			auto Key = InventoryComponent->FindItemByTag(BasementKeyTag);
			InventoryComponent->ConsumeItem(Key);
			Item->bNeedKeyActor = false;
		}
	}

	CurrentDraggedComponent = DragComponent;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("HandleDrag FAILED: No PlayerController!"));
		return;
	}

	DragComponent->StartDrag(PC);

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

void AHronoCharacter::Server_SetDoorRotation_Implementation(ADrag_Item* Door, FRotator NewRotation)
{
	if (!Door)
	{
		return;
	}

	Door->ApplyDoorRotationFromServer(NAME_None, NewRotation);
}

void AHronoCharacter::Server_SetDoorPanelRotation_Implementation(
	ADrag_Item* Door,
	FName DoorComponentName,
	FRotator NewRotation)
{
	if (!Door)
	{
		return;
	}

	Door->ApplyDoorRotationFromServer(DoorComponentName, NewRotation);
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

	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("[DropLog] DropCurrentItem failed: InventoryComponent is NULL!"));
		return;
	}

	// Read internal inventory variables directly for logging
	int32 TargetIndex = InventoryComponent->CurrentItemIndex;
	int32 InventoryCount = InventoryComponent->Items.Num();

	UE_LOG(LogTemp, Log, TEXT("[DropLog] 3. Server evaluating Inventory. CurrentItemIndex: %d, Total Items: %d"), TargetIndex, InventoryCount);

	ABase_Item* ItemToDrop = InventoryComponent->GetCurrentItem();

	if (!ItemToDrop)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DropLog] DropCurrentItem aborted: GetCurrentItem() returned nullptr!"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[DropLog] 4. Valid item found: %s. Initiating Item->Drop()."), *ItemToDrop->GetName());
	ItemToDrop->Drop();

	UE_LOG(LogTemp, Log, TEXT("[DropLog] 6. Removing item from inventory array..."));
	InventoryComponent->RemoveItem(ItemToDrop);
}

ABase_Item* AHronoCharacter::GetCurrentInventoryItem() const
{
	return InventoryComponent ? InventoryComponent->GetCurrentItem() : nullptr;
}

bool AHronoCharacter::ReleaseInventoryItemForPlacement(ABase_Item* Item)
{
	if (!HasAuthority() || !InventoryComponent || !IsValid(Item))
	{
		return false;
	}

	if (!InventoryComponent->Items.Contains(Item) || Item->OwningCharacter != this)
	{
		return false;
	}

	// Drop first so Base_Item clears attachment, ownership and held-state effects.
	// The rune will immediately disable physics and move itself into its slot.
	Item->Drop();
	InventoryComponent->RemoveItem(Item);

	if (CurrentHeldItem == Item)
	{
		CurrentHeldItem = nullptr;
	}

	ForceNetUpdate();
	return true;
}

void AHronoCharacter::Server_SetShelfPosition_Implementation(ADrag_Item* Shelf, const FVector& NewPosition)
{
	if (!Shelf) return;

	Shelf->ShelfPosition = NewPosition;
	Shelf->ItemMesh->SetRelativeLocation(NewPosition);
	Shelf->RefreshShelfOpenState();
}

bool AHronoCharacter::Server_SetShelfPosition_Validate(ADrag_Item* Shelf, const FVector& NewPosition)
{
	return Shelf != nullptr;

}

void AHronoCharacter::OnEnyInteractTrace(FHitResult HitResult)
{
	if (AActor* HitActor = HitResult.GetActor())
	{
		const bool bImplementsInterface = HitActor->Implements<UEnviroment_Interface>();
		UE_LOG(LogTemp, Warning,
			TEXT("[InteractionDebug] TRACE CALLBACK Player=%s Actor=%s Component=%s Interface=%d Authority=%d"),
			*GetName(), *GetNameSafe(HitActor), *GetNameSafe(HitResult.GetComponent()),
			bImplementsInterface, HasAuthority());
		if (bImplementsInterface)
		{
			if (HasAuthority())
			{
				// Server can interact directly
				IEnviroment_Interface::Execute_Interact(HitActor, this);
			}
			else
			{
				// Client MUST ask the server to do the interaction
				Server_InteractWithEnvironment(HitActor);
			}
		}
	}
}

void AHronoCharacter::PickupItem(ABase_Item* Item)
{
	UE_LOG(LogTemp, Warning, TEXT("PickupItem Authority=%d"), HasAuthority());


	if (!HasAuthority())
	{
		return;
	}

	if (!Item)
	{
		return;
	}

	if (Item->ItemType == EItemType::Chair)
	{
		Item->Use(this);
		return;
	}

	if (Item->ItemType == EItemType::Clock)
	{
		Item->Use(this);

		UE_LOG(LogTemp, Log, TEXT("RESET"));

		return;
	}

	
	if (Item->TryPickUp(this))
	{
		InventoryComponent->AddItem(Item);
	}
}
void AHronoCharacter::OnMakeInteractImpulse(FHitResult HitResult)
{
	UPrimitiveComponent* HitComp = HitResult.GetComponent();

	if (HitComp && HitComp->IsSimulatingPhysics())
	{
		FVector ImpulseDirection = GetControlRotation().Vector();
		float ImpulseStrength = 300.f;

		HitComp->AddImpulse(
			ImpulseDirection * ImpulseStrength,
			NAME_None,
			true
		);
	}
}
void AHronoCharacter::Server_InteractWithEnvironment_Implementation(AActor* InteractableActor)
{
	UE_LOG(LogTemp, Warning,
		TEXT("[InteractionDebug] SERVER RPC Player=%s Actor=%s Valid=%d Interface=%d Distance=%.1f"),
		*GetName(), *GetNameSafe(InteractableActor), IsValid(InteractableActor),
		InteractableActor && InteractableActor->Implements<UEnviroment_Interface>(),
		InteractableActor ? FVector::Distance(GetActorLocation(), InteractableActor->GetActorLocation()) : -1.0f);
	// The server verifies the actor is valid and implements the interface, then interacts
	if (InteractableActor && InteractableActor->Implements<UEnviroment_Interface>())
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
void AHronoCharacter::NextItemInput(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	if (MovementVector.X == 0) return;

	// Forward client input directly over to Server authority
	if (!HasAuthority())
	{
		ServerNextItemInput(MovementVector.X);
	}
	else
	{
		if (MovementVector.X > 0) {
			InventoryComponent->DoNextItem();
		}
		else {
			InventoryComponent->DoPreviousItem();
		}
	}
}

void AHronoCharacter::ServerNextItemInput_Implementation(float AxisValue)
{
	if (AxisValue > 0) {
		InventoryComponent->DoNextItem();
	}
	else {
		InventoryComponent->DoPreviousItem();
	}
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

	// play the jump sound
	UGameplayStatics::PlaySoundAtLocation(this, JumpSound, GetActorLocation());
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
	if (bSprinting && !bRecovering)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
}
void AHronoCharacter::DoStartSprint()
{
	if (!IsLocallyControlled()) return;

	bSprinting = true;
	if (!bRecovering)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
	ServerSetSprinting(true);
}

void AHronoCharacter::DoEndSprint()
{
	if (!IsLocallyControlled()) return;

	bSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	ServerSetSprinting(false);
}


void AHronoCharacter::SprintFixedTick()
{
	float Velocity = GetVelocity().Size();

	UE_LOG(LogTemp, Verbose, TEXT("[SprintTick] bSprinting=%d bRecovering=%d Velocity=%.2f SprintMeter=%.2f"),
		bSprinting, bRecovering, Velocity, SprintMeter);
	if (bSprinting && !bRecovering && Velocity > WalkSpeed)
	{
		if (SprintMeter > 0.0f)
		{
			SprintMeter = FMath::Max(SprintMeter - SprintFixedTickTime, 0.0f);

			UE_LOG(LogTemp, Warning, TEXT("[SprintTick] Drain stamina: %f"), SprintMeter);

			if (SprintMeter <= 0.0f)
			{
				bRecovering = true;

				GetCharacterMovement()->MaxWalkSpeed = RecoveringWalkSpeed;

				UE_LOG(LogTemp, Error, TEXT("[SprintTick] OUT OF STAMINA -> RECOVERING MODE"));
			}
		}
	}
}

void AHronoCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// play the landing sound
	UGameplayStatics::PlaySoundAtLocation(this, LandSound, GetActorLocation());
}

void AHronoCharacter::PlayFootstepSound()
{
	// called from an animation notify while walking/running
	UGameplayStatics::PlaySoundAtLocation(this, FootstepSound, GetActorLocation());
}
