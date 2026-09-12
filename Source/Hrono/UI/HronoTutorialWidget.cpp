#include "UI/HronoTutorialWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateColorBrush.h"
#include "Components/BackgroundBlur.h"
#include "Components/Border.h"
#include "Components/BorderSlot.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Texture2D.h"
#include "Framework/Application/IInputProcessor.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerController.h"
#include "ImageUtils.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

namespace HronoTutorial
{
	constexpr int32 TutorialCount = 5;
	const FLinearColor Accent(0.36f, 0.78f, 0.82f, 1.0f);
	const FLinearColor TextPrimary(0.91f, 0.93f, 0.92f, 1.0f);
	const FLinearColor TextMuted(0.55f, 0.62f, 0.62f, 1.0f);

	struct FTutorialPage
	{
		FText Title;
		FText Subtitle;
		FText Description;
		FText Controls;
		const TCHAR* ImageFilename;
	};

	/**
	 * UI-only input normally sends keys to the focused Slate widget. A click on
	 * empty menu space can clear that focus, so this processor keeps Tab/Escape
	 * available for closing the modal regardless of the current mouse focus.
	 */
	class FTutorialInputPreProcessor final : public IInputProcessor
	{
	public:
		explicit FTutorialInputPreProcessor(UHronoTutorialWidget* InWidget)
			: Widget(InWidget)
		{
		}

		virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override
		{
		}

		virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override
		{
			UHronoTutorialWidget* TutorialWidget = Widget.Get();
			if (TutorialWidget && TutorialWidget->IsMenuOpen()
				&& (InKeyEvent.GetKey() == EKeys::Tab || InKeyEvent.GetKey() == EKeys::Escape))
			{
				TutorialWidget->CloseMenu();
				return true;
			}
			return false;
		}

