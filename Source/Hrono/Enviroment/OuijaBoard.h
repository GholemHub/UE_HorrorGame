#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/Enviroment_Interface.h"
#include "OuijaBoard.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UBoxComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnNewLetterTyped,
	const FString&, NewLetter,
	const FString&, FullTypedText);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnOuijaWordAccepted,
	const FString&, CorrectWord);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnOuijaWordRejected,
	const FString&, IncorrectWord);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnOuijaAutomaticTypingEvent,
	const FString&, Word);

/**
 * An interactable Ouija board whose planchette moves to the point looked at by
 * the interacting character.
 *
 * The board mesh is expected to lie in its local XY plane. ArrowHeight keeps
 * the arrow above the board, while BoardHalfExtents constrains its X/Y motion.
 */
UCLASS(Blueprintable)
class HRONO_API AOuijaBoard : public AActor, public IEnviroment_Interface
{
	GENERATED_BODY()

public:
	AOuijaBoard();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;

	/** Moves the arrow from an already-known world-space point. */
	UFUNCTION(BlueprintCallable, Category = "Ouija")
	void MoveArrowToWorldPoint(const FVector& WorldPoint);

	/** Traces from Interactor's view and moves the arrow if the board is hit. */
	UFUNCTION(BlueprintCallable, Category = "Ouija")
	bool TraceBoardFromInteractor(AActor* Interactor);

	/** Returns the center of the arrow mesh bounds in world space. */
	UFUNCTION(BlueprintPure, Category = "Ouija|Letters")
	FVector GetArrowCenterWorldLocation() const;

	/**
	 * Checks the arrow center against Letter_A through Letter_Z.
	 * Returns true when the center is inside a letter box and outputs that letter.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ouija|Letters")
	bool CheckArrowLetterCollision(FString& OutLetter);

	/** Clears all text typed on the board. */
	UFUNCTION(BlueprintCallable, Category = "Ouija|Text")
	void CancelTypedText();

	/** Accepts the current text, notifies Blueprint listeners, and returns it. */
	UFUNCTION(BlueprintCallable, Category = "Ouija|Text")
	FString EnterTypedText();

	/** Returns true when the current TypedText matches RequiredWord. */
	UFUNCTION(BlueprintPure, Category = "Ouija|Word Puzzle")
	bool IsTypedWordCorrect() const;

	/** Allows the same board puzzle to be completed again. Server only. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Ouija|Word Puzzle")
	void ResetWordPuzzle(bool bClearTypedText = true);

	/**
	 * Moves the planchette over every letter in Word and types it using the normal
	 * letter dwell logic. Spaces are ignored; other unsupported characters reject
	 * the request. When bPressEnterWhenFinished is true, the planchette also moves
	 * onto Enter and submits the resulting TypedText.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Ouija|Automatic Typing",
		meta = (DisplayName = "Type Word Automatically"))
	bool TypeWordAutomatically(
		const FString& Word,
		bool bClearExistingText = true,
		bool bPressEnterWhenFinished = true);

	/** Stops the active automatic sequence. It does not move the planchette. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Ouija|Automatic Typing")
	void CancelAutomaticTyping(bool bClearTypedText = false);

	UFUNCTION(BlueprintPure, Category = "Ouija|Automatic Typing")
	bool IsAutomaticallyTyping() const { return bIsAutomaticallyTyping; }

	/**
	 * Event dispatcher broadcast whenever a new letter is appended.
	 * Bind a Blueprint Custom Event to this dispatcher to update UI.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Ouija|Text")
	FOnNewLetterTyped OnNewLetterTyped;

	/**
	 * Fired on the server when Enter is pressed with the correct word.
	 * Bind this from another Blueprint when the ritual logic lives outside the board.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Ouija|Word Puzzle")
	FOnOuijaWordAccepted OnRequiredWordAccepted;

	/** Fired before incorrect text is automatically cleared. */
	UPROPERTY(BlueprintAssignable, Category = "Ouija|Word Puzzle")
	FOnOuijaWordRejected OnRequiredWordRejected;

