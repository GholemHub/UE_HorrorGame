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

	CancelCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Cancel"));
	CancelCollisionBox->SetupAttachment(BoardMesh);
	CancelCollisionBox->SetBoxExtent(FVector(8.0f, 5.0f, 2.0f));
	CancelCollisionBox->SetRelativeLocation(FVector(-22.0f, 23.0f, ArrowHeight));
	CancelCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CancelCollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);

	EnterCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Enter"));
	EnterCollisionBox->SetupAttachment(BoardMesh);
	EnterCollisionBox->SetBoxExtent(FVector(8.0f, 5.0f, 2.0f));
	EnterCollisionBox->SetRelativeLocation(FVector(22.0f, 23.0f, ArrowHeight));
	EnterCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	EnterCollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
}

void AOuijaBoard::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ArrowStartPosition.X = FMath::Clamp(ArrowStartPosition.X, -BoardHalfExtents.X, BoardHalfExtents.X);
	ArrowStartPosition.Y = FMath::Clamp(ArrowStartPosition.Y, -BoardHalfExtents.Y, BoardHalfExtents.Y);
	ArrowLocalLocation = ArrowStartPosition;

	if (ArrowMesh)
	{
		ArrowMesh->SetRelativeLocation(ArrowLocalLocation);
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
	if (!HasAuthority() || bIsAutomaticallyTyping)
	{
		return;
	}

	TraceBoardFromInteractor(Interactor);
}

bool AOuijaBoard::TraceBoardFromInteractor(AActor* Interactor)
{
	if (!HasAuthority() || bIsAutomaticallyTyping || !IsValid(Interactor) || !GetWorld())
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
	if (!HasAuthority() || bIsAutomaticallyTyping)
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
	const int32 OverlappingIndex = FindHoveredInputBox(ArrowCenter);

	if (OverlappingIndex == INDEX_NONE)
	{
		ResetHoveredInputState();
		return false;
	}

	const bool bIsLetter = OverlappingIndex < LetterCollisionBoxes.Num();
	const FString InputLabel = bIsLetter
		? FString::Chr(TEXT('A') + OverlappingIndex)
		: (OverlappingIndex == LetterCollisionBoxes.Num() ? TEXT("CANCEL") : TEXT("ENTER"));
	// Return button names too so Blueprint/debug consumers can see that the
	// arrow is currently inside the Cancel or Enter box.
	OutLetter = InputLabel;
	HoveredLetter = InputLabel;
	bHoveringTextButton = !bIsLetter;
	const float RequiredDetectionDelay = bIsLetter ? LetterDetectionDelay : ButtonDetectionDelay;
	const double Now = GetWorld()->GetTimeSeconds();

	// Entering a new box starts a fresh dwell timer. Moving away before the
	// delay completes prevents that letter from being accepted.
	if (OverlappingIndex != CurrentLetterBoxIndex)
	{
		CurrentLetterBoxIndex = OverlappingIndex;
		LetterHoverStartTime = Now;
		LetterReadTimeRemaining = RequiredDetectionDelay;
		bCurrentLetterAccepted = false;
		return true;
	}

	const double HoverDuration = Now - LetterHoverStartTime;
	LetterReadTimeRemaining = FMath::Max(0.0f, RequiredDetectionDelay - static_cast<float>(HoverDuration));

	if (bCurrentLetterAccepted || HoverDuration < RequiredDetectionDelay)
	{
		return true;
	}

	bCurrentLetterAccepted = true;
	if (!HasAuthority())
	{
		return true;
	}

	AcceptHoveredInput(OverlappingIndex, InputLabel);
	ForceNetUpdate();
	return true;
}

