#include "Components/GravityAnomalyComponent.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Enviroment/Room.h"
#include "Items/Base_Item.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

UGravityAnomalyComponent::UGravityAnomalyComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	SetIsReplicatedByDefault(true);
}

void UGravityAnomalyComponent::BeginPlay()
{
	Super::BeginPlay();
	SetComponentTickEnabled(false);
	ARoom* Room = Cast<ARoom>(GetOwner());
	if (!IsValid(Room) || !IsValid(Room->RoomVolume))
	{
		return;
	}

	if (Room->HasAuthority())
	{
		ItemDropHandle = ABase_Item::OnServerDropped.AddUObject(this,
			&UGravityAnomalyComponent::HandleItemDropped);
		Room->OnCursedStateChanged.AddUniqueDynamic(this,
			&UGravityAnomalyComponent::HandleCursedStateChanged);
		Room->RoomVolume->OnComponentBeginOverlap.AddUniqueDynamic(this,
			&UGravityAnomalyComponent::HandleBeginOverlap);
		Room->RoomVolume->OnComponentEndOverlap.AddUniqueDynamic(this,
			&UGravityAnomalyComponent::HandleEndOverlap);
		RefreshCandidates();
		ScheduleNextEvent();
	}
	SyncActivity();
}

void UGravityAnomalyComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ItemDropHandle.IsValid())
	{
		ABase_Item::OnServerDropped.Remove(ItemDropHandle);
		ItemDropHandle = FDelegateHandle();
	}
	if (ARoom* Room = Cast<ARoom>(GetOwner()))
	{
		Room->OnCursedStateChanged.RemoveDynamic(this,
			&UGravityAnomalyComponent::HandleCursedStateChanged);
		if (IsValid(Room->RoomVolume))
		{
			Room->RoomVolume->OnComponentBeginOverlap.RemoveDynamic(this,
				&UGravityAnomalyComponent::HandleBeginOverlap);
			Room->RoomVolume->OnComponentEndOverlap.RemoveDynamic(this,
				&UGravityAnomalyComponent::HandleEndOverlap);
		}
	}
	StopAll();
	if (bLocalActivity)
	{
		bLocalActivity = false;
		OnActivityChanged.Broadcast(false);
	}
	Super::EndPlay(EndPlayReason);
}

void UGravityAnomalyComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGravityAnomalyComponent, ActiveItems);
}

bool UGravityAnomalyComponent::IsCursedRoom() const
{
	const ARoom* Room = Cast<ARoom>(GetOwner());
	return IsValid(Room) && Room->IsCursed();
}

TArray<ABase_Item*> UGravityAnomalyComponent::GetActiveItems() const
{
	TArray<ABase_Item*> Result;
	Result.Reserve(ActiveItems.Num());
	for (ABase_Item* Item : ActiveItems)
	{
		if (IsValid(Item))
		{
			Result.Add(Item);
		}
	}
	return Result;
}

void UGravityAnomalyComponent::SetEnabled(bool bNewEnabled)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || bEnabled == bNewEnabled)
	{
		return;
	}
	bEnabled = bNewEnabled;
	if (bEnabled)
	{
		RefreshCandidates();
		ScheduleNextEvent();
	}
	else
	{
		StopAll();
	}
}

void UGravityAnomalyComponent::HandleCursedStateChanged(ARoom* Room, bool bCursed)
{
	if (Room != GetOwner())
	{
		return;
	}
	if (bCursed)
	{
		RefreshCandidates();
		ScheduleNextEvent();
	}
	else
	{
		StopAll();
	}
}

void UGravityAnomalyComponent::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (ABase_Item* Item = Cast<ABase_Item>(OtherActor))
	{
		Candidates.Add(Item);
	}
}

void UGravityAnomalyComponent::HandleEndOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
	ABase_Item* Item = Cast<ABase_Item>(OtherActor);
	const ARoom* Room = Cast<ARoom>(GetOwner());
	if (Item && Room && Room->RoomVolume && !IsInsideRoom(Item))
	{
		Candidates.Remove(Item);
	}
}