	UPROPERTY(BlueprintAssignable, Category = "Ouija|Automatic Typing")
	FOnOuijaAutomaticTypingEvent OnAutomaticTypingStarted;

	/** Fired after all letters and the optional Enter step have completed. */
	UPROPERTY(BlueprintAssignable, Category = "Ouija|Automatic Typing")
	FOnOuijaAutomaticTypingEvent OnAutomaticTypingFinished;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ouija|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ouija|Components")
	TObjectPtr<UStaticMeshComponent> BoardMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ouija|Components")
	TObjectPtr<UStaticMeshComponent> ArrowMesh;

	/** Twenty-six editable trigger regions, ordered A through Z. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ouija|Components")
	TArray<TObjectPtr<UBoxComponent>> LetterCollisionBoxes;

	/** Trigger region that clears TypedText after the dwell delay. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ouija|Components")
	TObjectPtr<UBoxComponent> CancelCollisionBox;

	/** Trigger region that submits TypedText after the dwell delay. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ouija|Components")
	TObjectPtr<UBoxComponent> EnterCollisionBox;

	/** Maximum trace length measured from the character's view. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ouija|Trace", meta = (ClampMin = "1.0"))
	float TraceDistance = 500.0f;

	/** Movement limits in BoardMesh-local X and Y (before component scale). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ouija|Movement", meta = (ClampMin = "0.0"))
	FVector2D BoardHalfExtents = FVector2D(50.0f, 30.0f);

	/** Initial arrow location relative to BoardMesh. Editable in Blueprint defaults and instances. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ouija|Movement")
	FVector ArrowStartPosition = FVector(0.0f, 0.0f, 2.0f);

	/** Arrow position above the board in local Z. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ouija|Movement")
	float ArrowHeight = 2.0f;

	/** Zero snaps immediately; positive values smoothly approach the target. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ouija|Movement", meta = (ClampMin = "0.0"))
	float ArrowInterpSpeed = 10.0f;

	/** Time the arrow must remain on one letter before it is accepted. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ouija|Letters", meta = (ClampMin = "0.0", Units = "s"))
	float LetterDetectionDelay = 2.0f;

	/** Time the arrow must remain on Cancel or Enter before the button is activated. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ouija|Text", meta = (ClampMin = "0.0", Units = "s"))
	float ButtonDetectionDelay = 2.0f;

	/** Word that must be entered before the new success event is fired. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ouija|Word Puzzle")
	FString RequiredWord;

	/** Allows "RITUAL" and "ritual" to be treated as the same answer. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ouija|Word Puzzle")
	bool bIgnoreRequiredWordCase = true;

	/** Removes spaces at the beginning and end before comparing the words. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ouija|Word Puzzle")
	bool bTrimRequiredWordWhitespace = true;

	/** Prevents repeated Enter presses from firing the success event more than once. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ouija|Word Puzzle")
	bool bAcceptRequiredWordOnlyOnce = true;

	/** Master switch for every Ouija Board debug message and debug drawing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ouija|Debug")
	bool bEnableDebugLogs = false;

	/** Draws the tested arrow center and pending letter during play. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ouija|Debug",
		meta = (EditCondition = "bEnableDebugLogs"))
	bool bShowArrowCenterPoint = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ouija|Debug",
		meta = (ClampMin = "0.1", EditCondition = "bEnableDebugLogs && bShowArrowCenterPoint"))
	float ArrowCenterPointRadius = 2.0f;

	/** Vertical distance from the arrow center to the status text. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ouija|Debug",
		meta = (EditCondition = "bEnableDebugLogs && bShowArrowCenterPoint"))
	float ArrowStatusTextHeight = 20.0f;

	/** Letter currently underneath the arrow. It is not accepted until the delay completes. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Ouija|Letters")
	FString HoveredLetter;

	/** Remaining dwell time before HoveredLetter is accepted. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Ouija|Letters")
	float LetterReadTimeRemaining = 0.0f;

	/** True while the arrow is over the Cancel or Enter collision box. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Ouija|Text")
	bool bHoveringTextButton = false;

	/** Most recently accepted letter. Empty until the first detection. */
	UPROPERTY(ReplicatedUsing = OnRep_LastDetectedLetter, VisibleInstanceOnly, BlueprintReadOnly, Category = "Ouija|Letters")
	FString LastDetectedLetter;

