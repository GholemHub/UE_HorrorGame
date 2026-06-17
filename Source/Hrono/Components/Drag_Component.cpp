// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Drag_Component.h"
#include "Items/Drag_Item.h"
#include "HronoCharacter.h"


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

	// ...
	
}


// Called every frame
void UDrag_Component::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!bIsRotating || !RotatingController)
		return;

	XDrag();
	//YDrag();
}

void UDrag_Component::StartDrag(APlayerController* PC)
{
	if (!PC) return;

	bIsRotating = true;
	RotatingController = PC;

	UE_LOG(LogTemp, Log, TEXT("Drag started"));
}

void UDrag_Component::StopDrag()
{
	bIsRotating = false;
	RotatingController = nullptr;

	UE_LOG(LogTemp, Log, TEXT("Drag stopped"));
}

void UDrag_Component::XDrag()
{
	AActor* Owner = GetOwner();
	if (!Owner || !RotatingController) return;

	auto Door = Cast<ADrag_Item>(Owner);
	if (!Door) return;

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

	FRotator OldRotation = Door->ItemMesh->GetRelativeRotation();

	FRotator NewRotation = OldRotation;

	NewRotation.Yaw = FMath::Clamp(
		NewRotation.Yaw + MouseX * RotationSpeed * DirectionMultiplier,
		-90.f,
		0.f
	);

	AHronoCharacter* HronoChar = Cast<AHronoCharacter>(PlayerPawn);

	// Якщо рух блокується гравцем (твоя перевірка Overlap)
	bool bOverlappingPlayer = Door->ItemMesh->IsOverlappingActor(PlayerPawn);
	if (bOverlappingPlayer)
	{
		NewRotation = OldRotation; // скасовуємо рух
	}

	// Застосовуємо локально для миттєвого відгуку (Prediction)
	Door->ItemMesh->SetRelativeRotation(NewRotation);
	Door->DoorRotation = NewRotation;

	// --- НОВИЙ КОД ДИНАМІЧНОЇ КОЛІЗІЇ ---
	if (HronoChar && !HronoChar->HasAuthority())
	{
		// Якщо Yaw менше -5.0, вважаємо, що двері прочинені і крізь них можна йти.
		// Якщо Yaw від 0 до -5.0, вважаємо двері закритими (треба блокувати).
		bool bShouldIgnoreOnServer = (NewRotation.Yaw < -45.f);

		// Відправляємо RPC тільки тоді, коли стан реально змінився (захист від спаму)
		if (bServerCollisionIgnored != bShouldIgnoreOnServer)
		{
			bServerCollisionIgnored = bShouldIgnoreOnServer;
			HronoChar->Server_SetDoorCollisionIgnored(Door, bShouldIgnoreOnServer);
		}
	}
}



void UDrag_Component::YDrag()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;
	if (!RotatingController || !bIsRotating)
	{
		bIsRotating = false;
		return;
	}

	float MouseX, MouseY;
	RotatingController->GetInputMouseDelta(MouseX, MouseY);

	FRotator NewRotation = Owner->GetRootComponent()->GetRelativeRotation();

	NewRotation.Roll = FMath::Clamp(
		NewRotation.Roll + MouseY * RotationSpeed,
		0.f,
		60.f
	);

	Owner->GetRootComponent()->SetRelativeRotation(NewRotation);
}