void UGravityAnomalyComponent::HandlePropHit(UPrimitiveComponent* HitComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || Hit.ImpactNormal.Z < 0.45f)
	{
		return;
	}
	for (FActiveProp& Prop : ActiveProps)
	{
		const ABase_Item* Item = Prop.Item.Get();
		if (IsValid(Item) && Item->GetItemMesh() == HitComponent)
		{
			Prop.bLanded = true;
			break;
		}
	}
}

void UGravityAnomalyComponent::RefreshCandidates()
{
	const ARoom* Room = Cast<ARoom>(GetOwner());
	if (!Room || !Room->RoomVolume)
	{
		return;
	}
	Candidates.Reset();
	TArray<AActor*> Overlapping;
	Room->RoomVolume->GetOverlappingActors(Overlapping, ABase_Item::StaticClass());
	for (AActor* Actor : Overlapping)
	{
		if (ABase_Item* Item = Cast<ABase_Item>(Actor))
		{
			Candidates.Add(Item);
		}
	}
}

bool UGravityAnomalyComponent::IsInsideRoom(const ABase_Item* Item) const
{
	const ARoom* Room = Cast<ARoom>(GetOwner());
	if (!IsValid(Item) || !IsValid(Room) || !IsValid(Room->RoomVolume)
		|| !IsValid(Item->GetItemMesh()))
	{
		return false;
	}
	const FVector Local = Room->RoomVolume->GetComponentTransform().InverseTransformPosition(
		Item->GetItemMesh()->GetComponentLocation());
	const FVector Extent = Room->RoomVolume->GetUnscaledBoxExtent();
	return FMath::Abs(Local.X) <= Extent.X
		&& FMath::Abs(Local.Y) <= Extent.Y
		&& FMath::Abs(Local.Z) <= Extent.Z;
}

bool UGravityAnomalyComponent::IsEligible(const ABase_Item* Item) const
{
	return IsValid(Item) && Item->CanEnterGravityAnomaly()
		&& !AllowedActorTag.IsNone() && Item->ActorHasTag(AllowedActorTag)
		&& IsInsideRoom(Item);
}

void UGravityAnomalyComponent::HandleItemDropped(ABase_Item* Item)
{
	if (!IsValid(Item) || Item->GetWorld() != GetWorld() || !GetOwner()
		|| !GetOwner()->HasAuthority())
	{
		return;
	}
	if (bDebug)
	{
		UE_LOG(LogTemp, Log,
			TEXT("[GravityAnomaly] Drop %s Room=%s Cursed=%d Tagged=%d Inside=%d Eligible=%d"),
			*GetNameSafe(Item), *GetNameSafe(GetOwner()), IsCursedRoom() ? 1 : 0,
			Item->ActorHasTag(AllowedActorTag) ? 1 : 0,
			IsInsideRoom(Item) ? 1 : 0, IsEligible(Item) ? 1 : 0);
	}
	if (!bEnabled || !bReactToItemDrop || !IsCursedRoom() || !IsEligible(Item))
	{
		return;
	}
	// A fresh throw is a new attempt, even if pickup and drop happened before the
	// previous episode's next Tick could notice the held state.
	for (int32 Index = ActiveProps.Num() - 1; Index >= 0; --Index)
	{
		if (ActiveProps[Index].Item == Item)
		{
			ReleaseProp(Index, true);
		}
	}
	Candidates.Add(Item);
	if (FMath::FRand() > FMath::Clamp(EventChance, 0.0f, 1.0f))
	{
		return;
	}
	StartProp(Item, false);
}

void UGravityAnomalyComponent::ScheduleNextEvent()
{
	if (!GetWorld() || !GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}
	GetWorld()->GetTimerManager().ClearTimer(NextEventTimer);
	if (!bEnabled || !IsCursedRoom() || ActiveProps.Num() > 0)
	{
		return;
	}
	const float Low = FMath::Max(0.1f, MinEventInterval);
	const float High = FMath::Max(Low, MaxEventInterval);
	GetWorld()->GetTimerManager().SetTimer(NextEventTimer, this,
		&UGravityAnomalyComponent::TriggerEvent, FMath::FRandRange(Low, High), false);
}

