// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/Drag_Component.h"
#include "Items/Base_Item.h"
#include "Engine/Engine.h"

#include "Drag_Item.generated.h"

UCLASS()
class HRONO_API ADrag_Item : public ABase_Item
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADrag_Item();


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* FrameMesh;

	/*UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* DoorMesh;*/

	UPROPERTY(VisibleAnywhere)
	class UDrag_Component* DragComponent;

	virtual void UpdateMeshForLocalPlayer() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY()
	FRotator DoorRotation;

};
