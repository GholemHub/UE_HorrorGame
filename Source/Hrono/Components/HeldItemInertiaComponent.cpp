#include "Components/HeldItemInertiaComponent.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/Controller.h"
#include "HronoCharacter.h"
#include "Items/Base_Item.h"

UHeldItemInertiaComponent::UHeldItemInertiaComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UHeldItemInertiaComponent::SetProfile(UHeldItemInertiaProfile* NewProfile)
{
	Profile = NewProfile;
	ResetInertia();
}

FRotator UHeldItemInertiaComponent::GetCurrentRotationOffset() const
{
	return FRotator(RotationOffset.X, RotationOffset.Y, RotationOffset.Z);
}

void UHeldItemInertiaComponent::BeginHeld(
	AHronoCharacter* Character,
	const FTransform& InBaseRelativeTransform)
{
	HeldCharacter = Character;
	BaseRelativeTransform = InBaseRelativeTransform;
	bHeld = IsValid(Character);
	bLoggedFirstAppliedFrame = false;
	SetComponentTickEnabled(bHeld);
	ResetInertia();
}

void UHeldItemInertiaComponent::EndHeld(bool bRestoreBaseTransform)
{
	if (bRestoreBaseTransform && bHeld)
	{
		if (AActor* Item = GetOwner())
		{
			if (USceneComponent* Root = Item->GetRootComponent())
			{
				Root->SetRelativeTransform(BaseRelativeTransform);
			}
		}
	}

	bHeld = false;
	bLoggedFirstAppliedFrame = false;
	SetComponentTickEnabled(false);
	HeldCharacter.Reset();
	ResetSamples();
	PositionOffset = FVector::ZeroVector;
	PositionVelocity = FVector::ZeroVector;
	RotationOffset = FVector::ZeroVector;
	RotationVelocity = FVector::ZeroVector;
}

void UHeldItemInertiaComponent::ResetInertia()
{
	PositionOffset = FVector::ZeroVector;
	PositionVelocity = FVector::ZeroVector;
	RotationOffset = FVector::ZeroVector;
	RotationVelocity = FVector::ZeroVector;
	ResetSamples();

	if (bHeld)
	{
		ApplyCurrentOffset();
	}
}

const FHeldItemInertiaSettings& UHeldItemInertiaComponent::GetSettings() const
{
	return IsValid(Profile) ? Profile->Settings : InlineSettings;
}

FRotator UHeldItemInertiaComponent::GetViewRotation(const AHronoCharacter* Character) const
{
	if (!IsValid(Character))
	{
		return FRotator::ZeroRotator;
	}

	if (Character->IsLocallyControlled())
	{
		if (const AController* Controller = Character->GetController())
		{
			return Controller->GetControlRotation();
		}
	}

	// These are already supplied by CharacterMovement/controller replication.
	return Character->HasAuthority()
		? Character->GetControlRotation()
		: Character->GetBaseAimRotation();
}

void UHeldItemInertiaComponent::ResetSamples()
{
	bHaveSamples = false;
	PreviousCharacterVelocity = FVector::ZeroVector;
	PreviousCharacterLocation = FVector::ZeroVector;
	PreviousViewRotation = FRotator::ZeroRotator;
}

void UHeldItemInertiaComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ABase_Item* Item = Cast<ABase_Item>(GetOwner());
	AHronoCharacter* Character = HeldCharacter.Get();
	if (!bHeld || !IsValid(Item) || !IsValid(Character)
		|| Item->OwningCharacter != Character || !IsValid(Item->GetRootComponent()))
	{
		if (bHeld)
		{
			EndHeld(false);
		}
		return;
	}

	const FHeldItemInertiaSettings& Settings = GetSettings();
	if (!Settings.bEnabled)
	{
		if (!PositionOffset.IsNearlyZero() || !RotationOffset.IsNearlyZero())
		{
			ResetInertia();
		}
		return;
	}

	if (!FMath::IsFinite(DeltaTime) || DeltaTime <= UE_SMALL_NUMBER
		|| DeltaTime > FMath::Max(Settings.MaxDeltaTime, 0.016f))
	{
		ResetInertia();
		return;
	}

	const FVector CharacterLocation = Character->GetActorLocation();
	const FVector CharacterVelocity = Character->GetVelocity();
	const FRotator ViewRotation = GetViewRotation(Character);
	if (!IsFiniteVector(CharacterLocation) || !IsFiniteVector(CharacterVelocity)
		|| ViewRotation.ContainsNaN())
	{
		ResetInertia();
		return;
	}

	if (!bHaveSamples)
	{
		PreviousCharacterLocation = CharacterLocation;
		PreviousCharacterVelocity = CharacterVelocity;
		PreviousViewRotation = ViewRotation;
		bHaveSamples = true;
		return;
	}

	const FVector LocationDelta = CharacterLocation - PreviousCharacterLocation;
	const float PitchDelta = FRotator::NormalizeAxis(ViewRotation.Pitch - PreviousViewRotation.Pitch);
	const float YawDelta = FRotator::NormalizeAxis(ViewRotation.Yaw - PreviousViewRotation.Yaw);
	const float RollDelta = FRotator::NormalizeAxis(ViewRotation.Roll - PreviousViewRotation.Roll);
	const float LargestViewDelta = FMath::Max3(FMath::Abs(PitchDelta), FMath::Abs(YawDelta), FMath::Abs(RollDelta));

	if (LocationDelta.SizeSquared() > FMath::Square(FMath::Max(Settings.TeleportDistance, 1.0f))
		|| LargestViewDelta > FMath::Clamp(Settings.ViewDiscontinuityAngle, 1.0f, 180.0f))
	{
		ResetInertia();
		return;
	}

	USceneComponent* Anchor = Character->GetActiveInteractionPoint();
	if (!IsValid(Anchor) || Item->GetRootComponent()->GetAttachParent() != Anchor)
	{
		ResetInertia();
		return;
	}

	const FTransform AnchorTransform = Anchor->GetComponentTransform();
	FVector WorldAcceleration = (CharacterVelocity - PreviousCharacterVelocity) / DeltaTime;
	WorldAcceleration = WorldAcceleration.GetClampedToMaxSize(FMath::Max(Settings.MaxCharacterAcceleration, 1.0f));
	const FVector LocalAcceleration = AnchorTransform.InverseTransformVectorNoScale(WorldAcceleration);
	const FVector LocalPreviousVelocity = AnchorTransform.InverseTransformVectorNoScale(PreviousCharacterVelocity);

	const float MaxAngularSpeed = FMath::Max(Settings.MaxCameraAngularSpeed, 1.0f);
	const float PitchSpeed = FMath::Clamp(PitchDelta / DeltaTime, -MaxAngularSpeed, MaxAngularSpeed);
	const float YawSpeed = FMath::Clamp(YawDelta / DeltaTime, -MaxAngularSpeed, MaxAngularSpeed);
	const float Weight = FMath::Max(Settings.WeightMultiplier, 0.1f);
	const float EffectScale = Character->IsLocallyControlled()
		? 1.0f
		: FMath::Clamp(Settings.RemoteEffectScale, 0.0f, 1.0f);

	FVector TargetPosition = -LocalAcceleration * Settings.CharacterAccelerationStrength;
	const float HorizontalCameraLag = -YawSpeed * Settings.CameraHorizontalStrength;
	const float VerticalCameraLag = -PitchSpeed * Settings.CameraVerticalStrength;
	TargetPosition.Y += HorizontalCameraLag;
	TargetPosition.Z += VerticalCameraLag;

	const float PreviousSpeed = PreviousCharacterVelocity.Size();
	const float CurrentSpeed = CharacterVelocity.Size();
	const float SpeedLoss = FMath::Max(PreviousSpeed - CurrentSpeed, 0.0f);
	if (SpeedLoss > UE_SMALL_NUMBER && !LocalPreviousVelocity.IsNearlyZero())
	{
		const float StopFraction = FMath::Clamp(SpeedLoss / FMath::Max(PreviousSpeed, 1.0f), 0.0f, 1.0f);
		TargetPosition += LocalPreviousVelocity.GetSafeNormal()
			* Settings.SuddenStopSwingStrength * StopFraction;
	}

	FVector TargetRotation;
	TargetRotation.X = TargetPosition.X * Settings.PositionToRotation
		- TargetPosition.Z * Settings.PositionToRotation + VerticalCameraLag;
	TargetRotation.Y = TargetPosition.Y * Settings.PositionToRotation
		+ HorizontalCameraLag;
	TargetRotation.Z = -TargetPosition.Y * Settings.PositionToRotation
		+ HorizontalCameraLag * 0.65f;

	TargetPosition *= Weight * EffectScale;
	TargetRotation *= Weight * EffectScale;
	TargetPosition = ClampVectorAxes(TargetPosition, Settings.MaxPositionOffset.GetAbs());

	const FVector RotationLimits(
		FMath::Abs(Settings.MaxRotationOffset.Pitch),
		FMath::Abs(Settings.MaxRotationOffset.Yaw),
		FMath::Abs(Settings.MaxRotationOffset.Roll));
	TargetRotation = ClampVectorAxes(TargetRotation, RotationLimits);

	const float DeadZone = FMath::Max(Settings.InputDeadZone, 0.0f);
	for (int32 Axis = 0; Axis < 3; ++Axis)
	{
		if (FMath::Abs(TargetPosition[Axis]) < DeadZone)
		{
			TargetPosition[Axis] = 0.0f;
		}
		if (FMath::Abs(TargetRotation[Axis]) < DeadZone)
		{
			TargetRotation[Axis] = 0.0f;
		}
	}

	IntegrateSpring(TargetPosition, TargetRotation, DeltaTime, Settings);
	ApplyCurrentOffset();

	PreviousCharacterLocation = CharacterLocation;
	PreviousCharacterVelocity = CharacterVelocity;
	PreviousViewRotation = ViewRotation;
}

