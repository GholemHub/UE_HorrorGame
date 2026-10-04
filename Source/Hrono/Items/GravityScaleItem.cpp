#include "Items/GravityScaleItem.h"

#include "Components/BoxComponent.h"
#include "Components/GravityAnomalyComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/Engine.h"
#include "Engine/OverlapResult.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Enviroment/Room.h"
#include "HronoCharacter.h"
#include "HronoCollisionChannels.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	constexpr float NormalGravityMS2 = 9.81f;
	const FVector PlatformHalfExtent(22.5f, 17.5f, 4.0f);
}

AGravityScaleItem::AGravityScaleItem()
{
	ItemName = NSLOCTEXT("HronoItems", "GravityScaleName", "Gravity Scale");
	ItemDescription = NSLOCTEXT("HronoItems", "GravityScaleDescription",
		"Place a loose item on the platform to compare its apparent weight.");
	ItemTimeline = EItemTimeline::Both;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (Cube.Succeeded()) ItemMesh->SetStaticMesh(Cube.Object);
	ItemMesh->SetRelativeScale3D(FVector(0.45f, 0.35f, 0.08f));
	ItemMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	Display = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ScaleDisplay"));
	Display->SetupAttachment(ItemMesh);
	Display->SetRelativeLocation(FVector(0.0f, 0.0f, 65.0f));
	Display->SetAbsolute(false, false, true); // Do not squash text with the thin platform mesh.
	Display->SetHorizontalAlignment(EHTA_Center);
	Display->SetWorldSize(9.0f);
	Display->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Display->SetText(FText::FromString(TEXT("SCALE\n---")));
}

void AGravityScaleItem::BeginPlay()
{
	Super::BeginPlay();
	UpdateDisplay();
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(SampleTimer, this, &AGravityScaleItem::SamplePlatform,
			FMath::Max(0.05f, SampleInterval), true);
	}
	if (GetNetMode() != NM_DedicatedServer)
	{
		GetWorldTimerManager().SetTimer(DebugTimer, this, &AGravityScaleItem::ShowLocalDebug,
			0.2f, true);
	}
}

void AGravityScaleItem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(SampleTimer);
	GetWorldTimerManager().ClearTimer(DebugTimer);
	Super::EndPlay(EndPlayReason);
}

bool AGravityScaleItem::TryPickUp(AHronoCharacter* Character)
{
	const bool bPickedUp = Super::TryPickUp(Character);
	if (bPickedUp && HasAuthority()) SetReading(FGravityScaleReading());
	return bPickedUp;
}

void AGravityScaleItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGravityScaleItem, Reading);
}

void AGravityScaleItem::OnRep_Reading()
{
	UpdateDisplay();
}

const ARoom* AGravityScaleItem::FindContainingRoom() const
{
	const UWorld* World = GetWorld();
	if (!World || !IsValid(ItemMesh)) return nullptr;
	const FVector Location = ItemMesh->GetComponentLocation();
	const ARoom* BestRoom = nullptr;
	float BestDistanceSq = TNumericLimits<float>::Max();
	for (TActorIterator<ARoom> It(World); It; ++It)
	{
		const ARoom* Room = *It;
		if (!IsValid(Room) || !IsValid(Room->RoomVolume)) continue;
		const FVector Local = Room->RoomVolume->GetComponentTransform().InverseTransformPosition(Location);
		const FVector Extent = Room->RoomVolume->GetUnscaledBoxExtent();
		if (FMath::Abs(Local.X) > Extent.X || FMath::Abs(Local.Y) > Extent.Y
			|| FMath::Abs(Local.Z) > Extent.Z) continue;
		const float DistanceSq = FVector::DistSquared(Location, Room->RoomVolume->GetComponentLocation());
		if (DistanceSq < BestDistanceSq)
		{
			BestRoom = Room;
			BestDistanceSq = DistanceSq;
		}
	}
	return BestRoom;
}