int32 AOuijaBoard::FindHoveredInputBox(const FVector& ArrowCenter) const
{
	auto ContainsPoint = [&ArrowCenter](const UBoxComponent* Box)
	{
		if (!IsValid(Box))
		{
			return false;
		}
		const FVector LocalPoint = Box->GetComponentTransform().InverseTransformPosition(ArrowCenter);
		const FVector Extent = Box->GetUnscaledBoxExtent();
		return FMath::Abs(LocalPoint.X) <= Extent.X
			&& FMath::Abs(LocalPoint.Y) <= Extent.Y
			&& FMath::Abs(LocalPoint.Z) <= Extent.Z;
	};

	for (int32 Index = 0; Index < LetterCollisionBoxes.Num(); ++Index)
	{
		if (ContainsPoint(LetterCollisionBoxes[Index]))
		{
			return Index;
		}
	}
	if (ContainsPoint(CancelCollisionBox))
	{
		return LetterCollisionBoxes.Num();
	}
	if (ContainsPoint(EnterCollisionBox))
	{
		return LetterCollisionBoxes.Num() + 1;
	}
	return INDEX_NONE;
}

void AOuijaBoard::AcceptHoveredInput(int32 InputBoxIndex, const FString& InputLabel)
{
	if (InputBoxIndex < LetterCollisionBoxes.Num())
	{
		LastDetectedLetter = InputLabel;
		DetectedLetters.Append(LastDetectedLetter);
		TypedText.Append(LastDetectedLetter);
		AnnounceDetectedLetter();
		BP_OnTypedTextChanged(TypedText);
		OnNewLetterTyped.Broadcast(LastDetectedLetter, TypedText);
		HandleAutomaticLetterAccepted(InputLabel);
	}
	else if (InputBoxIndex == LetterCollisionBoxes.Num())
	{
		CancelTypedText();
	}
	else
	{
		const bool bCompletingAutomaticTyping = bIsAutomaticallyTyping
			&& bAutomaticTypingIsSubmitting;
		const FString CompletedAutomaticWord = AutomaticTypingWord;
		if (bCompletingAutomaticTyping)
		{
			ClearAutomaticTypingState();
		}

		EnterTypedText();

		if (bCompletingAutomaticTyping)
		{
			BroadcastAutomaticTypingFinished(CompletedAutomaticWord);
		}
	}
}

void AOuijaBoard::CancelTypedText()
{
	if (!HasAuthority())
	{
		return;
	}

	ClearAutomaticTypingState();
	TypedText.Reset();
	DetectedLetters.Reset();
	LastDetectedLetter.Reset();
	BP_OnTypedTextChanged(TypedText);
	BP_OnTextCancelled();
	ForceNetUpdate();
}

FString AOuijaBoard::EnterTypedText()
{
	if (!HasAuthority())
	{
		return TypedText;
	}

	const FString EnteredText = TypedText;
	BP_OnTextEntered(EnteredText);

	if (!IsTypedWordCorrect())
	{
		OnRequiredWordRejected.Broadcast(EnteredText);
		BP_OnIncorrectWordEntered(EnteredText);
		CancelTypedText();
		return EnteredText;
	}

	if (!bRequiredWordAccepted || !bAcceptRequiredWordOnlyOnce)
	{
		bRequiredWordAccepted = true;
		OnRequiredWordAccepted.Broadcast(EnteredText);
		BP_OnCorrectWordEntered(EnteredText);
		ForceNetUpdate();
	}

	return EnteredText;
}

bool AOuijaBoard::IsTypedWordCorrect() const
{
	const FString NormalizedRequiredWord = NormalizePuzzleWord(RequiredWord);
	if (NormalizedRequiredWord.IsEmpty())
	{
		return false;
	}

	const FString NormalizedTypedText = NormalizePuzzleWord(TypedText);
	const ESearchCase::Type SearchCase = bIgnoreRequiredWordCase
		? ESearchCase::IgnoreCase
		: ESearchCase::CaseSensitive;
	return NormalizedTypedText.Equals(NormalizedRequiredWord, SearchCase);
}

void AOuijaBoard::ResetWordPuzzle(bool bClearTypedText)
{
	if (!HasAuthority())
	{
		return;
	}

	bRequiredWordAccepted = false;
	if (bClearTypedText)
	{
		CancelTypedText();
	}
	else
	{
		ForceNetUpdate();
	}
}