void UGravityAnomalyComponent::TriggerEvent()
{
	if (!bEnabled || !IsCursedRoom() || !GetOwner()->HasAuthority())
	{
		return;
	}
	if (FMath::FRand() > FMath::Clamp(EventChance, 0.0f, 1.0f))
	{
		ScheduleNextEvent();
		return;
	}
	TArray<ABase_Item*> Eligible;
	for (auto It = Candidates.CreateIterator(); It; ++It)
	{
		ABase_Item* Item = It->Get();
		if (!IsValid(Item))
		{
			It.RemoveCurrent();
		}
		else if (IsEligible(Item))
		{
			const UStaticMeshComponent* Mesh = Item->GetItemMesh();
			// A previous drop resting on the floor must not be re-armed by the
			// periodic room timer. Unsimulated authored props may still fall.
			if (!Mesh->IsSimulatingPhysics()
				|| Mesh->GetPhysicsLinearVelocity().Z < -5.0f)
			{
			Eligible.Add(Item);
			}
		}
	}
	if (Eligible.IsEmpty())
	{
		ScheduleNextEvent();
		return;
	}

	for (int32 Index = Eligible.Num() - 1; Index > 0; --Index)
	{
		Eligible.Swap(Index, FMath::RandHelper(Index + 1));
	}
	const bool bStrong = Eligible.Num() >= 3
		&& FMath::FRand() < FMath::Clamp(StrongEventChance, 0.0f, 1.0f);
	const int32 Minimum = FMath::Max(1, MinAffectedObjects);
	const int32 Maximum = FMath::Max(Minimum, MaxAffectedObjects);
	const int32 Count = bStrong ? FMath::Clamp(StrongEventObjectCount, 3, 6)
		: (Minimum == Maximum || FMath::FRand() < 0.75f
			? Minimum : FMath::RandRange(Minimum + 1, Maximum));
	if (bDebug)
	{
		const UBoxComponent* Volume = CastChecked<ARoom>(GetOwner())->RoomVolume;
		DrawDebugBox(GetWorld(), Volume->GetComponentLocation(), Volume->GetScaledBoxExtent(),
			Volume->GetComponentQuat(), FColor::Purple, false, 5.0f, 0, 2.0f);
		UE_LOG(LogTemp, Log, TEXT("[GravityAnomaly] Room=%s Cached=%d Eligible=%d"),
			*GetNameSafe(GetOwner()), Candidates.Num(), Eligible.Num());
		for (const TWeakObjectPtr<ABase_Item>& Candidate : Candidates)
		{
			if (const ABase_Item* Item = Candidate.Get())
			{
				UE_LOG(LogTemp, Log, TEXT("[GravityAnomaly] Candidate %s Eligible=%d"),
					*GetNameSafe(Item), IsEligible(Item) ? 1 : 0);
				if (const UStaticMeshComponent* Mesh = Item->GetItemMesh())
				{
					DrawDebugSphere(GetWorld(), Mesh->GetComponentLocation(), 10.0f, 8,
						FColor::Cyan, false, 5.0f);
				}
			}
		}
	}

	for (int32 Index = 0; Index < FMath::Min(Eligible.Num(), Count); ++Index)
	{
		StartProp(Eligible[Index], bStrong);
	}

	if (bDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("[GravityAnomaly] Room=%s Props=%d Strong=%d"),
			*GetNameSafe(GetOwner()), ActiveProps.Num(), bStrong ? 1 : 0);
	}
	if (ActiveProps.IsEmpty())
	{
		ScheduleNextEvent();
		return;
	}
}

