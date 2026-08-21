#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "HronoSharedTools.h"
#include "HronoCollisionChannels.h"
#include "HronoCharacter.generated.h"


class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UDrag_Component;
class ADrag_Item;
class USpotLightComponent;
class AChair;
class USoundBase;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UPrimitiveComponent;
class AHronoCharacter;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUpdateSprintMeterDelegate, float, Percentage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSprintStateChangedDelegate, bool, bSprinting);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FStaminaChangedDelegate,
	float, CurrentStamina,
	float, MaxStamina,
	float, Percentage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHidingSafetyChangedDelegate, bool, bIsSafe);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FHidingWardrobeSafetyLostDelegate,
	AHronoCharacter*, ExposedPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FCharacterTimelineChangedDelegate,
	EItemTimeline, PreviousTimeline,
	EItemTimeline, NewTimeline);



UCLASS(abstract)
class AHronoCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: first person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpotLightComponent* SpotLight;
	
	
protected:

	void UpdateChairState(AChair* PreviousChair);

	void HandleSitStarted(AChair* Chair);
	void HandleSitEnded(AChair* Chair);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* CrouchAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* MouseLookAction;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* DropAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* StandAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* DragAction;

public:
	AHronoCharacter();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dream")
	bool bIsDreamCharacter = false;

	/** Legacy percentage-only sprint meter event. */
	UPROPERTY(BlueprintAssignable, Category = "Movement|Sprint")
	FUpdateSprintMeterDelegate OnSprintMeterUpdated;

	/** Fired whenever authoritative sprinting starts or stops. */
	UPROPERTY(BlueprintAssignable, Category = "Movement|Sprint")
	FSprintStateChangedDelegate OnSprintStateChanged;

	/** Fired whenever replicated stamina changes. Provides raw and normalized values. */
	UPROPERTY(BlueprintAssignable, Category = "Movement|Sprint")
	FStaminaChangedDelegate OnStaminaChanged;

	UFUNCTION(BlueprintPure, Category = "Movement|Sprint")
	float GetCurrentStamina() const { return CurrentStamina; }

	UFUNCTION(BlueprintPure, Category = "Movement|Sprint")
	float GetMaxStamina() const { return MaxStamina; }

	UFUNCTION(BlueprintPure, Category = "Movement|Sprint")
	float GetStaminaPercentage() const
	{
		return MaxStamina > KINDA_SMALL_NUMBER
			? FMath::Clamp(CurrentStamina / MaxStamina, 0.0f, 1.0f)
			: 0.0f;
	}

	UFUNCTION(BlueprintPure, Category = "Movement|Sprint")
	bool IsSprinting() const { return bSprinting; }

	UFUNCTION(BlueprintPure, Category = "Movement|Sprint")
	bool IsRecoveringStamina() const { return bRecovering; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> InteractionPoint;

	/** Alternate held-item attachment used while this character is in the Past timeline. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Interaction")
	TObjectPtr<USceneComponent> PastInteractionPoint;

	/** Returns the held-item attachment point selected by CharacterTimeline. */
	UFUNCTION(BlueprintPure, Category = "Interaction|Timeline")
	USceneComponent* GetActiveInteractionPoint() const;

	// =========================================================
	// AUDIO (placeholder sounds — assign any sound in Blueprint)
	// =========================================================

	/** Played for each footstep. Call PlayFootstepSound() from an animation notify. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> FootstepSound;

	/** Played when the character jumps. Assign any sound in Blueprint. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> JumpSound;

	/** Played when the character lands. Assign any sound in Blueprint. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> LandSound;

	/** Played when the character interacts with something. Assign any sound in Blueprint. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> InteractSound;

	/** Plays the footstep sound at the character's feet. Hook this up to an animation notify. */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayFootstepSound();

	/** Called by the engine when the character lands after a fall. */
	virtual void Landed(const FHitResult& Hit) override;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_CharacterTimeline, Category = "Timeline")
	EItemTimeline CharacterTimeline = EItemTimeline::Past;

	/** Fired after this character completes a replicated timeline change. */
	UPROPERTY(BlueprintAssignable, Category = "Timeline")
	FCharacterTimelineChangedDelegate OnCharacterTimelineChanged;

	/** Toggles Past <-> Future. Client calls are safely routed to the server. */
	UFUNCTION(BlueprintCallable, Category = "Timeline", meta = (DisplayName = "Switch Player Timeline"))
	void SwitchPlayerTimeline();

	/** Moves the player and the held item to a specific timeline. */
	UFUNCTION(BlueprintCallable, Category = "Timeline", meta = (DisplayName = "Set Player Timeline"))
	void SetPlayerTimeline(EItemTimeline NewTimeline);

	/** Immediately refreshes local visibility for every Base_Item timeline actor. */
	UFUNCTION(BlueprintCallable, Category = "Timeline")
	void RefreshTimelineVisibilityForLocalPlayer();

	/** Prevents a multicast death event from toggling the same player twice in one frame. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Timeline",
		meta = (ClampMin = "0.0", Units = "s"))
	float TimelineSwitchDuplicateGuardSeconds = 0.25f;

	// =========================================================
	// MIRRORED PAST VIEW
	// =========================================================

	/** Post-process material containing the scalar parameter named by
	 *  MirrorParameterName. Assign M_PP_MirrorPast in HE_CharacterHrono1. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timeline|Mirror")
	TObjectPtr<UMaterialInterface> MirrorPostProcessMaterial;

	/** Name of the scalar parameter that blends normal UVs (0) and mirrored UVs (1). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timeline|Mirror")
	FName MirrorParameterName = TEXT("Mirrored");

	/** Correct horizontal mouse-look while the final camera image is mirrored. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Timeline|Mirror|Input")
	bool bCorrectLookInputWhenMirrored = true;

	/** Correct A/D movement while the final camera image is mirrored. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Timeline|Mirror|Input")
	bool bCorrectMoveInputWhenMirrored = true;

	/** Correct horizontal mouse input used to drag doors and cupboards. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Timeline|Mirror|Input")
	bool bCorrectDragInputWhenMirrored = true;

	/** Current value sent to the material's Mirrored parameter. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Timeline|Mirror")
	float MirrorAmount = 0.0f;

	/** True only when the local camera has a valid mirror material and the effect is on. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Timeline|Mirror")
	bool bMirrorViewActive = false;

	/** One-call Blueprint API: 1 enables the effect and corrected controls; 0 disables both. */
	UFUNCTION(BlueprintCallable, Category = "Timeline|Mirror", meta = (DisplayName = "Set Mirrored View Enabled"))
	void SetMirroredViewEnabled(bool bEnabled);

	/** Directly changes the material scalar parameter. Use 0 or 1 for gameplay. */
	UFUNCTION(BlueprintCallable, Category = "Timeline|Mirror", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	void SetMirrorAmount(float NewMirrorAmount);

	/** Optional manual helper. Automatic use happens only during an explicit timeline switch. */
	UFUNCTION(BlueprintCallable, Category = "Timeline|Mirror")
	void ApplyMirrorFromCharacterTimeline();

	UFUNCTION(BlueprintPure, Category = "Timeline|Mirror")
	bool IsMirroredViewEnabled() const { return bMirrorViewActive; }

	UFUNCTION(BlueprintPure, Category = "Timeline|Mirror")
	float GetMirrorAmount() const { return MirrorAmount; }

	/** Horizontal scale for screen-relative interactions: -1 while mirrored, otherwise +1. */
	UFUNCTION(BlueprintPure, Category = "Timeline|Mirror|Input")
	float GetMirroredHorizontalInputScale() const;

	/** Returns the single item currently held by this character. */
	UFUNCTION(BlueprintPure, Category = "Items")
	class ABase_Item* GetHeldItem() const;

	/** Releases the held item without destroying it.
	 *  Intended for item sockets such as pentagram rune slots. Server only. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Items")
	bool ReleaseHeldItemForPlacement(class ABase_Item* Item);

	/** True only while the character overlaps a HidingWardrobe safety volume and
	 *  both wardrobe doors are below that wardrobe's unsafe angle. */
	UPROPERTY(ReplicatedUsing = OnRep_IsSafeInHidingWardrobe, VisibleInstanceOnly,
		BlueprintReadOnly, Category = "Hiding|Safety")
	bool bIsSafeInHidingWardrobe = false;

	/** Fired on the server and clients whenever wardrobe safety changes. */
	UPROPERTY(BlueprintAssignable, Category = "Hiding|Safety")
	FHidingSafetyChangedDelegate OnHidingSafetyChanged;

	/** Fired once whenever this character transitions from wardrobe-safe to exposed.
	 *  Bind to this on the server to re-check hazards already overlapping the player. */
	UPROPERTY(BlueprintAssignable, Category = "Hiding|Safety")
	FHidingWardrobeSafetyLostDelegate OnHidingWardrobeSafetyLost;

	UFUNCTION(BlueprintPure, Category = "Hiding|Safety")
	bool IsSafeInHidingWardrobe() const { return bIsSafeInHidingWardrobe; }

	/** Authority-only setter used by HidingWardrobe safety volumes. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Hiding|Safety")
	void SetSafeInHidingWardrobe(bool bNewSafe);

protected:
	UFUNCTION()
	void OnRep_IsSafeInHidingWardrobe(bool bPreviousSafe);

	UPROPERTY(ReplicatedUsing = OnRep_Sprinting, VisibleInstanceOnly, BlueprintReadOnly,
		Category = "Movement|Sprint")
	bool bSprinting = false;

	UFUNCTION()
	void OnRep_Sprinting();

	UFUNCTION(Server, Reliable)
	void ServerSetSprinting(bool bNewSprint);

	/** True after stamina is depleted and until StaminaRecoveryThreshold is reached. */
	UPROPERTY(ReplicatedUsing = OnRep_StaminaRecovery, VisibleInstanceOnly, BlueprintReadOnly,
		Category = "Movement|Sprint")
	bool bRecovering = false;

	UFUNCTION()
	void OnRep_StaminaRecovery();

	/** Current server-authoritative stamina. */
	UPROPERTY(ReplicatedUsing = OnRep_StaminaData, VisibleInstanceOnly, BlueprintReadOnly,
		Category = "Movement|Sprint")
	float CurrentStamina = 0.0f;

	UFUNCTION()
	void OnRep_StaminaData();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Sprint|Speed",
		meta = (ClampMin = "0.0", Units = "cm/s"))
	float WalkSpeed = 250.0f;

	/** Starts sprinting behavior */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void DoStartSprint();

	/** Stops sprinting behavior */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void DoEndSprint();

	void UpdateStamina(float DeltaSeconds);
	void SetCurrentStamina(float NewStamina);
	void SetSprintingState(bool bNewSprint);
	void SetStaminaRecoveryState(bool bNewRecovering);
	void ApplySprintMovementSpeed();
	void BroadcastStaminaChanged();

	/** Called from Input Actions for movement input */
	void MoveInput(const FInputActionValue& Value);

	/** Called from Input Actions for looking input */
	void LookInput(const FInputActionValue& Value);

	/** Handles aim inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoAim(float Yaw, float Pitch);

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles jump start inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump end inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	UFUNCTION(BlueprintCallable, Category = "Input")
	void DoCrouchStart();

	UFUNCTION(BlueprintCallable, Category = "Input")
	void DoStandUp();

	/** Handles jump end inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void DoCrouchEnd();



	UPROPERTY(EditAnywhere, Category = "Input", meta = (ClampMin = 0, Units = "cm"))
	float InteractTraceDistance = 300.0f;

	UFUNCTION()
	void OnRep_CharacterTimeline(EItemTimeline PreviousTimeline);

	UFUNCTION()
	void OnRep_TimelineMirrorRequested();

	UFUNCTION(Server, Reliable)
	void ServerSetPlayerTimeline(EItemTimeline NewTimeline);

	/** Guaranteed owning-client presentation update; replicated properties remain the source of truth. */
	UFUNCTION(Client, Reliable)
	void ClientApplyTimelineMirror(EItemTimeline NewTimeline);

	void ApplyTimelineCollision();
	void ApplyPlayerTimelineOnAuthority(EItemTimeline NewTimeline);
	void MoveCarriedItemsToTimeline(EItemTimeline NewTimeline);
	void RefreshHeldItemsInteractionPoint();
	bool EnsureMirrorPostProcessInstance();

	/** Replicated mirror state kept in sync with CharacterTimeline: Past=true, Future=false. */
	UPROPERTY(ReplicatedUsing = OnRep_TimelineMirrorRequested)
	bool bTimelineMirrorRequested = false;

	double LastTimelineSwitchServerTime = -1.0;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> MirrorPostProcessInstance;

	/** Authored OwnerNoSee values temporarily overridden for the local raster view. */
	TMap<TWeakObjectPtr<UPrimitiveComponent>, bool> TimelinePrimitiveOwnerNoSeeStates;

	/** Local-only set used to avoid repeated timeline visibility log messages. */
	TSet<TWeakObjectPtr<AHronoCharacter>> TimelineHiddenCharacters;
	

	/** Maximum stamina. CurrentStamina is initialized to this value by the server. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_StaminaData,
		Category = "Movement|Sprint|Stamina", meta = (ClampMin = "1.0"))
	float MaxStamina = 100.0f;

	/** Stamina consumed per second while sprinting and actually moving. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Sprint|Stamina",
		meta = (ClampMin = "0.0"))
	float StaminaDrainRate = 20.0f;

	/** Stamina restored per second when it is not being consumed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Sprint|Stamina",
		meta = (ClampMin = "0.0"))
	float StaminaRegenerationRate = 15.0f;

	/** Delay after the last stamina consumption before regeneration begins. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Sprint|Stamina",
		meta = (ClampMin = "0.0", Units = "s"))
	float StaminaRegenerationDelay = 1.0f;

	/** Minimum stamina required when a new sprint input begins. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Sprint|Stamina",
		meta = (ClampMin = "0.0"))
	float MinimumStaminaToStartSprint = 10.0f;

	/** After reaching zero, sprint remains locked until stamina reaches this value. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Sprint|Stamina",
		meta = (ClampMin = "0.0"))
	float StaminaRecoveryThreshold = 25.0f;

	/** Walk speed while sprinting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Sprint|Speed",
		meta = (ClampMin = "0.0", Units = "cm/s"))
	float SprintSpeed = 600.0f;

	/** Walk speed while recovering stamina */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Sprint|Speed",
		meta = (ClampMin = "0.0", Units = "cm/s"))
	float RecoveringWalkSpeed = 150.0f;

	/** Server-only countdown before stamina regeneration may resume. */
	float StaminaRegenerationDelayRemaining = 0.0f;

	/** Fire weapon input action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* SprintAction;

protected:

	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	virtual void BeginPlay() override;
	virtual void PawnClientRestart() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Items")
	void DoInteract();
	UFUNCTION(BlueprintCallable, Category = "Items")
	void DoDrop();

	UFUNCTION(BlueprintCallable, Category = "Items")
	void DoDrag();

	UFUNCTION(BlueprintCallable, Category = "Items")
	void DoUnDrag();

	FHitResult PerformInteractTrace(bool bIsDrag);
	void HandleInteraction(const FHitResult& HitResult);
	void HandleDrag(const FHitResult& HitResult);

	class UDrag_Component* CurrentDraggedComponent = nullptr;

	UFUNCTION(Server, Reliable)
	void ServerPickupItem(class ABase_Item* Item);

	/** Consumes the held key and unlocks a draggable actor on the authority. */
	UFUNCTION(Server, Reliable)
	void ServerUnlockWithHeldKey(ADrag_Item* Item);
	UFUNCTION(BlueprintCallable, Category = "Items")
	void PickupItem(class ABase_Item* Item);

	/** The only item this character may carry. Authoritative on the server. */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Items")
	TObjectPtr<class ABase_Item> CurrentHeldItem;

	UFUNCTION(BlueprintCallable, Category = "Items")
	void DropCurrentItem();

	UFUNCTION(Server, Reliable)
	void ServerDropCurrentItem();

	UPROPERTY(ReplicatedUsing = OnRep_CurrentChair)
	AChair* CurrentChair;
	UFUNCTION()
	void OnRep_CurrentChair(AChair* PreviousChair);

	UPROPERTY(BlueprintReadWrite, Replicated)
	bool bIsSitting;

	

