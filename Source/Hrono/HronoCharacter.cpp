// Copyright Epic Games, Inc. All Rights Reserved.

#include "HronoCharacter.h"
#include "HronoCollisionChannels.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Hrono.h"
#include "Net/UnrealNetwork.h"
#include "Components/Drag_Component.h"
#include "Items/Drag_Item.h"
#include "Components/SpotLightComponent.h"
#include "Interface/Enviroment_Interface.h"
#include "Items/Base_Item.h"


#include "Components/InventoryComponent.h"




void AHronoCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHronoCharacter, CharacterTimeline);
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

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));

	SpotLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotLight1"));
	SpotLight->SetupAttachment(GetFirstPersonCameraComponent());

	SpotLight->SetRelativeLocationAndRotation(FVector(30.0f, 17.5f, -5.0f), FRotator(-18.6f, -1.3f, 5.26f));
	SpotLight->Intensity = 0.5;
	SpotLight->SetIntensityUnits(ELightUnits::Lumens);
	SpotLight->AttenuationRadius = 1050.0f;
	SpotLight->InnerConeAngle = 18.7f;
	SpotLight->OuterConeAngle = 45.24f;

}

void AHronoCharacter::OnRep_CharacterTimeline()
{
	ApplyTimelineCollision();

}

void AHronoCharacter::ApplyTimelineCollision()
{
	if (CharacterTimeline == EItemTimeline::Past)
	{
		GetCapsuleComponent()->SetCollisionObjectType(COLLISION_CHANNEL_PAWN_PAST);
		GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_CHANNEL_DOOR_PAST, ECR_Block);
		GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_CHANNEL_DOOR_FUTURE, ECR_Ignore);
	}
	else
	{
		GetCapsuleComponent()->SetCollisionObjectType(COLLISION_CHANNEL_PAWN_FUTURE);
		GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_CHANNEL_DOOR_FUTURE, ECR_Block);
		GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_CHANNEL_DOOR_PAST, ECR_Ignore);
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

		// Drop held item
		if (DropAction)
		{
			EnhancedInputComponent->BindAction(DropAction, ETriggerEvent::Started, this, &AHronoCharacter::DoDrop);
		}
	}
	else
	{
		UE_LOG(LogHrono, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
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

	// DEBUG: Print timeline
	const char* TimelineStr = (CharacterTimeline == EItemTimeline::Future) ? "FUTURE" : "PAST";
	UE_LOG(LogTemp, Warning, TEXT("Character Timeline: %s"), ANSI_TO_TCHAR(TimelineStr));
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
		
		DrawDebugSphere(
			GetWorld(),
			HitResult.ImpactPoint,
			12.0f,
			12,
			FColor::Red,
			false,
			2.0f
		);

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

		if (Door->ItemTimeline != CharacterTimeline)
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

	FHitResult HitResult = PerformInteractTrace(false);

	if (HitResult.bBlockingHit)
	{
		HandleInteraction(HitResult);
	}
}

void AHronoCharacter::ServerPickupItem_Implementation(ABase_Item* Item)
{
	UE_LOG(LogTemp, Warning, TEXT("[Item] ServerPickupItem on %s"), *GetName());

	PickupItem(Item);
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

	auto DragComponent = Item->FindComponentByClass<UDrag_Component>();
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

	if (DragComponent->bIsShelf)
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
	if (!Door || !Door->ItemMesh) return;

	// Server is authoritative: apply the rotation to the door's collision body so the
	// movement validation sees the same open/closed state as the requesting client,
	// then replicate it (DoorRotation -> OnRep_DoorRotation) to every other machine.
	Door->ItemMesh->SetRelativeRotation(NewRotation);
	Door->DoorRotation = NewRotation;

	// Recompute and replicate the closed/open flag from the new Yaw.
	Door->RefreshDoorClosedState();
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
		if (HitActor->Implements<UEnviroment_Interface>())
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
void AHronoCharacter::PickupItem(ABase_Item* Item)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!Item)
	{
		return;
	}

	if (Item->TryPickUp(this))
	{
		InventoryComponent->AddItem(Item);
	}
}

void AHronoCharacter::Server_InteractWithEnvironment_Implementation(AActor* InteractableActor)
{
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
}

void AHronoCharacter::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
}