ABase_Item* AGravityScaleItem::FindSupportedItem() const
{
	if (!IsValid(ItemMesh) || ItemMesh->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
		return nullptr;
	const FVector Up = ItemMesh->GetUpVector();
	if (FVector::DotProduct(Up, FVector::UpVector) < 0.85f) return nullptr;
	const FVector Surface = ItemMesh->GetComponentLocation() + Up * PlatformHalfExtent.Z;
	const FVector ProbeCenter = Surface + Up * (MaximumItemHeight * 0.5f);
	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams Objects;
	Objects.AddObjectTypesToQuery(COLLISION_CHANNEL_ITEM);
	FCollisionQueryParams Query(SCENE_QUERY_STAT(GravityScaleProbe), false);
	Query.AddIgnoredActor(this);
	GetWorld()->OverlapMultiByObjectType(Overlaps, ProbeCenter, ItemMesh->GetComponentQuat(),
		Objects, FCollisionShape::MakeBox(FVector(PlatformHalfExtent.X,
			PlatformHalfExtent.Y, MaximumItemHeight * 0.5f)), Query);
	ABase_Item* Best = nullptr;
	float BestHeight = TNumericLimits<float>::Max();
	for (const FOverlapResult& Overlap : Overlaps)
	{
		ABase_Item* Candidate = Cast<ABase_Item>(Overlap.GetActor());
		if (!IsValid(Candidate) || Candidate == this || !Candidate->CanBePickedUp()
			|| Candidate->bIsPickedUp || IsValid(Candidate->OwningCharacter)
			|| (ItemTimeline != EItemTimeline::Both && Candidate->ItemTimeline != EItemTimeline::Both
				&& Candidate->ItemTimeline != ItemTimeline)) continue;
		UStaticMeshComponent* Mesh = Candidate->GetItemMesh();
		if (!IsValid(Mesh) || !Mesh->IsSimulatingPhysics() || Mesh->GetMass() <= 0.0f
			|| Mesh->GetPhysicsLinearVelocity().Size() > 15.0f
			|| Mesh->GetPhysicsAngularVelocityInDegrees().Size() > 90.0f) continue;
		const FVector Start = Mesh->Bounds.Origin + Up * 5.0f;
		FHitResult Hit;
		FCollisionQueryParams Trace(SCENE_QUERY_STAT(GravityScaleSupport), false);
		Trace.AddIgnoredActor(Candidate);
		if (!GetWorld()->LineTraceSingleByChannel(Hit, Start,
			Start - Up * (MaximumItemHeight + 10.0f), ECC_Visibility, Trace)
			|| Hit.GetComponent() != ItemMesh || FVector::DotProduct(Hit.ImpactNormal, Up) < 0.8f)
			continue;
		const FVector Relative = Hit.ImpactPoint - Surface;
		if (FMath::Abs(FVector::DotProduct(Relative, ItemMesh->GetForwardVector()))
				> PlatformHalfExtent.X - 2.0f
			|| FMath::Abs(FVector::DotProduct(Relative, ItemMesh->GetRightVector()))
				> PlatformHalfExtent.Y - 2.0f) continue;
		const float Height = FVector::DotProduct(Mesh->Bounds.Origin - Surface, Up);
		if (Height < BestHeight)
		{
			Best = Candidate;
			BestHeight = Height;
		}
	}
	return Best;
}

void AGravityScaleItem::SamplePlatform()
{
	if (!HasAuthority()) return;
	FGravityScaleReading NewReading;
	if (bIsPickedUp || IsValid(OwningCharacter))
	{
		SetReading(NewReading);
		return;
	}
	ABase_Item* Candidate = FindSupportedItem();
	if (IsValid(Candidate))
	{
		NewReading.Item = Candidate;
		NewReading.MassKg = Candidate->GetItemMesh()->GetMass();
		const ARoom* Room = FindContainingRoom();
		if (IsValid(Room) && IsValid(Room->GravityAnomaly)
			&& Room->bIsCursed && Room->GravityAnomaly->bEnabled)
		{
			NewReading.bCursedRoom = true;
			NewReading.GravityMS2 = FMath::Clamp(Room->GravityAnomaly->MaxFastGravity,
				NormalGravityMS2, 10.5f);
		}
		NewReading.WeightNewtons = NewReading.MassKg * NewReading.GravityMS2;
		NewReading.ApparentKg = NewReading.WeightNewtons / NormalGravityMS2;
	}
	SetReading(NewReading);
}

void AGravityScaleItem::SetReading(const FGravityScaleReading& NewReading)
{
	if (Reading.Item == NewReading.Item && Reading.bCursedRoom == NewReading.bCursedRoom
		&& FMath::IsNearlyEqual(Reading.MassKg, NewReading.MassKg, 0.001f)
		&& FMath::IsNearlyEqual(Reading.GravityMS2, NewReading.GravityMS2, 0.001f)) return;
	Reading = NewReading;
	UpdateDisplay();
	ForceNetUpdate();
	UE_LOG(LogTemp, Log,
		TEXT("[GravityScale] %s Item=%s Mass=%.3f kg g=%.3f m/s^2 Weight=%.3f N Display=%.3f kg Cursed=%d"),
		*GetName(), *GetNameSafe(Reading.Item), Reading.MassKg, Reading.GravityMS2,
		Reading.WeightNewtons, Reading.ApparentKg, Reading.bCursedRoom ? 1 : 0);
}

void AGravityScaleItem::UpdateDisplay()
{
	if (!IsValid(Display)) return;
	const FString Label = IsValid(Reading.Item)
		? FString::Printf(TEXT("%.3f kg\n%.2f N"), Reading.ApparentKg, Reading.WeightNewtons)
		: TEXT("SCALE\n---");
	Display->SetText(FText::FromString(Label));
}

void AGravityScaleItem::ShowLocalDebug()
{
	if (!bDebugOnScreen || !GEngine || !IsValid(Reading.Item)) return;
	const AHronoCharacter* Viewer = Cast<AHronoCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!IsValid(Viewer) || (ItemTimeline != EItemTimeline::Both
		&& Viewer->GetTimeline() != ItemTimeline)
		|| FVector::DistSquared(Viewer->GetActorLocation(), GetActorLocation()) > FMath::Square(500.0f))
		return;
	GEngine->AddOnScreenDebugMessage(static_cast<uint64>(GetUniqueID()), 0.25f,
		Reading.bCursedRoom ? FColor::Yellow : FColor::Green,
		FString::Printf(TEXT("SCALE %s | mass %.3f kg | g %.3f m/s^2 | weight %.3f N | display %.3f kg"),
			*GetNameSafe(Reading.Item), Reading.MassKg, Reading.GravityMS2,
			Reading.WeightNewtons, Reading.ApparentKg));
}
