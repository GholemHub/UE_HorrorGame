#include "UI/HronoFpsWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"

UHronoFpsWidget::UHronoFpsWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(false);
}

TSharedRef<SWidget> UHronoFpsWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		BuildWidgetTree();
	}

	return Super::RebuildWidget();
}

void UHronoFpsWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UHronoFpsWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (InDeltaTime <= 0.0f)
	{
		return;
	}

	AccumulatedTime += InDeltaTime;
	++AccumulatedFrames;
	if (AccumulatedTime < 0.25f || !IsValid(FpsText))
	{
		return;
	}

	const int32 FramesPerSecond = FMath::RoundToInt(
		static_cast<float>(AccumulatedFrames) / AccumulatedTime);
	FpsText->SetText(FText::Format(
		NSLOCTEXT("HronoFPS", "Counter", "FPS: {0}"),
		FText::AsNumber(FramesPerSecond)));
	AccumulatedTime = 0.0f;
	AccumulatedFrames = 0;
}

void UHronoFpsWidget::BuildWidgetTree()
{
	UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(
		UCanvasPanel::StaticClass(), TEXT("FpsCanvas"));
	WidgetTree->RootWidget = Canvas;

	FpsText = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(), TEXT("FpsText"));
	FpsText->SetText(NSLOCTEXT("HronoFPS", "InitialCounter", "FPS: --"));
	FpsText->SetColorAndOpacity(FSlateColor(FLinearColor(0.75f, 1.0f, 0.75f, 1.0f)));
	FpsText->SetShadowOffset(FVector2D(1.0f, 1.0f));
	FpsText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.9f));
	FSlateFontInfo Font = FpsText->GetFont();
	Font.Size = 18.0f;
	FpsText->SetFont(Font);

	if (UCanvasPanelSlot* FpsCanvasSlot = Canvas->AddChildToCanvas(FpsText))
	{
		FpsCanvasSlot->SetAnchors(FAnchors(1.0f, 0.0f));
		FpsCanvasSlot->SetAlignment(FVector2D(1.0f, 0.0f));
		FpsCanvasSlot->SetPosition(FVector2D(-24.0f, 24.0f));
		FpsCanvasSlot->SetAutoSize(true);
	}
}