public:

	UFUNCTION(BlueprintImplementableEvent, Category = "Chair")
	void OnSitStarted(AChair* Chair);

	UFUNCTION(BlueprintImplementableEvent, Category = "Chair")
	void OnSitEnded();

	UFUNCTION(Server, Reliable)
	void ServerStandUp();

	UFUNCTION()
	void StandUp();

	void SitOnChair(AChair* Chair);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetShelfPosition(ADrag_Item* Shelf, const FVector& NewPosition);

	/** Streams a position for one panel of a multi-drawer Drag_Item. */
	UFUNCTION(Server, Reliable)
	void Server_SetShelfPanelPosition(
		ADrag_Item* Shelf,
		FName ShelfComponentName,
		FVector NewPosition);

public:
		// The Client will call this to tell the server to interact with an object
		UFUNCTION(Server, Reliable)
		void Server_InteractWithEnvironment(AActor* InteractableActor);

public:
	/** Server-authoritative door rotation. Routed through the Character because the
	 *  door is a level actor not owned by the client and cannot receive client RPCs directly.
	 *  Unreliable: this streams during a drag like movement input. */
	UFUNCTION(Server, Unreliable)
	void Server_SetDoorRotation(ADrag_Item* Door, FRotator NewRotation);

	/** Streams rotation for a specific panel/pivot of a multi-door Drag_Item. */
	UFUNCTION(Server, Unreliable)
	void Server_SetDoorPanelRotation(ADrag_Item* Door, FName DoorComponentName, FRotator NewRotation);

	/** Returns the first person mesh **/
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	/** Returns first person camera component **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	/** Returns this character's timeline */
	EItemTimeline GetTimeline() const { return CharacterTimeline; }

	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	private:
		void OnEnyInteractTrace(FHitResult HitResult);
		void OnMakeInteractImpulse(FHitResult HitResult);
};

