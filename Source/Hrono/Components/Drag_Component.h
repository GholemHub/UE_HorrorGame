// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Drag_Component.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HRONO_API UDrag_Component : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDrag_Component();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION()
	void StartDrag(APlayerController* PC);
	
	UFUNCTION()
	void StopDrag();
	
	UFUNCTION()
	void XDrag();
	
	void YDrag();


	UPROPERTY()
	FRotator InitialRotation;

	UPROPERTY()
	float CurrentDoorAngle = 0.f;

	UPROPERTY()
	bool bIsRotating = false;

	UPROPERTY()
	APlayerController* RotatingController = nullptr;

	UPROPERTY()
	FRotator StartRelativeRotation;

	UPROPERTY(EditAnywhere, Category = "Inspect")
	float RotationSpeed = 4.f;

	bool bIsOpen = false;
};
