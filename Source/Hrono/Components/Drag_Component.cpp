// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Drag_Component.h"
#include "Items/Drag_Item.h"
#include "HronoCharacter.h"
#include "InputCoreTypes.h"


// Sets default values for this component's properties
UDrag_Component::UDrag_Component()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UDrag_Component::BeginPlay()
{
	Super::BeginPlay();

	if (const ADrag_Item* DragItem = Cast<ADrag_Item>(GetOwner()))
	{
		if (DragItem->ItemMesh)
		{
			CupBoardClosedLocation = DragItem->ItemMesh->GetRelativeLocation();
		}
	}
	
}


// Called every frame
void UDrag_Component::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!bIsRotating || !RotatingController)
	{
		return;
	}

	if (!RotatingController->IsInputKeyDown(EKeys::LeftMouseButton))
	{
		StopDrag();
		return;
	}

	
	if (bIsCupBoard)
	{
		CupBoardDrag();
	}
	else if (bIsShelf)
	{
		ShelfDrag();
	}
	else
	{
		XDrag();
	}
	
	
}

void UDrag_Component::StartDrag(APlayerController* PC)
{
	if (!PC) return;

	bIsRotating = true;
	RotatingController = PC;

	// Start the looping movement sound on the owning door/shelf actor.
	if (ADrag_Item* Drag_Item = Cast<ADrag_Item>(GetOwner()))
	{
		const bool bLinearDrag = bIsShelf || bIsCupBoard;
		Drag_Item->StartMoveSound(bLinearDrag);

		// Notify Blueprints that a drag interaction has begun.
		Drag_Item->NotifyDragStarted(bLinearDrag);
	}

	//UE_LOG(LogTemp, Log, TEXT("Drag started"));
}

void UDrag_Component::StopDrag()
{
	bIsRotating = false;
	RotatingController = nullptr;

	// Stop the looping movement sound on the owning door/shelf actor.
	if (ADrag_Item* Drag_Item = Cast<ADrag_Item>(GetOwner()))
	{
		Drag_Item->StopMoveSound();
	}

	//UE_LOG(LogTemp, Log, TEXT("Drag stopped"));
}

void UDrag_Component::XDrag()
{
	AActor* Owner = GetOwner();
	if (!Owner || !RotatingController) return;

	auto Drag_Item = Cast<ADrag_Item>(Owner);
	if (!Drag_Item) return;

	APawn* PlayerPawn = RotatingController->GetPawn();
	if (!PlayerPawn) return;

	float MouseX, MouseY;
	RotatingController->GetInputMouseDelta(MouseX, MouseY);

	FVector DoorForward = Owner->GetActorForwardVector();

	FVector DoorToPlayer =
		PlayerPawn->GetActorLocation() -
		Owner->GetActorLocation();

	DoorToPlayer.Z = 0.f;
	DoorToPlayer.Normalize();

	FVector DoorRight = Owner->GetActorRightVector();

	float Side =
		FVector::DotProduct(
			DoorRight,
			DoorToPlayer
		);

	float DirectionMultiplier =
		(Side < 0.f) ? -1.f : 1.f;
	FRotator OldRotation = Drag_Item->ItemMesh->GetRelativeRotation();

	FRotator NewRotation = OldRotation;

	if (Drag_Item->ItemType == EItemType::DraggableInvertLeft)
	{
		NewRotation.Yaw = FMath::Clamp(
			NewRotation.Yaw + MouseX * RotationSpeed * DirectionMultiplier * -1,
			0.f,
			90.f
		);
	}
	else if (Drag_Item->ItemType == EItemType::DraggableInvertRight)
	{
		NewRotation.Yaw = FMath::Clamp(
			NewRotation.Yaw + MouseX * RotationSpeed * DirectionMultiplier * -1,
			-90.f,
			0.f
			
		);
	}
	else 
	{
		NewRotation.Yaw = FMath::Clamp(
			NewRotation.Yaw + MouseX * RotationSpeed * DirectionMultiplier,
			-90.f,
			0.f
		);
	}


	bool bOverlappingPlayer = Drag_Item->ItemMesh->IsOverlappingActor(PlayerPawn);
	if (bOverlappingPlayer)
	{
		NewRotation = OldRotation;
	}

	// Apply rotation locally for immediate feedback (prediction)
	Drag_Item->ItemMesh->SetRelativeRotation(NewRotation);
	Drag_Item->DoorRotation = NewRotation;

	// Send the new rotation to the server so it updates the authoritative collision
	// body and replicates it to every other client. The door is a level actor and
	// cannot receive client RPCs directly, so we route through the owning character.
	if (AHronoCharacter* Character = Cast<AHronoCharacter>(RotatingController->GetPawn()))
	{
		if (!Character->HasAuthority())
		{
			Character->Server_SetDoorRotation(Drag_Item, NewRotation);
		}
		else
		{
			// Listen-server host drags the door locally with authority, so refresh
			// the replicated closed/open state here (no RPC round-trip happens).
			Drag_Item->RefreshDoorClosedState();
		}
	}
}



