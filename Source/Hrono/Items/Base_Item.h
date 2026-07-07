#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HronoSharedTools.h"

#include "Net/UnrealNetwork.h"
#include "GameplayTagContainer.h"
#include "HronoSharedTools.h"

#include "Base_Item.generated.h"

class AHronoCharacter;
class USoundBase;

UCLASS()
class HRONO_API ABase_Item : public AActor
{
	GENERATED_BODY()
	
public:	
	ABase_Item();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemType ItemType = EItemType::None;

	//EItemTimeline ItemTimeline = EItemTimeline::Future;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item TAG")
	FGameplayTag ItemTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item TAG")
	FGameplayTagContainer ItemTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item TAG")
	bool UsableValid = true;

	
	bool HasTag(FGameplayTag Tag) const
	{
		return ItemTags.HasTag(Tag);
	}
	UFUNCTION(BlueprintNativeEvent)
	void Use(AActor* Character);

	virtual void Use_Implementation(AActor* Character);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> DefaultSceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* ItemMesh;
	/** Display name of the item */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FText ItemName;

	/** Description of the item */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FText ItemDescription;

	/** Played when this item is picked up. Assign any sound in Blueprint. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> PickupSound;

	/** Played when this item is dropped. Assign any sound in Blueprint. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> DropSound;

	/** Which timeline this item belongs to (determines who can see/pick it up) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EItemTimeline ItemTimeline = EItemTimeline::Both;

	UPROPERTY(EditAnywhere)
	UStaticMesh* PastMesh;

	UPROPERTY(EditAnywhere)
	UStaticMesh* FutureMesh;

	UFUNCTION(BlueprintCallable, Category = "Item")
	virtual void UpdateMeshForLocalPlayer();

	bool TryPickUp(AHronoCharacter* Character);
	void OnPickedUp(AHronoCharacter* Character);

	void AttachToCharacter();

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OwningCharacter)
	AHronoCharacter* OwningCharacter;

	UFUNCTION()
	void OnRep_OwningCharacter();

	UFUNCTION(BlueprintCallable, Category = "Item")

	void Drop();

	UPROPERTY(BlueprintReadOnly, Category = "Item")
	bool bIsPickedUp = false;

	UFUNCTION()
	void DetachFromCharacter();
	
	UFUNCTION()
	void UpdateVisibilityForLocalPlayer(EItemTimeline ViewerTimeline);
	
protected:
	virtual void BeginPlay() override;

	// Tracks what mesh state is currently visible to avoid spamming updates
	EItemTimeline CurrentCachedTimeline = EItemTimeline::Both;
public:	
	virtual void Tick(float DeltaTime) override;
	UStaticMeshComponent* GetItemMesh() const { return ItemMesh; }

};
