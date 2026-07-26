#include "Items/TimelineTransferItem.h"
#include "HronoCharacter.h"

#include "Components/BoxComponent.h"

ATimelineTransferItem::ATimelineTransferItem()
{
	TransferBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TransferBox"));
	RootComponent = TransferBox;
	ItemMesh->SetupAttachment(TransferBox);

	TransferBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TransferBox->SetCollisionResponseToAllChannels(ECR_Overlap);
	TransferBox->SetGenerateOverlapEvents(true);
}

void ATimelineTransferItem::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		TransferBox->OnComponentBeginOverlap.AddDynamic(this, &ATimelineTransferItem::OnTransferBoxBeginOverlap);
		TransferBox->OnComponentEndOverlap.AddDynamic(this, &ATimelineTransferItem::OnTransferBoxEndOverlap);
	}
}

void ATimelineTransferItem::OnTransferBoxBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	ABase_Item* Item = Cast<ABase_Item>(OtherActor);
	if (!Item || Item == this || ItemsInTransferBox.Contains(Item))
	{
		return;
	}

	ItemsInTransferBox.Add(Item);
	TransferItem(*Item);
}

void ATimelineTransferItem::OnTransferBoxEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (ABase_Item* Item = Cast<ABase_Item>(OtherActor))
	{
		if (Item->OwningCharacter) {
			Item->ItemTimeline = Item->OwningCharacter->CharacterTimeline;
			ItemsInTransferBox.Remove(Item);
		}
		
	}
}

void ATimelineTransferItem::TransferItem(ABase_Item& Item)
{
	switch (Item.ItemTimeline)
	{
	case EItemTimeline::Past:
		Item.SetItemTimeline(EItemTimeline::Both);
		break;

	case EItemTimeline::Future:
		Item.SetItemTimeline(EItemTimeline::Both);
		break;

	case EItemTimeline::Both:
		if (bTransferBothTimelineItems)
		{
			Item.SetItemTimeline(EItemTimeline::Past);
		}
		break;
	}
}
