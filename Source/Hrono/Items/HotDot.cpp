#include "Items/HotDot.h"

#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

AHotDot::AHotDot()
{
	PrimaryActorTick.bCanEverTick = false;
	SetReplicateMovement(false);
	UsableValid = false;

	if (IsValid(ItemMesh))
	{
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ItemMesh->SetGenerateOverlapEvents(false);
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetRelativeScale3D(FVector(0.1f));

		static ConstructorHelpers::FObjectFinder<UStaticMesh> MarkerMesh(
			TEXT("/Game/HorrorEngine/Meshes/Ball_SM.Ball_SM"));
		if (MarkerMesh.Succeeded())
		{
			ItemMesh->SetStaticMesh(MarkerMesh.Object);
		}
	}
}

void AHotDot::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHotDot, bHotDotActive);
	DOREPLIFETIME(AHotDot, bIgnoredByDosimeter);
}

void AHotDot::SetHotDotActive(bool bNewActive)
{
	if (!HasAuthority() || bHotDotActive == bNewActive)
	{
		return;
	}

	bHotDotActive = bNewActive;
	ReceiveHotDotActiveChanged(bHotDotActive);
	ForceNetUpdate();
}

void AHotDot::OnRep_HotDotActive()
{
	ReceiveHotDotActiveChanged(bHotDotActive);
}

bool AHotDot::IsDetectableByDosimeter(EItemTimeline ViewerTimeline) const
{
	if (!bHotDotActive || bIgnoredByDosimeter)
	{
		return false;
	}

	return ItemTimeline == EItemTimeline::Both
		|| ViewerTimeline == EItemTimeline::Both
		|| ItemTimeline == ViewerTimeline;
}

bool AHotDot::TryPickUp(AHronoCharacter* Character)
{
	return false;
}