bool AOuijaBoard::TypeWordAutomatically(
	const FString& Word,
	bool bClearExistingText,
	bool bPressEnterWhenFinished)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[OuijaBoard] Type Word Automatically ignored for %s because it was not called on the server"),
			*GetName());
		return false;
	}

	FString SanitizedWord;
	if (!SanitizeAutomaticWord(Word, SanitizedWord))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[OuijaBoard] %s cannot type '%s'. Only letters A-Z and spaces are supported"),
			*GetName(),
			*Word);
		return false;
	}

	ClearAutomaticTypingState();
	if (bClearExistingText)
	{
		CancelTypedText();
	}

	AutomaticTypingWord = SanitizedWord;
	AutomaticTypingLetterIndex = 0;
	bAutomaticTypingShouldPressEnter = bPressEnterWhenFinished;
	bAutomaticTypingIsSubmitting = false;
	bIsAutomaticallyTyping = true;
	ResetHoveredInputState();
	MoveArrowToCurrentAutomaticLetter();

	OnAutomaticTypingStarted.Broadcast(AutomaticTypingWord);
	BP_OnAutomaticTypingStarted(AutomaticTypingWord);

	UE_LOG(LogTemp, Log,
		TEXT("[OuijaBoard] %s started automatically typing '%s'"),
		*GetName(),
		*AutomaticTypingWord);
	return true;
}

void AOuijaBoard::CancelAutomaticTyping(bool bClearTypedText)
{
	if (!HasAuthority())
	{
		return;
	}

	ClearAutomaticTypingState();
	if (bClearTypedText)
	{
		CancelTypedText();
	}
	else
	{
		ForceNetUpdate();
	}
}

FString AOuijaBoard::NormalizePuzzleWord(const FString& Word) const
{
	FString NormalizedWord = Word;
	if (bTrimRequiredWordWhitespace)
	{
		NormalizedWord.TrimStartAndEndInline();
	}
	return NormalizedWord;
}

bool AOuijaBoard::SanitizeAutomaticWord(const FString& Word, FString& OutSanitizedWord) const
{
	OutSanitizedWord.Reset();
	const FString UpperWord = Word.ToUpper();
	for (const TCHAR Character : UpperWord)
	{
		if (Character >= TEXT('A') && Character <= TEXT('Z'))
		{
			OutSanitizedWord.AppendChar(Character);
		}
		else if (!FChar::IsWhitespace(Character))
		{
			OutSanitizedWord.Reset();
			return false;
		}
	}

	return !OutSanitizedWord.IsEmpty();
}

void AOuijaBoard::MoveArrowToInputBox(const UBoxComponent* InputBox)
{
	if (!HasAuthority() || !IsValid(InputBox) || !IsValid(ArrowMesh) || !IsValid(BoardMesh))
	{
		return;
	}

	// CheckArrowLetterCollision tests ArrowMesh bounds rather than its pivot. The
	// offset correction keeps meshes with an off-centre pivot over the box too.
	const FVector ArrowBoundsOffset = ArrowMesh->Bounds.Origin - ArrowMesh->GetComponentLocation();
	const FVector DesiredArrowComponentLocation = InputBox->GetComponentLocation() - ArrowBoundsOffset;
	FVector DesiredLocalLocation = BoardMesh->GetComponentTransform().InverseTransformPosition(
		DesiredArrowComponentLocation);
	DesiredLocalLocation.X = FMath::Clamp(
		DesiredLocalLocation.X,
		-BoardHalfExtents.X,
		BoardHalfExtents.X);
	DesiredLocalLocation.Y = FMath::Clamp(
		DesiredLocalLocation.Y,
		-BoardHalfExtents.Y,
		BoardHalfExtents.Y);
	DesiredLocalLocation.Z = ArrowHeight;

	ArrowLocalLocation = DesiredLocalLocation;
	ApplyArrowTarget(false);
	ForceNetUpdate();
}

void AOuijaBoard::MoveArrowToCurrentAutomaticLetter()
{
	if (!bIsAutomaticallyTyping
		|| bAutomaticTypingIsSubmitting
		|| !AutomaticTypingWord.IsValidIndex(AutomaticTypingLetterIndex))
	{
		return;
	}

	const TCHAR Letter = AutomaticTypingWord[AutomaticTypingLetterIndex];
	const int32 LetterIndex = static_cast<int32>(Letter - TEXT('A'));
	if (!LetterCollisionBoxes.IsValidIndex(LetterIndex))
	{
		CancelAutomaticTyping(false);
		return;
	}

	MoveArrowToInputBox(LetterCollisionBoxes[LetterIndex]);
}