void UHeldItemInertiaComponent::IntegrateSpring(
	const FVector& TargetPosition,
	const FVector& TargetRotation,
	float DeltaTime,
	const FHeldItemInertiaSettings& Settings)
{
	const float Weight = FMath::Max(Settings.WeightMultiplier, 0.1f);
	const float Stiffness = FMath::Max(Settings.SpringStiffness, 0.0f) / Weight;
	const float Damping = FMath::Max(Settings.Damping, 0.0f) / FMath::Sqrt(Weight);
	const float MaxStep = FMath::Clamp(Settings.MaxSubstep, 0.002f, 0.033f);
	const int32 NumSteps = FMath::Max(1, FMath::CeilToInt(DeltaTime / MaxStep));
	const float Step = DeltaTime / static_cast<float>(NumSteps);

	for (int32 StepIndex = 0; StepIndex < NumSteps; ++StepIndex)
	{
		PositionVelocity += (Stiffness * (TargetPosition - PositionOffset) - Damping * PositionVelocity) * Step;
		PositionOffset += PositionVelocity * Step;
		RotationVelocity += (Stiffness * (TargetRotation - RotationOffset) - Damping * RotationVelocity) * Step;
		RotationOffset += RotationVelocity * Step;
	}

	const FVector PositionLimits = Settings.MaxPositionOffset.GetAbs();
	const FVector RotationLimits(
		FMath::Abs(Settings.MaxRotationOffset.Pitch),
		FMath::Abs(Settings.MaxRotationOffset.Yaw),
		FMath::Abs(Settings.MaxRotationOffset.Roll));
	const FVector UnclampedPosition = PositionOffset;
	const FVector UnclampedRotation = RotationOffset;
	PositionOffset = ClampVectorAxes(PositionOffset, PositionLimits);
	RotationOffset = ClampVectorAxes(RotationOffset, RotationLimits);

	for (int32 Axis = 0; Axis < 3; ++Axis)
	{
		if (!FMath::IsNearlyEqual(PositionOffset[Axis], UnclampedPosition[Axis]))
		{
			PositionVelocity[Axis] = 0.0f;
		}
		if (!FMath::IsNearlyEqual(RotationOffset[Axis], UnclampedRotation[Axis]))
		{
			RotationVelocity[Axis] = 0.0f;
		}
	}

	if (!IsFiniteVector(PositionOffset) || !IsFiniteVector(PositionVelocity)
		|| !IsFiniteVector(RotationOffset) || !IsFiniteVector(RotationVelocity))
	{
		ResetInertia();
	}
}

void UHeldItemInertiaComponent::ApplyCurrentOffset()
{
	AActor* Item = GetOwner();
	USceneComponent* Root = IsValid(Item) ? Item->GetRootComponent() : nullptr;
	if (!IsValid(Root))
	{
		return;
	}

	const FVector FinalLocation = BaseRelativeTransform.GetLocation() + PositionOffset;
	const FQuat OffsetRotation(FRotator(RotationOffset.X, RotationOffset.Y, RotationOffset.Z));
	FQuat FinalRotation = OffsetRotation * BaseRelativeTransform.GetRotation();
	FinalRotation.Normalize();
	Root->SetRelativeLocationAndRotation(FinalLocation, FinalRotation);

#if !UE_BUILD_SHIPPING
	if (!bLoggedFirstAppliedFrame)
	{
		const AHronoCharacter* Character = HeldCharacter.Get();
		const USceneComponent* Anchor = Character ? Character->GetActiveInteractionPoint() : nullptr;
		const FVector RootWorld = Root->GetComponentLocation();
		const FVector AnchorWorld = Anchor ? Anchor->GetComponentLocation() : FVector::ZeroVector;
		UE_LOG(LogTemp, Warning,
			TEXT("[HeldInertia] FirstApply Item=%s Authority=%d LocalOwner=%d BaseRelative=%s "
				"PositionOffset=%s FinalRelative=%s RootWorld=%s AnchorWorld=%s DeltaRootAnchor=%s"),
			*GetNameSafe(Item),
			Item->HasAuthority() ? 1 : 0,
			Character && Character->IsLocallyControlled() ? 1 : 0,
			*BaseRelativeTransform.GetLocation().ToCompactString(),
			*PositionOffset.ToCompactString(),
			*FinalLocation.ToCompactString(),
			*RootWorld.ToCompactString(),
			*AnchorWorld.ToCompactString(),
			*(RootWorld - AnchorWorld).ToCompactString());
		bLoggedFirstAppliedFrame = true;
	}
#endif
}

FVector UHeldItemInertiaComponent::ClampVectorAxes(
	const FVector& Value,
	const FVector& AbsoluteLimits)
{
	return FVector(
		FMath::Clamp(Value.X, -AbsoluteLimits.X, AbsoluteLimits.X),
		FMath::Clamp(Value.Y, -AbsoluteLimits.Y, AbsoluteLimits.Y),
		FMath::Clamp(Value.Z, -AbsoluteLimits.Z, AbsoluteLimits.Z));
}

bool UHeldItemInertiaComponent::IsFiniteVector(const FVector& Value)
{
	return FMath::IsFinite(Value.X) && FMath::IsFinite(Value.Y) && FMath::IsFinite(Value.Z);
}
