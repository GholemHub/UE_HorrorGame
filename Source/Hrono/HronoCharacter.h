#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "HronoSharedTools.h"
#include "HronoCollisionChannels.h"
#include "Components/InventoryComponent.h"

#include "HronoCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UDrag_Component;
class ADrag_Item;
class USpotLightComponent;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUpdateSprintMeterDelegate, float, Percentage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSprintStateChangedDelegate, bool, bSprinting);

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

	UPROPERTY(VisibleAnywhere)
	class UInventoryComponent* InventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpotLightComponent* SpotLight;
	
	
protected:


	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* JumpAction;

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
	UInputAction* DragAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* NextItemAction;

public:
	AHronoCharacter();

	/** Delegate called when the sprint meter should be updated */
	FUpdateSprintMeterDelegate OnSprintMeterUpdated;

	/** Delegate called when we start and stop sprinting */
	FSprintStateChangedDelegate OnSprintStateChanged;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> InteractionPoint;

protected:

	UPROPERTY(ReplicatedUsing = OnRep_Sprinting)

	bool bSprinting = false;
	UFUNCTION()
	void OnRep_Sprinting();

	UFUNCTION(Server, Reliable)
	void ServerSetSprinting(bool bNewSprint);
	/** If true, we're recovering stamina */
	bool bRecovering = false;

	UPROPERTY(EditAnywhere, Category = "Walk")
	float WalkSpeed = 250.0f;

	/** Starts sprinting behavior */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void DoStartSprint();

	/** Stops sprinting behavior */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void DoEndSprint();

	/** Called while sprinting at a fixed time interval */
	void SprintFixedTick();

	/** Called from Input Actions for movement input */
	void MoveInput(const FInputActionValue& Value);

	/** Called from Input Actions for looking input */
	void LookInput(const FInputActionValue& Value);

	void NextItemInput(const FInputActionValue& Value);

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_CharacterTimeline, Category = "Timeline")
	EItemTimeline CharacterTimeline = EItemTimeline::Past;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (ClampMin = 0, Units = "cm"))
	float InteractTraceDistance = 300.0f;

	UFUNCTION()
	void OnRep_CharacterTimeline();

	void ApplyTimelineCollision();

	UPROPERTY(EditAnywhere, Category = "Sprint", meta = (ClampMin = 0, ClampMax = 1, Units = "s"))
	float SprintFixedTickTime = 0.03333f;

	/** Sprint stamina amount. Maxes at SprintTime */
	float SprintMeter = 0.0f;

	/** How long we can sprint for, in seconds */
	UPROPERTY(EditAnywhere, Category = "Sprint", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float SprintTime = 3.0f;

	/** Walk speed while sprinting */
	UPROPERTY(EditAnywhere, Category = "Sprint", meta = (ClampMin = 0, ClampMax = 10, Units = "cm/s"))
	float SprintSpeed = 600.0f;

	/** Walk speed while recovering stamina */
	UPROPERTY(EditAnywhere, Category = "Recovery", meta = (ClampMin = 0, ClampMax = 10, Units = "cm/s"))
	float RecoveringWalkSpeed = 150.0f;

	/** Time it takes for the sprint meter to recover */
	UPROPERTY(EditAnywhere, Category = "Recovery", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float RecoveryTime = 0.0f;

	/** Fire weapon input action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* SprintAction;

protected:

	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	virtual void BeginPlay() override;
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
	UFUNCTION(BlueprintCallable, Category = "Items")
	void PickupItem(class ABase_Item* Item);

	UPROPERTY(BlueprintReadWrite, Category = "Items")
	class ABase_Item* CurrentHeldItem;

	UFUNCTION(BlueprintCallable, Category = "Items")
	void DropCurrentItem();

	UFUNCTION(Server, Reliable)
	void ServerDropCurrentItem();
public:
		UFUNCTION(Server, Reliable, WithValidation)
		void Server_SetShelfPosition(ADrag_Item* Shelf, const FVector& NewPosition);

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

	/** Returns the first person mesh **/
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	/** Returns first person camera component **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	/** Returns this character's timeline */
	EItemTimeline GetTimeline() const { return CharacterTimeline; }

	UFUNCTION(Server, Reliable)
	void ServerNextItemInput(float AxisValue);


	private:
		void OnEnyInteractTrace(FHitResult HitResult);
		void OnMakeInteractImpulse(FHitResult HitResult);
};

