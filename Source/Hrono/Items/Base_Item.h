#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HronoSharedTools.h"
#include "HronoCharacter.h"
#include "Net/UnrealNetwork.h"
#include "GameplayTagContainer.h"


#include "Base_Item.generated.h"



UCLASS()
class HRONO_API ABase_Item : public AActor
{
	GENERATED_BODY()
	
public:	
	ABase_Item();


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item TAG")
	FGameplayTagContainer ItemTags;
	
	bool HasTag(FGameplayTag Tag) const
	{
		return ItemTags.HasTag(Tag);
	}

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

	UPROPERTY(ReplicatedUsing = OnRep_OwningCharacter)
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
