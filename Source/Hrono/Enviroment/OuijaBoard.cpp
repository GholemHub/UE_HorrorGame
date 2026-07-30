#include "Enviroment/OuijaBoard.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"

AOuijaBoard::AOuijaBoard()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	BoardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoardMesh"));
	BoardMesh->SetupAttachment(SceneRoot);
	BoardMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BoardMesh->SetCollisionResponseToAllChannels(ECR_Block);

	ArrowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrowMesh"));
	ArrowMesh->SetupAttachment(BoardMesh);
	ArrowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ArrowLocalLocation = FVector(0.0f, 0.0f, ArrowHeight);
	ArrowMesh->SetRelativeLocation(ArrowLocalLocation);

	LetterCollisionBoxes.Reserve(26);
	for (int32 LetterIndex = 0; LetterIndex < 26; ++LetterIndex)
	{
		const TCHAR Letter = static_cast<TCHAR>(TEXT('A') + LetterIndex);
		const FName ComponentName(*FString::Printf(TEXT("Letter_%c"), Letter));
		UBoxComponent* LetterBox = CreateDefaultSubobject<UBoxComponent>(ComponentName);
		LetterBox->SetupAttachment(BoardMesh);
		LetterBox->SetBoxExtent(FVector(3.5f, 5.0f, 2.0f));
		LetterBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		LetterBox->SetCollisionResponseToAllChannels(ECR_Ignore);

		// A-M form the upper row and N-Z the lower row. These are only useful
		// defaults; position every component over its matching board letter.
		const int32 Column = LetterIndex % 13;
		const int32 Row = LetterIndex / 13;
		LetterBox->SetRelativeLocation(FVector(-48.0f + Column * 8.0f, Row == 0 ? -8.0f : 8.0f, ArrowHeight));
		LetterCollisionBoxes.Add(LetterBox);
	}
}

void AOuijaBoard::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!ArrowMesh)
	{
		return;
	}

	const FVector Current = ArrowMesh->GetRelativeLocation();
	const FVector NewLocation = ArrowInterpSpeed <= 0.0f
		? ArrowLocalLocation
		: FMath::VInterpTo(Current, ArrowLocalLocation, DeltaSeconds, ArrowInterpSpeed);

	ArrowMesh->SetRelativeLocation(NewLocation);

	FString Letter;
	CheckArrowLetterCollision(Letter);
	DrawArrowCenterPoint();
}

void AOuijaBoard::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority())
	{
		return;
	}

	TraceBoardFromInteractor(Interactor);
}

bool AOuijaBoard::TraceBoardFromInteractor(AActor* Interactor)
{
	if (!HasAuthority() || !IsValid(Interactor) || !GetWorld())
	{
		return false;
	}

	FVector ViewLocation;
	FRotator ViewRotation;

	if (const APawn* Pawn = Cast<APawn>(Interactor))
	{
		if (const AController* Controller = Pawn->GetController())
		{
			Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);
		}
		else
		{
			Pawn->GetActorEyesViewPoint(ViewLocation, ViewRotation);
		}
	}
	else
	{
		Interactor->GetActorEyesViewPoint(ViewLocation, ViewRotation);
	}

	const FVector TraceEnd = ViewLocation + ViewRotation.Vector() * TraceDistance;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(OuijaBoardTrace), false, Interactor);
	FHitResult Hit;

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		ViewLocation,
		TraceEnd,
		ECC_Visibility,
		QueryParams);

	if (!bHit || Hit.GetActor() != this || Hit.GetComponent() != BoardMesh)
	{
		return false;
	}

	MoveArrowToWorldPoint(Hit.ImpactPoint);
	return true;
}

void AOuijaBoard::MoveArrowToWorldPoint(const FVector& WorldPoint)
{
	if (!HasAuthority())
	{
		return;
	}

	FVector LocalPoint = BoardMesh->GetComponentTransform().InverseTransformPosition(WorldPoint);
	LocalPoint.X = FMath::Clamp(LocalPoint.X, -BoardHalfExtents.X, BoardHalfExtents.X);
	LocalPoint.Y = FMath::Clamp(LocalPoint.Y, -BoardHalfExtents.Y, BoardHalfExtents.Y);
	LocalPoint.Z = ArrowHeight;

	ArrowLocalLocation = LocalPoint;
	ApplyArrowTarget(false);
	ForceNetUpdate();
}

FVector AOuijaBoard::GetArrowCenterWorldLocation() const
{
	return ArrowMesh ? ArrowMesh->Bounds.Origin : GetActorLocation();
}