void UDrag_Component::ShelfDrag()
{
	AActor* Owner = GetOwner();
	if (!Owner || !RotatingController)
		return;

	ADrag_Item* Shelf = Cast<ADrag_Item>(Owner);
	if (!Shelf)
		return;

	APawn* PlayerPawn = RotatingController->GetPawn();
	if (!PlayerPawn)
		return;

	float MouseX, MouseY;
	RotatingController->GetInputMouseDelta(MouseX, MouseY);

	// Use only vertical mouse movement
	float DragDelta = MouseY;

	FVector OldRelativeLocation =
		Shelf->ItemMesh->GetRelativeLocation();

	FVector NewRelativeLocation =
		OldRelativeLocation;

	float NewY = FMath::Clamp(
		OldRelativeLocation.Y +
		DragDelta * ShelfSpeed,
		-ShelfMaxDistance,
		0.f
	);

	NewRelativeLocation.Y = NewY;

	bool bOverlappingPlayer =
		Shelf->ItemMesh->IsOverlappingActor(PlayerPawn);

	if (bOverlappingPlayer)
	{
		NewRelativeLocation =
			OldRelativeLocation;
	}

	Shelf->ItemMesh->SetRelativeLocation(
		NewRelativeLocation
	);

	Shelf->ShelfPosition =
		NewRelativeLocation;

	if (AHronoCharacter* Character =
		Cast<AHronoCharacter>(PlayerPawn))
	{
		if (!Character->HasAuthority())
		{
			Character->Server_SetShelfPosition(
				Shelf,
				NewRelativeLocation
			);
		}
		else
		{
			Shelf->RefreshShelfOpenState();
		}
	}
}

void UDrag_Component::CupBoardDrag()
{
	ADrag_Item* CupBoard = Cast<ADrag_Item>(GetOwner());
	if (!CupBoard || !CupBoard->ItemMesh || !RotatingController)
	{
		return;
	}

	APawn* PlayerPawn = RotatingController->GetPawn();
	if (!PlayerPawn)
	{
		return;
	}

	float MouseX = 0.0f;
	float MouseY = 0.0f;
	RotatingController->GetInputMouseDelta(MouseX, MouseY);

	const FVector SlideAxis = CupBoardSlideAxis.GetSafeNormal();
	if (SlideAxis.IsNearlyZero())
	{
		return;
	}

	const FVector OldRelativeLocation = CupBoard->ItemMesh->GetRelativeLocation();
	const float CurrentOffset = FVector::DotProduct(
		OldRelativeLocation - CupBoardClosedLocation,
		SlideAxis);
	const float NewOffset = FMath::Clamp(
		CurrentOffset - MouseX * CupBoardSlideSpeed,
		0.0f,
		CupBoardMaxDistance);

	FVector NewRelativeLocation = CupBoardClosedLocation + SlideAxis * NewOffset;
	if (CupBoard->ItemMesh->IsOverlappingActor(PlayerPawn))
	{
		NewRelativeLocation = OldRelativeLocation;
	}

	CupBoard->ItemMesh->SetRelativeLocation(NewRelativeLocation);
	CupBoard->ShelfPosition = NewRelativeLocation;

	if (AHronoCharacter* Character = Cast<AHronoCharacter>(PlayerPawn))
	{
		if (!Character->HasAuthority())
		{
			Character->Server_SetShelfPosition(CupBoard, NewRelativeLocation);
		}
		else
		{
			CupBoard->RefreshShelfOpenState();
		}
	}
}