		virtual bool HandleMouseButtonDownEvent(
			FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override
		{
			UHronoTutorialWidget* TutorialWidget = Widget.Get();
			bMouseDownConsumed = TutorialWidget && TutorialWidget->HandleMenuMouseButtonDown(MouseEvent);
			return bMouseDownConsumed;
		}

		virtual bool HandleMouseButtonUpEvent(
			FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override
		{
			UHronoTutorialWidget* TutorialWidget = Widget.Get();
			// A mouse-down handled by the modal must not leak its release into gameplay,
			// including when the close button hid the menu on mouse-down.
			const bool bHandle = bMouseDownConsumed || (TutorialWidget && TutorialWidget->IsMenuOpen());
			bMouseDownConsumed = false;
			return bHandle;
		}

		virtual bool HandleMouseWheelOrGestureEvent(
			FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent,
			const FPointerEvent* InGestureEvent) override
		{
			UHronoTutorialWidget* TutorialWidget = Widget.Get();
			return TutorialWidget && TutorialWidget->IsMenuOpen();
		}

		virtual const TCHAR* GetDebugName() const override
		{
			return TEXT("HronoTutorialInput");
		}

	private:
		TWeakObjectPtr<UHronoTutorialWidget> Widget;
		bool bMouseDownConsumed = false;
	};

	FTutorialPage GetPage(EHronoTutorialItem Item)
	{
		switch (Item)
		{
		case EHronoTutorialItem::Dosimeter:
			return {
				NSLOCTEXT("HronoTutorial", "DosimeterTitle", "HOW TO USE THE DOSIMETER"),
				FText::GetEmpty(),
				NSLOCTEXT("HronoTutorial", "DosimeterDescription",
					"Each player must search the same room with their dosimeter. Follow the increasing readings "
					"and faster clicking until both devices lead to the same location.\n\n"
					"✓ CONFIRMED EVIDENCE\n"
					"If both dosimeters react strongly at the same point, the evidence is confirmed.\n\n"
					"✗ NOT EVIDENCE\n"
					"If only one dosimeter reacts, or the devices lead to different locations, it does not count "
					"as evidence. Keep searching together."),
				FText::GetEmpty(),
				nullptr };
		case EHronoTutorialItem::Clock:
			return {
				NSLOCTEXT("HronoTutorial", "ClockTitle", "HOW TO CHECK THE CLOCKS"),
				FText::GetEmpty(),
				NSLOCTEXT("HronoTutorial", "ClockDescription",
					"Each player must examine the clock in the same room within their own timeline. Compare how "
					"both clocks behave.\n\n"
					"✓ CONFIRMED EVIDENCE\n"
					"If both clocks behave abnormally—such as spinning rapidly or moving erratically—the evidence "
					"is confirmed.\n\n"
					"✗ NOT EVIDENCE\n"
					"If only one clock behaves abnormally while the other works normally, it does not count as "
					"evidence. Check another pair of clocks."),
				FText::GetEmpty(),
				nullptr };
		case EHronoTutorialItem::Skull:
			return {
				NSLOCTEXT("HronoTutorial", "SkullTitle", "RITUAL SKULL"),
				NSLOCTEXT("HronoTutorial", "SkullSubtitle", "UNITE PAST AND FUTURE"),
				NSLOCTEXT("HronoTutorial", "SkullDescription",
					"The ritual needs a matching pair of skulls: one from the Past and one from the Future. "
					"Bring both skulls into the same cursed room and drop them to begin the ritual."),
				NSLOCTEXT("HronoTutorial", "SkullControls", "DROP BOTH SKULLS IN THE SAME RITUAL ROOM"),
				TEXT("Tutorial_Skull.png") };
		case EHronoTutorialItem::Axe:
			return {
				NSLOCTEXT("HronoTutorial", "AxeTitle", "AXE"),
				NSLOCTEXT("HronoTutorial", "AxeSubtitle", "BREAK THE BARRICADE"),
				NSLOCTEXT("HronoTutorial", "AxeDescription",
					"The axe breaks wooden boards that barricade doors. Hold the axe, aim at a board, "
					"and interact. Each valid strike damages the obstruction until the way is clear."),
				NSLOCTEXT("HronoTutorial", "AxeControls", "AIM AT A BOARD  •  PRESS INTERACT TO SWING"),
				TEXT("Tutorial_Axe.png") };
		case EHronoTutorialItem::Monocle:
		default:
			return {
				NSLOCTEXT("HronoTutorial", "MonocleTitle", "HOW TO USE THE MONOCLE"),
				FText::GetEmpty(),
				NSLOCTEXT("HronoTutorial", "MonocleDescription",
					"Each player must use their monocle to examine the same painting in their own timeline. "
					"Keep the painting inside the frame and tell your partner what you see.\n\n"
					"✓ CONFIRMED EVIDENCE\n"
					"If both players see the same anomaly—such as glowing white eyes—the evidence is confirmed.\n\n"
					"✗ NOT EVIDENCE\n"
					"If only one player sees the anomaly while the other sees a normal painting, it does not "
					"count as evidence. Keep searching and examine another painting together."),
				FText::GetEmpty(),
				TEXT("Tutorial_Monocle.png") };
		}
	}

	UTextBlock* MakeText(UWidgetTree* Tree, const FText& Text, int32 Size, const FLinearColor& Color)
	{
		UTextBlock* Label = Tree->ConstructWidget<UTextBlock>();
		Label->SetText(Text);
		Label->SetColorAndOpacity(FSlateColor(Color));
		Label->SetAutoWrapText(true);
		Label->SetShadowOffset(FVector2D(1.0f, 1.0f));
		Label->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f));
		FSlateFontInfo Font = Label->GetFont();
		Font.Size = static_cast<float>(Size);
		Label->SetFont(Font);
		return Label;
	}

	void SetButtonStyle(UButton* Button)
	{
		const FSlateColorBrush Normal(FLinearColor::Transparent);
		const FSlateColorBrush Hovered(FLinearColor(0.10f, 0.10f, 0.10f, 0.88f));
		const FSlateColorBrush Pressed(FLinearColor(0.16f, 0.16f, 0.16f, 0.96f));
		FButtonStyle Style;
		Style.SetNormal(Normal).SetHovered(Hovered).SetPressed(Pressed).SetDisabled(Normal)
			.SetNormalPadding(FMargin(13.0f, 11.0f)).SetPressedPadding(FMargin(13.0f, 12.0f, 13.0f, 10.0f));
		Button->SetStyle(Style);
	}

	UButton* MakeNavigationButton(UWidgetTree* Tree, UVerticalBox* Parent, const FText& Text, FName Name)
	{
		UButton* Button = Tree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
		SetButtonStyle(Button);
		Button->SetClickMethod(EButtonClickMethod::MouseDown);
		Button->SetIsEnabled(true);
		Button->SetVisibility(ESlateVisibility::Visible);
		UTextBlock* Label = MakeText(Tree, Text, 18, TextPrimary);
		Label->SetVisibility(ESlateVisibility::HitTestInvisible);
		if (UButtonSlot* Slot = Cast<UButtonSlot>(Button->AddChild(Label)))
		{
			Slot->SetHorizontalAlignment(HAlign_Left);
			Slot->SetVerticalAlignment(VAlign_Center);
		}
		if (UVerticalBoxSlot* Slot = Parent->AddChildToVerticalBox(Button))
		{
			Slot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
			Slot->SetHorizontalAlignment(HAlign_Fill);
		}
		return Button;
	}