bool AOuijaBoard::CheckArrowLetterCollision(FString& OutLetter)
{
	OutLetter.Reset();

	if (!ArrowMesh || !GetWorld())
	{
		return false;
	}

	const FVector ArrowCenter = GetArrowCenterWorldLocation();
	int32 OverlappingIndex = INDEX_NONE;

	for (int32 Index = 0; Index < LetterCollisionBoxes.Num(); ++Index)
	{
		const UBoxComponent* LetterBox = LetterCollisionBoxes[Index];
		if (!IsValid(LetterBox))
		{
			continue;
		}

		const FVector LocalPoint = LetterBox->GetComponentTransform().InverseTransformPosition(ArrowCenter);
		const FVector Extent = LetterBox->GetUnscaledBoxExtent();
		if (FMath::Abs(LocalPoint.X) <= Extent.X
			&& FMath::Abs(LocalPoint.Y) <= Extent.Y
			&& FMath::Abs(LocalPoint.Z) <= Extent.Z)
		{
			OverlappingIndex = Index;
			break;
		}
	}

	if (OverlappingIndex == INDEX_NONE)
	{
		CurrentLetterBoxIndex = INDEX_NONE;
		HoveredLetter.Reset();
		LetterReadTimeRemaining = 0.0f;
		bCurrentLetterAccepted = false;
		return false;
	}

	OutLetter = FString::Chr(TEXT('A') + OverlappingIndex);
	HoveredLetter = OutLetter;
	const double Now = GetWorld()->GetTimeSeconds();

	// Entering a new box starts a fresh dwell timer. Moving away before the
	// delay completes prevents that letter from being accepted.
	if (OverlappingIndex != CurrentLetterBoxIndex)
	{
		CurrentLetterBoxIndex = OverlappingIndex;
		LetterHoverStartTime = Now;
		LetterReadTimeRemaining = LetterDetectionDelay;
		bCurrentLetterAccepted = false;
		return true;
	}

	const double HoverDuration = Now - LetterHoverStartTime;
	LetterReadTimeRemaining = FMath::Max(0.0f, LetterDetectionDelay - static_cast<float>(HoverDuration));

	if (bCurrentLetterAccepted || HoverDuration < LetterDetectionDelay)
	{
		return true;
	}

	bCurrentLetterAccepted = true;
	if (!HasAuthority())
	{
		return true;
	}

	LastDetectedLetter = OutLetter;
	DetectedLetters.Append(LastDetectedLetter);
	AnnounceDetectedLetter();
	ForceNetUpdate();
	return true;
}

void AOuijaBoard::DrawArrowCenterPoint() const
{
	if (!bShowArrowCenterPoint || !GetWorld())
	{
		return;
	}

	const FVector Center = GetArrowCenterWorldLocation();
	const bool bWaiting = !HoveredLetter.IsEmpty() && LetterReadTimeRemaining > 0.0f;
	const FColor PointColor = HoveredLetter.IsEmpty()
		? FColor::Red
		: (bWaiting ? FColor::Yellow : FColor::Green);

	DrawDebugSphere(
		GetWorld(),
		Center,
		ArrowCenterPointRadius,
		12,
		PointColor,
		false,
		0.0f,
		0,
		1.5f);

	const FString PointText = HoveredLetter.IsEmpty()
		? TEXT("No letter")
		: (bWaiting
			? FString::Printf(TEXT("%s - reading in %.1fs"), *HoveredLetter, LetterReadTimeRemaining)
			: FString::Printf(TEXT("%s - read"), *HoveredLetter));

	DrawDebugString(
		GetWorld(),
		Center + FVector(0.0f, 0.0f, ArrowStatusTextHeight),
		PointText,
		nullptr,
		PointColor,
		0.0f,
		true,
		1.0f);
}

void AOuijaBoard::OnRep_ArrowLocalLocation()
{
	ApplyArrowTarget(false);
}

void AOuijaBoard::OnRep_LastDetectedLetter()
{
	AnnounceDetectedLetter();
}

void AOuijaBoard::AnnounceDetectedLetter()
{
	if (LastDetectedLetter.IsEmpty())
	{
		return;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			1.5f,
			FColor::Cyan,
			FString::Printf(TEXT("Ouija letter: %s"), *LastDetectedLetter));
	}

	BP_OnLetterDetected(LastDetectedLetter);
}

void AOuijaBoard::ApplyArrowTarget(bool bSnap)
{
	if (bSnap && ArrowMesh)
	{
		ArrowMesh->SetRelativeLocation(ArrowLocalLocation);
	}

	BP_OnArrowTargetChanged(ArrowLocalLocation);
}

void AOuijaBoard::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AOuijaBoard, ArrowLocalLocation);
	DOREPLIFETIME(AOuijaBoard, LastDetectedLetter);
	DOREPLIFETIME(AOuijaBoard, DetectedLetters);
}
