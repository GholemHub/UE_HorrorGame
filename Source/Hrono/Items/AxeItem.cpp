#include "Items/AxeItem.h"

#include "HronoCharacter.h"
#include "Components/HeldItemInertiaComponent.h"
#include "Engine/World.h"

AAxeItem::AAxeItem()
{
	ItemType = EItemType::Tool;
	ItemName = NSLOCTEXT("HronoItems", "AxeName", "Axe");
	ItemDescription = NSLOCTEXT(
		"HronoItems",
		"AxeDescription",
		"A sturdy axe capable of breaking boards barricading doors.");

	// A useful starting pose for a Blueprint child. It remains fully editable.
	HoldOffset = FTransform(
		FRotator(0.0f, 90.0f, -20.0f),
		FVector(10.0f, 8.0f, -12.0f),
		FVector::OneVector);
}

void AAxeItem::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bSwingAnimationActive)
	{
		return;
	}

	// A dropped axe must keep its detached world-space pose. The normal pickup
	// path restores HoldOffset the next time it is attached.
	if (!IsValid(OwningCharacter) || !bIsPickedUp)
	{
		bSwingAnimationActive = false;
		return;
	}

	if (!IsValid(HeldItemInertia))
	{
		bSwingAnimationActive = false;
		return;
	}

	SwingAnimationElapsed += DeltaSeconds;
	const float SafeReturnDuration = FMath::Max(SwingReturnDuration, 0.05f);
	const float LinearAlpha = FMath::Clamp(SwingAnimationElapsed / SafeReturnDuration, 0.0f, 1.0f);
	const float RaisedAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, LinearAlpha, 2.0f);

	const FQuat RaisedRotation = FQuat::Slerp(
		FRotator(SwingAngleY, 0.0f, 0.0f).Quaternion(),
		FQuat::Identity,
		RaisedAlpha);
	HeldItemInertia->SetActionPoseOffset(FVector::ZeroVector, RaisedRotation.Rotator());

	if (LinearAlpha >= 1.0f)
	{
		HeldItemInertia->ClearActionPoseOffset();
		bSwingAnimationActive = false;
	}
}

void AAxeItem::Use_Implementation(AActor* Character)
{
	if (HasAuthority() && Character == OwningCharacter)
	{
		TryStartSwing();
	}
}

bool AAxeItem::TryStartSwing()
{
	UWorld* World = GetWorld();
	if (!HasAuthority()
		|| !IsValid(World)
		|| !IsValid(OwningCharacter)
		|| !bIsPickedUp)
	{
		return false;
	}

	const float CurrentTime = World->GetTimeSeconds();
	if (CurrentTime < NextAllowedSwingTime)
	{
		return false;
	}

	const float TotalLockout = FMath::Max(SwingReturnDuration, 0.05f)
		+ FMath::Max(SwingRecoveryDuration, 0.0f);
	NextAllowedSwingTime = CurrentTime + TotalLockout;
	MulticastStartSwing();
	return true;
}

void AAxeItem::MulticastStartSwing_Implementation()
{
	if (!IsValid(OwningCharacter) || !bIsPickedUp)
	{
		return;
	}

	SwingAnimationElapsed = 0.0f;
	bSwingAnimationActive = true;

	// The downward part of the swing is deliberately immediate. Tick then raises
	// the axe smoothly back to the original held rotation.
	if (IsValid(HeldItemInertia))
	{
		HeldItemInertia->SetActionPoseOffset(
			FVector::ZeroVector,
			FRotator(SwingAngleY, 0.0f, 0.0f));
	}
}