	FText ItemName(EHronoTutorialItem Item)
	{
		return GetPage(Item).Title;
	}
}

UHronoTutorialWidget::UHronoTutorialWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);

	static ConstructorHelpers::FObjectFinder<UTexture2D> CorrectMonocleImage(
		TEXT("/Game/_Alex/Images/Tutorial/Monocle_Correct.Monocle_Correct"));
	if (CorrectMonocleImage.Succeeded())
	{
		MonocleCorrectTexture = CorrectMonocleImage.Object;
	}
	static ConstructorHelpers::FObjectFinder<UTexture2D> WrongMonocleImage(
		TEXT("/Game/_Alex/Images/Tutorial/Monocle_Wrong.Monocle_Wrong"));
	if (WrongMonocleImage.Succeeded())
	{
		MonocleWrongTexture = WrongMonocleImage.Object;
	}
	static ConstructorHelpers::FObjectFinder<UTexture2D> CorrectDosimeterImage(
		TEXT("/Game/_Alex/Images/Tutorial/Dozimetr_Correct.Dozimetr_Correct"));
	if (CorrectDosimeterImage.Succeeded())
	{
		DosimeterCorrectTexture = CorrectDosimeterImage.Object;
	}
	static ConstructorHelpers::FObjectFinder<UTexture2D> WrongDosimeterImage(
		TEXT("/Game/_Alex/Images/Tutorial/Dozimetr_Wrong.Dozimetr_Wrong"));
	if (WrongDosimeterImage.Succeeded())
	{
		DosimeterWrongTexture = WrongDosimeterImage.Object;
	}
	static ConstructorHelpers::FObjectFinder<UTexture2D> CorrectClockImage(
		TEXT("/Game/_Alex/Images/Tutorial/Clock_Correct.Clock_Correct"));
	if (CorrectClockImage.Succeeded())
	{
		ClockCorrectTexture = CorrectClockImage.Object;
	}
	static ConstructorHelpers::FObjectFinder<UTexture2D> WrongClockImage(
		TEXT("/Game/_Alex/Images/Tutorial/Clock_Wrong.Clock_Wrong"));
	if (WrongClockImage.Succeeded())
	{
		ClockWrongTexture = WrongClockImage.Object;
	}
}

TSharedRef<SWidget> UHronoTutorialWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		BuildWidgetTree();
	}
	return Super::RebuildWidget();
}

void UHronoTutorialWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BindButtons();
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	SetGameStageText(NSLOCTEXT("HronoTutorial", "DefaultStage", "STAGE  •  INVESTIGATION"));
	SelectTutorial(SelectedItem);
}

void UHronoTutorialWidget::NativeDestruct()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(PromptTimer);
	}
	CloseMenu();
	Super::NativeDestruct();
}

FReply UHronoTutorialWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Tab || InKeyEvent.GetKey() == EKeys::Escape)
	{
		CloseMenu();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UHronoTutorialWidget::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Tab || InKeyEvent.GetKey() == EKeys::Escape)
	{
		CloseMenu();
		return FReply::Handled();
	}
	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

FReply UHronoTutorialWidget::NativeOnPreviewMouseButtonDown(
	const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (HandleMenuMouseButtonDown(InMouseEvent))
	{
		return FReply::Handled();
	}
	return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
}

bool UHronoTutorialWidget::HandleMenuMouseButtonDown(const FPointerEvent& MouseEvent)
{
	if (!bMenuOpen)
	{
		return false;
	}

	// The modal consumes every mouse button so gameplay can never see a click.
	if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return true;
	}

	const FVector2D PointerPosition = MouseEvent.GetScreenSpacePosition();
	const auto WasClicked = [&PointerPosition](const UButton* Button)
	{
		return Button && Button->IsVisible() && Button->GetCachedGeometry().IsUnderLocation(PointerPosition);
	};
	const bool bMonocleHit = WasClicked(MonocleButton);
	const bool bDosimeterHit = WasClicked(DosimeterButton);
	const bool bClockHit = WasClicked(ClockButton);
	const bool bSkullHit = WasClicked(SkullButton);
	const bool bAxeHit = WasClicked(AxeButton);
	const bool bCloseHit = WasClicked(CloseButton);

	UButton* NewFocus = nullptr;
	if (bMonocleHit)
	{
		SelectTutorial(EHronoTutorialItem::Monocle);
		NewFocus = MonocleButton;
	}
	else if (bDosimeterHit)
	{
		SelectTutorial(EHronoTutorialItem::Dosimeter);
		NewFocus = DosimeterButton;
	}
	else if (bClockHit)
	{
		SelectTutorial(EHronoTutorialItem::Clock);
		NewFocus = ClockButton;
	}
	else if (bSkullHit)
	{
		SelectTutorial(EHronoTutorialItem::Skull);
		NewFocus = SkullButton;
	}
	else if (bAxeHit)
	{
		SelectTutorial(EHronoTutorialItem::Axe);
		NewFocus = AxeButton;
	}
	else if (bCloseHit)
	{
		CloseMenu();
	}

	if (NewFocus)
	{
		NewFocus->SetUserFocus(GetOwningPlayer());
	}
	return true;
}