bool UGravityAnomalyComponent::StartProp(ABase_Item* Item, bool bStrong)
{
	if (!IsEligible(Item) || ActiveItems.Contains(Item))
	{
		return false;
	}
	UStaticMeshComponent* Mesh = Item->GetItemMesh();
	if (!Mesh->IsSimulatingPhysics())
	{
		Item->EnableDroppedPhysics();
	}
	if (!Mesh->IsSimulatingPhysics())
	{
		if (bDebug)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[GravityAnomaly] Physics unavailable for %s Mesh=%s Mobility=%d Collision=%d Registered=%d PhysicsState=%d ValidBody=%d"),
				*GetNameSafe(Item), *GetNameSafe(Mesh->GetStaticMesh()),
				static_cast<int32>(Mesh->Mobility), static_cast<int32>(Mesh->GetCollisionEnabled()),
				Mesh->IsRegistered() ? 1 : 0, Mesh->IsPhysicsStateCreated() ? 1 : 0,
				Mesh->BodyInstance.IsValidBodyInstance() ? 1 : 0);
		}
		return false;
	}
	// Preserve the hand-drop velocity and ordinary gravity until this attempt's
	// sampled freeze time. A landing before then cancels the attempt.
	Mesh->SetEnableGravity(true);
	const float Angle = FMath::FRandRange(0.0f, 2.0f * PI);
	FActiveProp& Prop = ActiveProps.AddDefaulted_GetRef();
	Prop.Item = Item;
	Prop.SideDirection = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f);
	const float MinFall = FMath::Max(0.0f, FallBeforePause);
	const float MaxFall = FMath::Max(MinFall, MaxFallBeforePause);
	Prop.FallDelay = FMath::FRandRange(MinFall, MaxFall);
	Prop.bStrong = bStrong;
	Prop.bWasNotifyRigidBodyCollision = Mesh->BodyInstance.bNotifyRigidBodyCollision;
	Mesh->SetNotifyRigidBodyCollision(true);
	Mesh->OnComponentHit.AddUniqueDynamic(this, &UGravityAnomalyComponent::HandlePropHit);
	if (bDebug)
	{
		DrawDebugDirectionalArrow(GetWorld(), Mesh->GetComponentLocation(),
			Mesh->GetComponentLocation() + Prop.SideDirection * 60.0f, 12.0f,
			FColor::Yellow, false, 5.0f, 0, 2.0f);
		UE_LOG(LogTemp, Log, TEXT("[GravityAnomaly] Fall %s Side=%s Fall=%.2f Pause=%.2f"),
			*GetNameSafe(Item), *Prop.SideDirection.ToCompactString(),
			Prop.FallDelay, FMath::Max(0.0f, PauseDuration));
	}
	ActiveItems.Add(Item);
	GetWorld()->GetTimerManager().ClearTimer(NextEventTimer);
	SetComponentTickEnabled(true);
	SyncActivity();
	GetOwner()->ForceNetUpdate();
	Item->ForceNetUpdate();
	MulticastPropPhase(Item, false, bStrong);
	return true;
}

void UGravityAnomalyComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}
	for (int32 Index = ActiveProps.Num() - 1; Index >= 0; --Index)
	{
		FActiveProp& Prop = ActiveProps[Index];
		ABase_Item* Item = Prop.Item.Get();
		if (!IsEligible(Item))
		{
			ReleaseProp(Index);
			continue;
		}
		UStaticMeshComponent* Mesh = Item->GetItemMesh();
		if (!IsValid(Mesh) || !Mesh->IsSimulatingPhysics())
		{
			ReleaseProp(Index);
			continue;
		}
		if (Prop.bLanded)
		{
			// A support collision ended the fall. Keep the natural landing/bounce,
			// but never apply the anomaly impulse from a resting surface.
			ReleaseProp(Index, true);
			continue;
		}
		Prop.Elapsed += DeltaTime;
		if (!Prop.bPaused)
		{
			const float VerticalSpeed = Mesh->GetPhysicsLinearVelocity().Z;
			if (VerticalSpeed < -5.0f)
			{
				Prop.bWasFalling = true;
			}
			else if (Prop.bWasFalling || !Mesh->IsAnyRigidBodyAwake())
			{
				// Covers a missed hit callback after landing or physics sleep.
				ReleaseProp(Index, true);
				continue;
			}
			if (Prop.Elapsed >= Prop.FallDelay)
			{
				if (VerticalSpeed >= -5.0f)
				{
					// The object has not started descending (for example it was
					// thrown upward). Wait only until the configured maximum.
					if (Prop.Elapsed >= FMath::Max(FallBeforePause, MaxFallBeforePause))
					{
						ReleaseProp(Index, true);
					}
					continue;
				}
				Prop.bPaused = true;
				Mesh->SetEnableGravity(false);
				Mesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
				Mesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
				if (bDebug)
				{
					UE_LOG(LogTemp, Log, TEXT("[GravityAnomaly] Freeze %s"), *GetNameSafe(Item));
				}
			}
		}
		if (Prop.bPaused && Prop.Elapsed >= Prop.FallDelay
			+ FMath::Max(0.0f, PauseDuration))
		{
			Mesh->SetEnableGravity(true);
			const float Strength = Prop.bStrong ? FMath::Max(1.0f, StrongImpulseMultiplier) : 1.0f;
			const FVector Impulse = Strength * (
				Prop.SideDirection * FMath::Max(0.0f, SideImpulseSpeed)
				- FVector::UpVector * FMath::Max(0.0f, DownwardImpulseSpeed));
			Mesh->AddImpulse(Impulse, NAME_None, true);
			if (bDebug)
			{
				UE_LOG(LogTemp, Log, TEXT("[GravityAnomaly] Impulse %s DeltaV=%s"),
					*GetNameSafe(Item), *Impulse.ToCompactString());
			}
			ReleaseProp(Index, true);
			continue;
		}
		if (Prop.bPaused)
		{
			Mesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
			Mesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
		}
	}
	if (ActiveProps.IsEmpty())
	{
		SetComponentTickEnabled(false);
		ScheduleNextEvent();
	}
}

void UGravityAnomalyComponent::ReleaseProp(int32 Index, bool bPreserveMomentum)
{
	FActiveProp Prop = ActiveProps[Index];
	if (bDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("[GravityAnomaly] Release %s Elapsed=%.2f Strong=%d PreserveMomentum=%d"),
			*GetNameSafe(Prop.Item.Get()), Prop.Elapsed, Prop.bStrong ? 1 : 0,
			bPreserveMomentum ? 1 : 0);
	}
	ActiveProps.RemoveAtSwap(Index);
	ABase_Item* Item = Prop.Item.Get();
	ActiveItems.Remove(Item);
	if (IsValid(Item))
	{
		if (UStaticMeshComponent* Mesh = Item->GetItemMesh(); IsValid(Mesh))
		{
			Mesh->OnComponentHit.RemoveDynamic(this, &UGravityAnomalyComponent::HandlePropHit);
			Mesh->SetNotifyRigidBodyCollision(Prop.bWasNotifyRigidBodyCollision);
		}
		if (UStaticMeshComponent* Mesh = Item->GetItemMesh();
			IsValid(Mesh) && Mesh->IsSimulatingPhysics() && Item->CanEnterGravityAnomaly())
		{
			if (!bPreserveMomentum)
			{
				Mesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
				Mesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
			}
			Mesh->SetEnableGravity(true);
		}
		Item->ForceNetUpdate();
		MulticastPropPhase(Item, true, Prop.bStrong);
	}
	SyncActivity();
	GetOwner()->ForceNetUpdate();
}

void UGravityAnomalyComponent::StopAll()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(NextEventTimer);
	}
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		while (!ActiveProps.IsEmpty())
		{
			ReleaseProp(ActiveProps.Num() - 1);
		}
	}
	SetComponentTickEnabled(false);
}

void UGravityAnomalyComponent::OnRep_ActiveItems()
{
	SyncActivity();
}

void UGravityAnomalyComponent::SyncActivity()
{
	const bool bActive = !ActiveItems.IsEmpty();
	if (bLocalActivity != bActive)
	{
		bLocalActivity = bActive;
		OnActivityChanged.Broadcast(bActive);
	}
}

void UGravityAnomalyComponent::MulticastPropPhase_Implementation(
	ABase_Item* Item, bool bReleased, bool bStrongEvent)
{
	if (IsValid(Item))
	{
		OnPropPhaseChanged.Broadcast(Item, bReleased, bStrongEvent);
	}
}
