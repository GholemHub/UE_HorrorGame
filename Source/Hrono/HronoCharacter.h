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



	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> InteractionPoint;

protected:

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