void UHronoTutorialWidget::BuildWidgetTree()
{
	using namespace HronoTutorial;

	UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("TutorialRoot"));
	WidgetTree->RootWidget = Root;

	MenuBlur = WidgetTree->ConstructWidget<UBackgroundBlur>(UBackgroundBlur::StaticClass(), TEXT("MenuBlur"));
	MenuBlur->SetBlurStrength(22.0f);
	MenuBlur->SetApplyAlphaToBlur(true);
	MenuBlur->SetPadding(FMargin(32.0f));
	MenuBlur->SetVisibility(ESlateVisibility::Collapsed);
	const FSlateColorBrush BlurFallback(FLinearColor(0.0f, 0.0f, 0.0f, 0.78f));
	MenuBlur->SetLowQualityFallbackBrush(BlurFallback);
	if (UOverlaySlot* LayoutSlot = Root->AddChildToOverlay(MenuBlur))
	{
		LayoutSlot->SetHorizontalAlignment(HAlign_Fill);
		LayoutSlot->SetVerticalAlignment(VAlign_Fill);
	}

	// Keep interactive content outside UBackgroundBlur. The blur is a visual-only
	// sibling, so it can never capture or disturb the menu's pointer routing.
	MenuLayer = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("MenuLayer"));
	MenuLayer->SetBrushColor(FLinearColor::Transparent);
	MenuLayer->SetPadding(FMargin(2.0f));
	MenuLayer->SetVisibility(ESlateVisibility::Collapsed);
	if (UOverlaySlot* LayoutSlot = Root->AddChildToOverlay(MenuLayer))
	{
		LayoutSlot->SetHorizontalAlignment(HAlign_Fill);
		LayoutSlot->SetVerticalAlignment(VAlign_Fill);
	}

	USizeBox* MenuSize = WidgetTree->ConstructWidget<USizeBox>();
	MenuSize->SetWidthOverride(1280.0f);
	MenuSize->SetHeightOverride(780.0f);
	if (UBorderSlot* LayoutSlot = Cast<UBorderSlot>(MenuLayer->AddChild(MenuSize)))
	{
		LayoutSlot->SetHorizontalAlignment(HAlign_Center);
		LayoutSlot->SetVerticalAlignment(VAlign_Center);
	}

	UHorizontalBox* MenuColumns = WidgetTree->ConstructWidget<UHorizontalBox>();
	MenuSize->AddChild(MenuColumns);

	UBorder* NavigationBorder = WidgetTree->ConstructWidget<UBorder>();
	NavigationBorder->SetBrushColor(FLinearColor(0.025f, 0.025f, 0.025f, 0.97f));
	NavigationBorder->SetPadding(FMargin(28.0f));
	if (UHorizontalBoxSlot* LayoutSlot = MenuColumns->AddChildToHorizontalBox(NavigationBorder))
	{
		LayoutSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		LayoutSlot->SetPadding(FMargin(0.0f, 0.0f, 2.0f, 0.0f));
	}

	USizeBox* NavigationSize = WidgetTree->ConstructWidget<USizeBox>();
	NavigationSize->SetWidthOverride(310.0f);
	NavigationBorder->AddChild(NavigationSize);
	UVerticalBox* NavigationBox = WidgetTree->ConstructWidget<UVerticalBox>();
	NavigationSize->AddChild(NavigationBox);

	UTextBlock* JournalTitle = MakeText(
		WidgetTree, NSLOCTEXT("HronoTutorial", "JournalTitle", "FIELD JOURNAL"), 28, Accent);
	if (UVerticalBoxSlot* LayoutSlot = NavigationBox->AddChildToVerticalBox(JournalTitle))
	{
		LayoutSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
	}
	StageText = MakeText(WidgetTree, FText::GetEmpty(), 14, TextMuted);
	if (UVerticalBoxSlot* LayoutSlot = NavigationBox->AddChildToVerticalBox(StageText))
	{
		LayoutSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 2.0f));
	}
	CollectionText = MakeText(WidgetTree, FText::GetEmpty(), 14, TextMuted);
	if (UVerticalBoxSlot* LayoutSlot = NavigationBox->AddChildToVerticalBox(CollectionText))
	{
		LayoutSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 28.0f));
	}

	MonocleButton = MakeNavigationButton(WidgetTree, NavigationBox, FText::GetEmpty(), TEXT("MonocleButton"));
	DosimeterButton = MakeNavigationButton(WidgetTree, NavigationBox, FText::GetEmpty(), TEXT("DosimeterButton"));
	ClockButton = MakeNavigationButton(WidgetTree, NavigationBox, FText::GetEmpty(), TEXT("ClockButton"));
	SkullButton = MakeNavigationButton(WidgetTree, NavigationBox, FText::GetEmpty(), TEXT("SkullButton"));
	AxeButton = MakeNavigationButton(WidgetTree, NavigationBox, FText::GetEmpty(), TEXT("AxeButton"));

	UTextBlock* NavigationHint = MakeText(
		WidgetTree,
		NSLOCTEXT("HronoTutorial", "NavigationHint", "Select any item to review its tutorial."),
		14, TextMuted);
	if (UVerticalBoxSlot* LayoutSlot = NavigationBox->AddChildToVerticalBox(NavigationHint))
	{
		LayoutSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		LayoutSlot->SetVerticalAlignment(VAlign_Bottom);
		LayoutSlot->SetPadding(FMargin(0.0f, 20.0f, 0.0f, 18.0f));
	}

	CloseButton = MakeNavigationButton(
		WidgetTree, NavigationBox, NSLOCTEXT("HronoTutorial", "Close", "CLOSE  [TAB / ESC]"), TEXT("CloseButton"));

	UBorder* ContentBorder = WidgetTree->ConstructWidget<UBorder>();
	ContentBorder->SetBrushColor(FLinearColor(0.012f, 0.012f, 0.012f, 0.97f));
	ContentBorder->SetPadding(FMargin(42.0f, 30.0f));
	if (UHorizontalBoxSlot* LayoutSlot = MenuColumns->AddChildToHorizontalBox(ContentBorder))
	{
		LayoutSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	UVerticalBox* Content = WidgetTree->ConstructWidget<UVerticalBox>();
	ContentBorder->AddChild(Content);

	UHorizontalBox* HeadingRow = WidgetTree->ConstructWidget<UHorizontalBox>();
	if (UVerticalBoxSlot* LayoutSlot = Content->AddChildToVerticalBox(HeadingRow))
	{
		LayoutSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 18.0f));
	}
	UVerticalBox* Heading = WidgetTree->ConstructWidget<UVerticalBox>();
	if (UHorizontalBoxSlot* LayoutSlot = HeadingRow->AddChildToHorizontalBox(Heading))
	{
		LayoutSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}
	ItemTitleText = MakeText(WidgetTree, FText::GetEmpty(), 36, TextPrimary);
	Heading->AddChildToVerticalBox(ItemTitleText);
	ItemSubtitleText = MakeText(WidgetTree, FText::GetEmpty(), 16, Accent);
	Heading->AddChildToVerticalBox(ItemSubtitleText);
	USizeBox* ImageSize = WidgetTree->ConstructWidget<USizeBox>();
	ImageSize->SetHeightOverride(315.0f);
	if (UVerticalBoxSlot* LayoutSlot = Content->AddChildToVerticalBox(ImageSize))
	{
		LayoutSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 22.0f));
		LayoutSlot->SetHorizontalAlignment(HAlign_Fill);
	}
	UOverlay* ImageOverlay = WidgetTree->ConstructWidget<UOverlay>();
	ImageSize->AddChild(ImageOverlay);
	UBorder* ImageBackdrop = WidgetTree->ConstructWidget<UBorder>();
	ImageBackdrop->SetBrushColor(FLinearColor(0.01f, 0.01f, 0.01f, 1.0f));
	ImageOverlay->AddChildToOverlay(ImageBackdrop);
	MissingImageText = MakeText(
		WidgetTree, NSLOCTEXT("HronoTutorial", "LoadingImage", "LOADING FIELD ILLUSTRATION..."), 15, TextMuted);
	MissingImageText->SetJustification(ETextJustify::Center);
	if (UOverlaySlot* LayoutSlot = ImageOverlay->AddChildToOverlay(MissingImageText))
	{
		LayoutSlot->SetHorizontalAlignment(HAlign_Center);
		LayoutSlot->SetVerticalAlignment(VAlign_Center);
	}
	UHorizontalBox* TutorialImageRow = WidgetTree->ConstructWidget<UHorizontalBox>();
	if (UOverlaySlot* LayoutSlot = ImageOverlay->AddChildToOverlay(TutorialImageRow))
	{
		LayoutSlot->SetHorizontalAlignment(HAlign_Fill);
		LayoutSlot->SetVerticalAlignment(VAlign_Fill);
	}

	TutorialImage = WidgetTree->ConstructWidget<UImage>();
	TutorialImage->SetVisibility(ESlateVisibility::Collapsed);
	if (UHorizontalBoxSlot* LayoutSlot = TutorialImageRow->AddChildToHorizontalBox(TutorialImage))
	{
		LayoutSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		LayoutSlot->SetVerticalAlignment(VAlign_Fill);
		LayoutSlot->SetPadding(FMargin(0.0f, 0.0f, 5.0f, 0.0f));
	}
	SecondaryTutorialImage = WidgetTree->ConstructWidget<UImage>();
	SecondaryTutorialImage->SetVisibility(ESlateVisibility::Collapsed);
	if (UHorizontalBoxSlot* LayoutSlot = TutorialImageRow->AddChildToHorizontalBox(SecondaryTutorialImage))
	{
		LayoutSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		LayoutSlot->SetVerticalAlignment(VAlign_Fill);
		LayoutSlot->SetPadding(FMargin(5.0f, 0.0f, 0.0f, 0.0f));
	}

	ItemDescriptionText = MakeText(WidgetTree, FText::GetEmpty(), 18, TextPrimary);
	if (UVerticalBoxSlot* LayoutSlot = Content->AddChildToVerticalBox(ItemDescriptionText))
	{
		LayoutSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 18.0f));
		LayoutSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}
	ItemControlText = MakeText(WidgetTree, FText::GetEmpty(), 16, Accent);
	Content->AddChildToVerticalBox(ItemControlText);

	PickupPrompt = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("PickupPrompt"));
	PickupPrompt->SetBrushColor(FLinearColor::Transparent);
	PickupPrompt->SetPadding(FMargin(0.0f));
	PickupPrompt->SetVisibility(ESlateVisibility::Collapsed);
	if (UOverlaySlot* LayoutSlot = Root->AddChildToOverlay(PickupPrompt))
	{
		LayoutSlot->SetHorizontalAlignment(HAlign_Right);
		LayoutSlot->SetVerticalAlignment(VAlign_Top);
		LayoutSlot->SetPadding(FMargin(0.0f, 48.0f, 48.0f, 0.0f));
	}
	PickupPromptText = MakeText(WidgetTree, FText::GetEmpty(), 19, TextPrimary);
	PickupPromptText->SetJustification(ETextJustify::Center);
	PickupPrompt->AddChild(PickupPromptText);
}