void AOuijaBoard::HandleAutomaticLetterAccepted(const FString& Letter)
{
	if (!bIsAutomaticallyTyping
		|| bAutomaticTypingIsSubmitting
		|| !AutomaticTypingWord.IsValidIndex(AutomaticTypingLetterIndex))
	{
		return;
	}

	const FString ExpectedLetter = AutomaticTypingWord.Mid(AutomaticTypingLetterIndex, 1);
	if (!Letter.Equals(ExpectedLetter, ESearchCase::CaseSensitive))
	{
		return;
	}

	++AutomaticTypingLetterIndex;
	ResetHoveredInputState();

	if (AutomaticTypingWord.IsValidIndex(AutomaticTypingLetterIndex))
	{
		MoveArrowToCurrentAutomaticLetter();
		return;
	}

	if (bAutomaticTypingShouldPressEnter)
	{
		bAutomaticTypingIsSubmitting = true;
		MoveArrowToInputBox(EnterCollisionBox);
		return;
	}

	const FString CompletedWord = AutomaticTypingWord;
	ClearAutomaticTypingState();
	ForceNetUpdate();
	BroadcastAutomaticTypingFinished(CompletedWord);
}

void AOuijaBoard::ClearAutomaticTypingState()
{
	bIsAutomaticallyTyping = false;
	AutomaticTypingWord.Reset();
	AutomaticTypingLetterIndex = INDEX_NONE;
	bAutomaticTypingShouldPressEnter = true;
	bAutomaticTypingIsSubmitting = false;
}

void AOuijaBoard::BroadcastAutomaticTypingFinished(const FString& CompletedWord)
{
	OnAutomaticTypingFinished.Broadcast(CompletedWord);
	BP_OnAutomaticTypingFinished(CompletedWord);

	UE_LOG(LogTemp, Log,
		TEXT("[OuijaBoard] %s finished automatically typing '%s'"),
		*GetName(),
		*CompletedWord);
}

void AOuijaBoard::ResetHoveredInputState()
{
	CurrentLetterBoxIndex = INDEX_NONE;
	HoveredLetter.Reset();
	LetterReadTimeRemaining = 0.0f;
	bHoveringTextButton = false;
	bCurrentLetterAccepted = false;
}

void AOuijaBoard::DrawArrowCenterPoint() const
{
	if (!bEnableDebugLogs || !bShowArrowCenterPoint || !GetWorld())
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

	FString PointText;
	if (HoveredLetter.IsEmpty())
	{
		PointText = TEXT("No letter or button");
	}
	else if (bHoveringTextButton)
	{
		PointText = bWaiting
			? FString::Printf(TEXT("Button %s - activating in %.1fs"), *HoveredLetter, LetterReadTimeRemaining)
			: FString::Printf(TEXT("Button %s - activated"), *HoveredLetter);
	}
	else
	{
		PointText = bWaiting
			? FString::Printf(TEXT("%s - reading in %.1fs"), *HoveredLetter, LetterReadTimeRemaining)
			: FString::Printf(TEXT("%s - read"), *HoveredLetter);
	}

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

void AOuijaBoard::OnRep_TypedText()
{
	BP_OnTypedTextChanged(TypedText);

	if (!TypedText.IsEmpty())
	{
		OnNewLetterTyped.Broadcast(TypedText.Right(1), TypedText);
	}
}

void AOuijaBoard::AnnounceDetectedLetter()
{
	if (LastDetectedLetter.IsEmpty())
	{
		return;
	}

	if (bEnableDebugLogs && GEngine)
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
	DOREPLIFETIME(AOuijaBoard, TypedText);
	DOREPLIFETIME(AOuijaBoard, bRequiredWordAccepted);
	DOREPLIFETIME(AOuijaBoard, bIsAutomaticallyTyping);
	DOREPLIFETIME(AOuijaBoard, AutomaticTypingWord);
	DOREPLIFETIME(AOuijaBoard, AutomaticTypingLetterIndex);
}
