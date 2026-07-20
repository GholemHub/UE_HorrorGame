#include "Components/MirrorCaptureControllerComponent.h"

#include "Camera/PlayerCameraManager.h"
#include "Components/SceneCaptureComponent2D.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UMirrorCaptureControllerComponent::UMirrorCaptureControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
}

void UMirrorCaptureControllerComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!IsValid(SceneCapture) && IsValid(Owner))
	{
		SceneCapture = Owner->FindComponentByClass<USceneCaptureComponent2D>();
	}

	// The capture's authored placement is a convenient default mirror plane.
	// Cache it because the capture itself moves to the virtual reflected camera.
	USceneComponent* PlaneComponent = IsValid(MirrorPlane) ? MirrorPlane.Get() : SceneCapture.Get();
	if (IsValid(PlaneComponent))
	{
		PlaneOrigin = PlaneComponent->GetComponentLocation();
		PlaneNormal = PlaneComponent->GetForwardVector().GetSafeNormal();
		bHasMirrorPlane = !PlaneNormal.IsNearlyZero();
	}

	if (IsValid(SceneCapture))
	{
		SceneCapture->bEnableClipPlane = true;
	}
}

void UMirrorCaptureControllerComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsValid(SceneCapture) || !bHasMirrorPlane)
	{
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, PlayerIndex);
	APlayerCameraManager* CameraManager = IsValid(PlayerController) ? PlayerController->PlayerCameraManager : nullptr;
	if (!IsValid(CameraManager))
	{
		return;
	}

	const FVector CameraLocation = CameraManager->GetCameraLocation();
	const FRotator CameraRotation = CameraManager->GetCameraRotation();

	const FPlane ReflectionPlane(PlaneOrigin, PlaneNormal);
	const FVector ReflectedLocation = CameraLocation.MirrorByPlane(ReflectionPlane);
	const FVector ReflectedForward = FMath::GetReflectionVector(CameraRotation.Vector(), PlaneNormal);
	const FVector ReflectedUp = FMath::GetReflectionVector(
		FRotationMatrix(CameraRotation).GetScaledAxis(EAxis::Z),
		PlaneNormal);
	const FRotator ReflectedRotation = FRotationMatrix::MakeFromXZ(ReflectedForward, ReflectedUp).Rotator();

	SceneCapture->SetWorldLocationAndRotation(ReflectedLocation, ReflectedRotation);
	SceneCapture->ClipPlaneBase = PlaneOrigin;
	SceneCapture->ClipPlaneNormal = PlaneNormal;

	if (bMatchPlayerFOV)
	{
		SceneCapture->FOVAngle = FMath::Clamp(CameraManager->GetFOVAngle() + FOVOffset, 5.0f, 170.0f);
	}
}