void UHronoTutorialWidget::BindButtons()
{
	if (MonocleButton) MonocleButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleMonocleClicked);
	if (DosimeterButton) DosimeterButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleDosimeterClicked);
	if (ClockButton) ClockButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleClockClicked);
	if (SkullButton) SkullButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleSkullClicked);
	if (AxeButton) AxeButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleAxeClicked);
	if (CloseButton) CloseButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleCloseClicked);
}

void UHronoTutorialWidget::ShowPickupPrompt(EHronoTutorialItem Item)
{
	if (Item == EHronoTutorialItem::None)
	{
		return;
	}

	PendingPromptItem = Item;
	SetItemDiscovered(Item);
	if (PickupPromptText)
	{
		PickupPromptText->SetText(
			NSLOCTEXT("HronoTutorial", "PickupPrompt", "Press Tab to read more"));
	}
	if (PickupPrompt)
	{
		PickupPrompt->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			PromptTimer, this, &ThisClass::HidePickupPrompt, 8.0f, false);
	}
}

void UHronoTutorialWidget::SetItemDiscovered(EHronoTutorialItem Item, bool bDiscovered)
{
	if (Item == EHronoTutorialItem::None)
	{
		return;
	}
	if (bDiscovered) DiscoveredItems.Add(Item);
	else DiscoveredItems.Remove(Item);
	RefreshNavigation();
	if (SelectedItem == Item) SelectTutorial(Item);
}