	/** All accepted letters in detection order. */
	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Ouija|Letters")
	FString DetectedLetters;

	/** Text assembled from accepted letters. Cancel clears it and Enter submits it. */
	UPROPERTY(ReplicatedUsing = OnRep_TypedText, VisibleInstanceOnly, BlueprintReadOnly, Category = "Ouija|Text")
	FString TypedText;

	/** Replicated result that can be queried by UI and other Blueprint actors. */
	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Ouija|Word Puzzle")
	bool bRequiredWordAccepted = false;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Ouija|Automatic Typing")
	bool bIsAutomaticallyTyping = false;

	/** Sanitized A-Z word currently being written. */
	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Ouija|Automatic Typing")
	FString AutomaticTypingWord;

	/** Index of the letter currently targeted by the planchette. */
	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Ouija|Automatic Typing")
	int32 AutomaticTypingLetterIndex = INDEX_NONE;

	UPROPERTY(ReplicatedUsing = OnRep_ArrowLocalLocation, VisibleInstanceOnly, BlueprintReadOnly, Category = "Ouija")
	FVector ArrowLocalLocation = FVector::ZeroVector;

	UFUNCTION()
	void OnRep_ArrowLocalLocation();

	UFUNCTION()
	void OnRep_LastDetectedLetter();

	UFUNCTION()
	void OnRep_TypedText();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ouija", meta = (DisplayName = "On Arrow Target Changed"))
	void BP_OnArrowTargetChanged(FVector NewLocalLocation);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ouija|Letters", meta = (DisplayName = "On Letter Detected"))
	void BP_OnLetterDetected(const FString& Letter);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ouija|Text", meta = (DisplayName = "On Typed Text Changed"))
	void BP_OnTypedTextChanged(const FString& NewTypedText);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ouija|Text", meta = (DisplayName = "On Text Cancelled"))
	void BP_OnTextCancelled();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ouija|Text", meta = (DisplayName = "On Text Entered"))
	void BP_OnTextEntered(const FString& EnteredText);

	/** Automatic server-side Blueprint event fired after the word is validated. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ouija|Word Puzzle",
		meta = (DisplayName = "On Correct Word Entered"))
	void BP_OnCorrectWordEntered(const FString& CorrectWord);

	/** Automatic server-side Blueprint event fired before incorrect text is cleared. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ouija|Word Puzzle",
		meta = (DisplayName = "On Incorrect Word Entered"))
	void BP_OnIncorrectWordEntered(const FString& IncorrectWord);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ouija|Automatic Typing",
		meta = (DisplayName = "On Automatic Typing Started"))
	void BP_OnAutomaticTypingStarted(const FString& Word);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ouija|Automatic Typing",
		meta = (DisplayName = "On Automatic Typing Finished"))
	void BP_OnAutomaticTypingFinished(const FString& Word);

private:
	void ApplyArrowTarget(bool bSnap);
	void AnnounceDetectedLetter();
	void DrawArrowCenterPoint() const;
	int32 FindHoveredInputBox(const FVector& ArrowCenter) const;
	void AcceptHoveredInput(int32 InputBoxIndex, const FString& InputLabel);
	FString NormalizePuzzleWord(const FString& Word) const;
	bool SanitizeAutomaticWord(const FString& Word, FString& OutSanitizedWord) const;
	void MoveArrowToInputBox(const UBoxComponent* InputBox);
	void MoveArrowToCurrentAutomaticLetter();
	void HandleAutomaticLetterAccepted(const FString& Letter);
	void ClearAutomaticTypingState();
	void BroadcastAutomaticTypingFinished(const FString& CompletedWord);
	void ResetHoveredInputState();

	double LetterHoverStartTime = 0.0;
	int32 CurrentLetterBoxIndex = INDEX_NONE;
	bool bCurrentLetterAccepted = false;
	bool bAutomaticTypingShouldPressEnter = true;
	bool bAutomaticTypingIsSubmitting = false;
};