void UHronoTutorialWidget::SetGameStageText(const FText& NewStageText)
{
	if (StageText)
	{
		StageText->SetText(NewStageText.IsEmpty()
			? NSLOCTEXT("HronoTutorial", "DefaultStage", "STAGE  •  INVESTIGATION")
			: NewStageText);
	}
}

void UHronoTutorialWidget::ToggleMenu()
{
	if (bMenuOpen)
	{
		CloseMenu();
	}
	else
	{
		OpenMenu(PendingPromptItem);
	}
}

void UHronoTutorialWidget::OpenMenu(EHronoTutorialItem Item)
{
	if (Item == EHronoTutorialItem::None)
	{
		Item = SelectedItem;
	}
	PendingPromptItem = EHronoTutorialItem::None;
	HidePickupPrompt();
	SelectTutorial(Item);

	if (MenuBlur)
	{
		MenuBlur->SetVisibility(ESlateVisibility::HitTestInvisible);
		MenuBlur->SetIsEnabled(true);
	}
	if (MenuLayer)
	{
		MenuLayer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		MenuLayer->SetIsEnabled(true);
	}
	SetIsEnabled(true);
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	bMenuOpen = true;
	RegisterMenuInputPreProcessor();
	if (APlayerController* Controller = GetOwningPlayer())
	{
		bPreviousMouseCursor = Controller->bShowMouseCursor;
		bPreviousClickEvents = Controller->bEnableClickEvents;
		bPreviousMouseOverEvents = Controller->bEnableMouseOverEvents;
		if (!UGameplayStatics::IsGamePaused(this))
		{
			bOwnsGamePause = Controller->SetPause(true);
		}
		Controller->bShowMouseCursor = true;
		Controller->bEnableClickEvents = true;
		Controller->bEnableMouseOverEvents = true;
		Controller->SetIgnoreMoveInput(true);
		Controller->SetIgnoreLookInput(true);
		Controller->FlushPressedKeys();
		FSlateApplication::Get().ReleaseAllPointerCapture();
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		Controller->SetInputMode(InputMode);
		if (UButton* FocusButton = Item == EHronoTutorialItem::Dosimeter ? DosimeterButton
			: Item == EHronoTutorialItem::Clock ? ClockButton
			: Item == EHronoTutorialItem::Skull ? SkullButton
			: Item == EHronoTutorialItem::Axe ? AxeButton
			: MonocleButton)
		{
			FocusButton->SetUserFocus(Controller);
		}
	}
}

void UHronoTutorialWidget::CloseMenu()
{
	if (!bMenuOpen)
	{
		return;
	}
	if (MenuBlur)
	{
		MenuBlur->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (MenuLayer)
	{
		MenuLayer->SetVisibility(ESlateVisibility::Collapsed);
	}
	UnregisterMenuInputPreProcessor();
	bMenuOpen = false;
	if (APlayerController* Controller = GetOwningPlayer())
	{
		if (bOwnsGamePause)
		{
			Controller->SetPause(false);
			bOwnsGamePause = false;
		}
		Controller->bShowMouseCursor = bPreviousMouseCursor;
		Controller->bEnableClickEvents = bPreviousClickEvents;
		Controller->bEnableMouseOverEvents = bPreviousMouseOverEvents;
		Controller->SetIgnoreMoveInput(false);
		Controller->SetIgnoreLookInput(false);
		FInputModeGameOnly InputMode;
		Controller->SetInputMode(InputMode);
	}
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UHronoTutorialWidget::RegisterMenuInputPreProcessor()
{
	if (MenuInputPreProcessor.IsValid() || !FSlateApplication::IsInitialized())
	{
		return;
	}

	MenuInputPreProcessor = MakeShared<HronoTutorial::FTutorialInputPreProcessor>(this);
	FSlateApplication::Get().RegisterInputPreProcessor(MenuInputPreProcessor, 0);
}

void UHronoTutorialWidget::UnregisterMenuInputPreProcessor()
{
	if (!MenuInputPreProcessor.IsValid())
	{
		return;
	}
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(MenuInputPreProcessor);
	}
	MenuInputPreProcessor.Reset();
}

void UHronoTutorialWidget::SelectTutorial(EHronoTutorialItem Item)
{
	if (Item == EHronoTutorialItem::None)
	{
		Item = EHronoTutorialItem::Monocle;
	}
	SelectedItem = Item;
	const HronoTutorial::FTutorialPage Page = HronoTutorial::GetPage(Item);
	if (ItemTitleText) ItemTitleText->SetText(Page.Title);
	if (ItemSubtitleText) ItemSubtitleText->SetText(Page.Subtitle);
	if (ItemDescriptionText) ItemDescriptionText->SetText(Page.Description);
	if (ItemControlText) ItemControlText->SetText(Page.Controls);

	if (Item == EHronoTutorialItem::Monocle
		|| Item == EHronoTutorialItem::Dosimeter
		|| Item == EHronoTutorialItem::Clock)
	{
		UTexture2D* CorrectTexture = Item == EHronoTutorialItem::Monocle ? MonocleCorrectTexture.Get()
			: Item == EHronoTutorialItem::Dosimeter ? DosimeterCorrectTexture.Get()
			: ClockCorrectTexture.Get();
		UTexture2D* WrongTexture = Item == EHronoTutorialItem::Monocle ? MonocleWrongTexture.Get()
			: Item == EHronoTutorialItem::Dosimeter ? DosimeterWrongTexture.Get()
			: ClockWrongTexture.Get();
		if (TutorialImage && CorrectTexture)
		{
			TutorialImage->SetBrushFromTexture(CorrectTexture, false);
			TutorialImage->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else if (TutorialImage)
		{
			TutorialImage->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (SecondaryTutorialImage && WrongTexture)
		{
			SecondaryTutorialImage->SetBrushFromTexture(WrongTexture, false);
			SecondaryTutorialImage->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else if (SecondaryTutorialImage)
		{
			SecondaryTutorialImage->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (MissingImageText)
		{
			MissingImageText->SetVisibility(
				CorrectTexture && WrongTexture
					? ESlateVisibility::Collapsed
					: ESlateVisibility::HitTestInvisible);
		}
	}
	else if (UTexture2D* Texture = LoadTutorialTexture(Item))
	{
		if (TutorialImage)
		{
			TutorialImage->SetBrushFromTexture(Texture, false);
			TutorialImage->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		if (SecondaryTutorialImage) SecondaryTutorialImage->SetVisibility(ESlateVisibility::Collapsed);
		if (MissingImageText) MissingImageText->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		if (TutorialImage) TutorialImage->SetVisibility(ESlateVisibility::Collapsed);
		if (SecondaryTutorialImage) SecondaryTutorialImage->SetVisibility(ESlateVisibility::Collapsed);
		if (MissingImageText)
		{
			MissingImageText->SetText(NSLOCTEXT("HronoTutorial", "MissingImage", "FIELD ILLUSTRATION UNAVAILABLE"));
			MissingImageText->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}
	RefreshNavigation();
}

void UHronoTutorialWidget::RefreshNavigation()
{
	if (CollectionText)
	{
		CollectionText->SetText(FText::Format(
			NSLOCTEXT("HronoTutorial", "CollectionProgress", "EQUIPMENT FOUND  •  {0} / {1}"),
			FText::AsNumber(DiscoveredItems.Num()), FText::AsNumber(HronoTutorial::TutorialCount)));
	}

	const auto RefreshButton = [this](UButton* Button, EHronoTutorialItem Item)
	{
		if (!Button) return;
		Button->SetIsEnabled(true);
		Button->SetVisibility(ESlateVisibility::Visible);
		if (UTextBlock* Label = Cast<UTextBlock>(Button->GetContent()))
		{
			Label->SetText(HronoTutorial::ItemName(Item));
			Label->SetColorAndOpacity(FSlateColor(
				Item == SelectedItem ? HronoTutorial::Accent : HronoTutorial::TextPrimary));
		}
		// Selection is indicated by text only; keep the button background neutral.
		Button->SetBackgroundColor(FLinearColor::White);
	};

	RefreshButton(MonocleButton, EHronoTutorialItem::Monocle);
	RefreshButton(DosimeterButton, EHronoTutorialItem::Dosimeter);
	RefreshButton(ClockButton, EHronoTutorialItem::Clock);
	RefreshButton(SkullButton, EHronoTutorialItem::Skull);
	RefreshButton(AxeButton, EHronoTutorialItem::Axe);
}

void UHronoTutorialWidget::HidePickupPrompt()
{
	if (PickupPrompt)
	{
		PickupPrompt->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(PromptTimer);
	}
}

UTexture2D* UHronoTutorialWidget::LoadTutorialTexture(EHronoTutorialItem Item)
{
	if (TObjectPtr<UTexture2D>* Existing = LoadedTextures.Find(Item))
	{
		return Existing->Get();
	}

	const FString ImagePath = FPaths::Combine(
		FPaths::ProjectContentDir(), TEXT("_Alex/Images/Tutorial/Runtime"),
		HronoTutorial::GetPage(Item).ImageFilename);
	UTexture2D* Texture = FImageUtils::ImportFileAsTexture2D(ImagePath);
	if (Texture)
	{
		LoadedTextures.Add(Item, Texture);
	}
	return Texture;
}

void UHronoTutorialWidget::HandleMonocleClicked()
{
	SelectTutorial(EHronoTutorialItem::Monocle);
}
void UHronoTutorialWidget::HandleDosimeterClicked()
{
	SelectTutorial(EHronoTutorialItem::Dosimeter);
}
void UHronoTutorialWidget::HandleClockClicked()
{
	SelectTutorial(EHronoTutorialItem::Clock);
}
void UHronoTutorialWidget::HandleSkullClicked()
{
	SelectTutorial(EHronoTutorialItem::Skull);
}
void UHronoTutorialWidget::HandleAxeClicked()
{
	SelectTutorial(EHronoTutorialItem::Axe);
}
void UHronoTutorialWidget::HandleCloseClicked()
{
	CloseMenu();
}
