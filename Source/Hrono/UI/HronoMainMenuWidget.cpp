// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/HronoMainMenuWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateColorBrush.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/WidgetSwitcher.h"
#include "EnhancedActionKeyMapping.h"
#include "Engine/Font.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Sound/AudioSettings.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "UI/HronoLoadingSubsystem.h"
#include "UI/HronoMenuSettingsSaveGame.h"
#include "UObject/ConstructorHelpers.h"

const FString UHronoMainMenuWidget::AudioSettingsSlot(TEXT("HronoMenuSettings"));

namespace HronoMenu
{
	TArray<TWeakObjectPtr<UHronoMainMenuWidget>> ActiveMenuInstances;

	UTextBlock* MakeText(UWidgetTree* Tree, UFont* FontObject, const FText& Text, int32 Size = 18, bool bBold = false)
	{
		UTextBlock* Label = Tree->ConstructWidget<UTextBlock>();
		Label->SetText(Text);
		Label->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.9f, 0.92f)));
		FSlateFontInfo Font = FontObject
			? FSlateFontInfo(FontObject, Size, bBold ? FName(TEXT("Bold")) : FName(TEXT("Regular")))
			: Label->GetFont();
		Font.Size = static_cast<float>(Size);
		Label->SetFont(Font);
		Label->SetShadowOffset(FVector2D(1.0f, 1.0f));
		Label->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.8f));
		return Label;
	}

	void ApplyTextOnlyStyle(UButton* Button)
	{
		// Keep a real Slate button brush for stable painting and hit testing. Its
		// alpha is zero, so the button box remains completely invisible.
		const FSlateColorBrush InvisibleBrush(FLinearColor::Transparent);
		FButtonStyle TextOnlyStyle;
		TextOnlyStyle
			.SetNormal(InvisibleBrush)
			.SetHovered(InvisibleBrush)
			.SetPressed(InvisibleBrush)
			.SetDisabled(InvisibleBrush)
			.SetNormalPadding(FMargin(0.0f))
			.SetPressedPadding(FMargin(0.0f));
		Button->SetStyle(TextOnlyStyle);
		Button->SetBackgroundColor(FLinearColor::White);
	}

	UButton* MakeButton(UWidgetTree* Tree, UFont* FontObject, UVerticalBox* Parent, FName Name, const FText& Text)
	{
		// Keep pointer interaction on a fixed rectangle. The text is free to animate
		// inside it without changing or leaving the button's hover geometry.
		USizeBox* HitArea = Tree->ConstructWidget<USizeBox>();
		HitArea->SetWidthOverride(380.0f);
		HitArea->SetHeightOverride(58.0f);

		UButton* Button = Tree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
		ApplyTextOnlyStyle(Button);
		UTextBlock* ButtonText = MakeText(Tree, FontObject, Text, 24, true);
		ButtonText->SetVisibility(ESlateVisibility::HitTestInvisible);
		if (UButtonSlot* ContentSlot = Cast<UButtonSlot>(Button->AddChild(ButtonText)))
		{
			ContentSlot->SetPadding(FMargin(18.0f, 10.0f));
			ContentSlot->SetHorizontalAlignment(HAlign_Center);
			ContentSlot->SetVerticalAlignment(VAlign_Center);
		}
		HitArea->AddChild(Button);
		if (UVerticalBoxSlot* Slot = Parent->AddChildToVerticalBox(HitArea))
		{
			Slot->SetPadding(FMargin(0.0f, 3.0f));
			Slot->SetHorizontalAlignment(HAlign_Center);
		}
		return Button;
	}

	UHorizontalBox* MakeSettingRow(UWidgetTree* Tree, UFont* FontObject, UVerticalBox* Parent, const FText& LabelText)
	{
		UHorizontalBox* Row = Tree->ConstructWidget<UHorizontalBox>();
		if (UVerticalBoxSlot* RowSlot = Parent->AddChildToVerticalBox(Row))
		{
			RowSlot->SetPadding(FMargin(0.0f, 3.0f));
		}

		UTextBlock* Label = MakeText(Tree, FontObject, LabelText, 16);
		if (UHorizontalBoxSlot* LabelSlot = Row->AddChildToHorizontalBox(Label))
		{
			LabelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			LabelSlot->SetVerticalAlignment(VAlign_Center);
		}
		return Row;
	}

	void AddRowControl(UHorizontalBox* Row, UWidget* Control)
	{
		if (UHorizontalBoxSlot* Slot = Row->AddChildToHorizontalBox(Control))
		{
			Slot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			Slot->SetVerticalAlignment(VAlign_Center);
		}
	}
}

UHronoMainMenuWidget::UHronoMainMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UFont> HorrorEngineFont(
		TEXT("/Game/HorrorEngine/Blueprints/Widgets/Fonts/HorrorEngine_Font.HorrorEngine_Font"));
	if (HorrorEngineFont.Succeeded())
	{
		MenuFont = HorrorEngineFont.Object;
	}

	ControlMappingContexts.Add(TSoftObjectPtr<UInputMappingContext>(
		FSoftObjectPath(TEXT("/Game/_Alex/IMC_HE_Hrono.IMC_HE_Hrono"))));

	GameplayMapToPreload = TSoftObjectPtr<UWorld>(
		FSoftObjectPath(TEXT("/Game/_Alex/DemoMap1.DemoMap1")));
}

TSharedRef<SWidget> UHronoMainMenuWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		BuildDefaultWidgetTree();
	}

	return Super::RebuildWidget();
}

void UHronoMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Level Blueprint execution or network PIE can construct the same menu more
	// than once. With a transparent main page those instances look like two text
	// colors when only the top one animates. Keep one menu per local player.
	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		for (const TWeakObjectPtr<UHronoMainMenuWidget>& MenuReference : HronoMenu::ActiveMenuInstances)
		{
			UHronoMainMenuWidget* ExistingMenu = MenuReference.Get();
			if (ExistingMenu && ExistingMenu != this && ExistingMenu->GetOwningLocalPlayer() == LocalPlayer)
			{
				ExistingMenu->RemoveFromParent();
			}
		}
		HronoMenu::ActiveMenuInstances.RemoveAll([LocalPlayer, this](const TWeakObjectPtr<UHronoMainMenuWidget>& MenuReference)
		{
			const UHronoMainMenuWidget* ExistingMenu = MenuReference.Get();
			return !ExistingMenu || ExistingMenu == this || ExistingMenu->GetOwningLocalPlayer() == LocalPlayer;
		});
		HronoMenu::ActiveMenuInstances.Add(this);
	}

	ResolveNamedWidgets();
	PopulateGraphicsSettings();
	LoadAudioSettings();
	RefreshControlsList();
	BindWidgetEvents();
	ApplyAudioSettings(0.0f);
	ShowMainMenu();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UHronoLoadingSubsystem* LoadingSubsystem = GameInstance->GetSubsystem<UHronoLoadingSubsystem>())
		{
			LoadingSubsystem->OnPreloadCompleted.AddUniqueDynamic(
				this, &ThisClass::HandleGameplayPreloadCompleted);
		}
	}

	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		PlayerController->bShowMouseCursor = true;
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PlayerController->SetInputMode(InputMode);
	}
}

void UHronoMainMenuWidget::NativeDestruct()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UHronoLoadingSubsystem* LoadingSubsystem = GameInstance->GetSubsystem<UHronoLoadingSubsystem>())
		{
			LoadingSubsystem->OnPreloadCompleted.RemoveDynamic(
				this, &ThisClass::HandleGameplayPreloadCompleted);
		}
	}

	Super::NativeDestruct();
}

void UHronoMainMenuWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const FLinearColor IdleColor(0.9f, 0.9f, 0.92f, 1.0f);
	for (int32 Index = AnimatedButtons.Num() - 1; Index >= 0; --Index)
	{
		UButton* Button = AnimatedButtons[Index].Get();
		if (!Button || !ButtonHoverAlphas.IsValidIndex(Index) || !ButtonScaleValues.IsValidIndex(Index))
		{
			AnimatedButtons.RemoveAt(Index);
			if (ButtonHoverAlphas.IsValidIndex(Index)) ButtonHoverAlphas.RemoveAt(Index);
			if (ButtonScaleValues.IsValidIndex(Index)) ButtonScaleValues.RemoveAt(Index);
			continue;
		}

		UTextBlock* ButtonText = Cast<UTextBlock>(Button->GetContent());
		const bool bHighlighted = Button->IsHovered() || Button->IsPressed();
		const float HoverTarget = bHighlighted ? 1.0f : 0.0f;
		ButtonHoverAlphas[Index] = FMath::FInterpTo(
			ButtonHoverAlphas[Index], HoverTarget, InDeltaTime, ButtonAnimationSpeed);

		const float TargetScale = Button->IsPressed()
			? ButtonPressedScale
			: FMath::Lerp(1.0f, ButtonHoverScale, ButtonHoverAlphas[Index]);
		ButtonScaleValues[Index] = FMath::FInterpTo(
			ButtonScaleValues[Index], TargetScale, InDeltaTime, ButtonAnimationSpeed * 1.35f);

		if (ButtonText)
		{
			// Animate the only text widget, not its transparent button container. Keeping
			// the hit box stationary also prevents hover oscillation near its edges.
			ButtonText->SetRenderScale(FVector2D(ButtonScaleValues[Index]));
			ButtonText->SetRenderTranslation(FVector2D(ButtonHoverOffset * ButtonHoverAlphas[Index], 0.0f));
			ButtonText->SetRenderOpacity(FMath::Lerp(ButtonIdleOpacity, 1.0f, ButtonHoverAlphas[Index]));
			ButtonText->SetColorAndOpacity(FSlateColor(bHighlighted ? ButtonHoverColor : IdleColor));
			// Flush the previous white draw state before Slate paints the hover color.
			ButtonText->InvalidateLayoutAndVolatility();
		}
	}
}

void UHronoMainMenuWidget::BuildDefaultWidgetTree()
{
	using namespace HronoMenu;

	UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("MainMenuRoot"));
	WidgetTree->RootWidget = Root;

	PageSwitcher = WidgetTree->ConstructWidget<UWidgetSwitcher>(UWidgetSwitcher::StaticClass(), TEXT("PageSwitcher"));
	if (UOverlaySlot* PanelSlot = Root->AddChildToOverlay(PageSwitcher))
	{
		PanelSlot->SetHorizontalAlignment(HAlign_Fill);
		PanelSlot->SetVerticalAlignment(VAlign_Fill);
	}

	UOverlay* MainPage = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("MainPage"));
	PageSwitcher->AddChild(MainPage);
	USizeBox* MainSize = WidgetTree->ConstructWidget<USizeBox>();
	MainSize->SetWidthOverride(440.0f);
	if (UOverlaySlot* PanelSlot = MainPage->AddChildToOverlay(MainSize))
	{
		PanelSlot->SetHorizontalAlignment(HAlign_Center);
		PanelSlot->SetVerticalAlignment(VAlign_Center);
	}

	UVerticalBox* MainButtons = WidgetTree->ConstructWidget<UVerticalBox>();
	MainSize->AddChild(MainButtons);

	UTextBlock* Title = MakeText(WidgetTree, MenuFont, FText::FromString(TEXT("DIVIDED")), 48, true);
	Title->SetJustification(ETextJustify::Center);
	if (UVerticalBoxSlot* PanelSlot = MainButtons->AddChildToVerticalBox(Title))
	{
		PanelSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 28.0f));
	}

	CreateSessionButton = MakeButton(WidgetTree, MenuFont, MainButtons, TEXT("CreateSessionButton"), FText::FromString(TEXT("CREATE SESSION")));
	JoinSessionButton = MakeButton(WidgetTree, MenuFont, MainButtons, TEXT("JoinSessionButton"), FText::FromString(TEXT("JOIN SESSION")));
	OptionsButton = MakeButton(WidgetTree, MenuFont, MainButtons, TEXT("OptionsButton"), FText::FromString(TEXT("OPTIONS")));
	ExitButton = MakeButton(WidgetTree, MenuFont, MainButtons, TEXT("ExitButton"), FText::FromString(TEXT("EXIT")));

	UOverlay* OptionsPage = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("OptionsPage"));
	PageSwitcher->AddChild(OptionsPage);
	USizeBox* OptionsSize = WidgetTree->ConstructWidget<USizeBox>();
	OptionsSize->SetWidthOverride(1180.0f);
	OptionsSize->SetHeightOverride(800.0f);
	if (UOverlaySlot* PanelSlot = OptionsPage->AddChildToOverlay(OptionsSize))
	{
		PanelSlot->SetHorizontalAlignment(HAlign_Center);
		PanelSlot->SetVerticalAlignment(VAlign_Center);
	}

	UBorder* OptionsPanel = WidgetTree->ConstructWidget<UBorder>();
	OptionsPanel->SetPadding(FMargin(32.0f));
	OptionsPanel->SetBrushColor(FLinearColor(0.035f, 0.04f, 0.055f, 0.99f));
	OptionsSize->AddChild(OptionsPanel);
	UVerticalBox* OptionsLayout = WidgetTree->ConstructWidget<UVerticalBox>();
	OptionsPanel->AddChild(OptionsLayout);

	UTextBlock* OptionsTitle = MakeText(WidgetTree, MenuFont, FText::FromString(TEXT("OPTIONS")), 32, true);
	if (UVerticalBoxSlot* PanelSlot = OptionsLayout->AddChildToVerticalBox(OptionsTitle))
	{
		PanelSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 20.0f));
	}

	UHorizontalBox* Columns = WidgetTree->ConstructWidget<UHorizontalBox>();
	if (UVerticalBoxSlot* PanelSlot = OptionsLayout->AddChildToVerticalBox(Columns))
	{
		PanelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	UVerticalBox* SettingsColumn = WidgetTree->ConstructWidget<UVerticalBox>();
	if (UHorizontalBoxSlot* PanelSlot = Columns->AddChildToHorizontalBox(SettingsColumn))
	{
		PanelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		PanelSlot->SetPadding(FMargin(0.0f, 0.0f, 30.0f, 0.0f));
	}

	UTextBlock* GraphicsHeading = MakeText(WidgetTree, MenuFont, FText::FromString(TEXT("GRAPHICS")), 22, true);
	SettingsColumn->AddChildToVerticalBox(GraphicsHeading)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));

	UHorizontalBox* ResolutionRow = MakeSettingRow(WidgetTree, MenuFont, SettingsColumn, FText::FromString(TEXT("Resolution")));
	ResolutionCombo = WidgetTree->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass(), TEXT("ResolutionCombo"));
	AddRowControl(ResolutionRow, ResolutionCombo);

	UHorizontalBox* WindowModeRow = MakeSettingRow(WidgetTree, MenuFont, SettingsColumn, FText::FromString(TEXT("Display mode")));
	WindowModeCombo = WidgetTree->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass(), TEXT("WindowModeCombo"));
	WindowModeCombo->AddOption(TEXT("Fullscreen"));
	WindowModeCombo->AddOption(TEXT("Borderless"));
	WindowModeCombo->AddOption(TEXT("Windowed"));
	AddRowControl(WindowModeRow, WindowModeCombo);

	UHorizontalBox* QualityRow = MakeSettingRow(WidgetTree, MenuFont, SettingsColumn, FText::FromString(TEXT("Quality preset")));
	QualityCombo = WidgetTree->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass(), TEXT("QualityCombo"));
	QualityCombo->AddOption(TEXT("Low"));
	QualityCombo->AddOption(TEXT("Medium"));
	QualityCombo->AddOption(TEXT("High"));
	QualityCombo->AddOption(TEXT("Epic"));
	QualityCombo->AddOption(TEXT("Cinematic"));
	QualityCombo->AddOption(TEXT("Custom"));
	AddRowControl(QualityRow, QualityCombo);

	UHorizontalBox* FrameRateRow = MakeSettingRow(WidgetTree, MenuFont, SettingsColumn, FText::FromString(TEXT("Frame-rate limit")));
	FrameRateCombo = WidgetTree->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass(), TEXT("FrameRateCombo"));
	for (const TCHAR* Option : { TEXT("30"), TEXT("60"), TEXT("120"), TEXT("144"), TEXT("Unlimited") })
	{
		FrameRateCombo->AddOption(Option);
	}
	AddRowControl(FrameRateRow, FrameRateCombo);

	UHorizontalBox* VSyncRow = MakeSettingRow(WidgetTree, MenuFont, SettingsColumn, FText::FromString(TEXT("VSync")));
	VSyncCheckBox = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass(), TEXT("VSyncCheckBox"));
	AddRowControl(VSyncRow, VSyncCheckBox);

	UTextBlock* AudioHeading = MakeText(WidgetTree, MenuFont, FText::FromString(TEXT("AUDIO")), 22, true);
	SettingsColumn->AddChildToVerticalBox(AudioHeading)->SetPadding(FMargin(0.0f, 22.0f, 0.0f, 10.0f));

	auto MakeVolumeRow = [this, SettingsColumn](const TCHAR* Label, const TCHAR* Name) -> USlider*
	{
		UHorizontalBox* Row = HronoMenu::MakeSettingRow(WidgetTree, MenuFont, SettingsColumn, FText::FromString(Label));
		USlider* Slider = WidgetTree->ConstructWidget<USlider>(USlider::StaticClass(), FName(Name));
		Slider->SetMinValue(0.0f);
		Slider->SetMaxValue(1.0f);
		Slider->SetStepSize(0.01f);
		HronoMenu::AddRowControl(Row, Slider);
		return Slider;
	};

	MasterVolumeSlider = MakeVolumeRow(TEXT("Master volume"), TEXT("MasterVolumeSlider"));
	MusicVolumeSlider = MakeVolumeRow(TEXT("Music volume"), TEXT("MusicVolumeSlider"));
	SfxVolumeSlider = MakeVolumeRow(TEXT("Effects volume"), TEXT("SfxVolumeSlider"));

	UVerticalBox* ControlsColumn = WidgetTree->ConstructWidget<UVerticalBox>();
	if (UHorizontalBoxSlot* PanelSlot = Columns->AddChildToHorizontalBox(ControlsColumn))
	{
		PanelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}
	ControlsColumn->AddChildToVerticalBox(MakeText(WidgetTree, MenuFont, FText::FromString(TEXT("CONTROLS")), 22, true))->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
	UScrollBox* ControlsScroll = WidgetTree->ConstructWidget<UScrollBox>();
	if (UVerticalBoxSlot* PanelSlot = ControlsColumn->AddChildToVerticalBox(ControlsScroll))
	{
		PanelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}
	ControlsList = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ControlsList"));
	ControlsScroll->AddChild(ControlsList);

	UHorizontalBox* Footer = WidgetTree->ConstructWidget<UHorizontalBox>();
	if (UVerticalBoxSlot* PanelSlot = OptionsLayout->AddChildToVerticalBox(Footer))
	{
		PanelSlot->SetPadding(FMargin(0.0f, 22.0f, 0.0f, 0.0f));
		PanelSlot->SetHorizontalAlignment(HAlign_Right);
	}
	BackButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("BackButton"));
	ApplyTextOnlyStyle(BackButton);
	UTextBlock* BackText = MakeText(WidgetTree, MenuFont, FText::FromString(TEXT("BACK")), 20, true);
	BackText->SetVisibility(ESlateVisibility::HitTestInvisible);
	if (UButtonSlot* ContentSlot = Cast<UButtonSlot>(BackButton->AddChild(BackText)))
	{
		ContentSlot->SetPadding(FMargin(48.0f, 10.0f));
	}
	USizeBox* BackHitArea = WidgetTree->ConstructWidget<USizeBox>();
	BackHitArea->SetWidthOverride(180.0f);
	BackHitArea->SetHeightOverride(48.0f);
	BackHitArea->AddChild(BackButton);
	Footer->AddChildToHorizontalBox(BackHitArea)->SetPadding(FMargin(6.0f));

	ApplyButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("ApplyButton"));
	ApplyTextOnlyStyle(ApplyButton);
	UTextBlock* ApplyText = MakeText(WidgetTree, MenuFont, FText::FromString(TEXT("APPLY")), 20, true);
	ApplyText->SetVisibility(ESlateVisibility::HitTestInvisible);
	if (UButtonSlot* ContentSlot = Cast<UButtonSlot>(ApplyButton->AddChild(ApplyText)))
	{
		ContentSlot->SetPadding(FMargin(48.0f, 10.0f));
	}
	USizeBox* ApplyHitArea = WidgetTree->ConstructWidget<USizeBox>();
	ApplyHitArea->SetWidthOverride(180.0f);
	ApplyHitArea->SetHeightOverride(48.0f);
	ApplyHitArea->AddChild(ApplyButton);
	Footer->AddChildToHorizontalBox(ApplyHitArea)->SetPadding(FMargin(6.0f));
}

void UHronoMainMenuWidget::ResolveNamedWidgets()
{
	if (!WidgetTree)
	{
		return;
	}

	auto Find = [this](const TCHAR* Name) { return WidgetTree->FindWidget(FName(Name)); };
	if (!PageSwitcher) PageSwitcher = Cast<UWidgetSwitcher>(Find(TEXT("PageSwitcher")));
	if (!CreateSessionButton) CreateSessionButton = Cast<UButton>(Find(TEXT("CreateSessionButton")));
	if (!JoinSessionButton) JoinSessionButton = Cast<UButton>(Find(TEXT("JoinSessionButton")));
	if (!OptionsButton) OptionsButton = Cast<UButton>(Find(TEXT("OptionsButton")));
	if (!ExitButton) ExitButton = Cast<UButton>(Find(TEXT("ExitButton")));
	if (!ApplyButton) ApplyButton = Cast<UButton>(Find(TEXT("ApplyButton")));
	if (!BackButton) BackButton = Cast<UButton>(Find(TEXT("BackButton")));
	if (!ResolutionCombo) ResolutionCombo = Cast<UComboBoxString>(Find(TEXT("ResolutionCombo")));
	if (!WindowModeCombo) WindowModeCombo = Cast<UComboBoxString>(Find(TEXT("WindowModeCombo")));
	if (!QualityCombo) QualityCombo = Cast<UComboBoxString>(Find(TEXT("QualityCombo")));
	if (!FrameRateCombo) FrameRateCombo = Cast<UComboBoxString>(Find(TEXT("FrameRateCombo")));
	if (!VSyncCheckBox) VSyncCheckBox = Cast<UCheckBox>(Find(TEXT("VSyncCheckBox")));
	if (!MasterVolumeSlider) MasterVolumeSlider = Cast<USlider>(Find(TEXT("MasterVolumeSlider")));
	if (!MusicVolumeSlider) MusicVolumeSlider = Cast<USlider>(Find(TEXT("MusicVolumeSlider")));
	if (!SfxVolumeSlider) SfxVolumeSlider = Cast<USlider>(Find(TEXT("SfxVolumeSlider")));
	if (!ControlsList) ControlsList = Cast<UVerticalBox>(Find(TEXT("ControlsList")));
}

void UHronoMainMenuWidget::BindWidgetEvents()
{
	if (bEventsBound)
	{
		return;
	}

	if (CreateSessionButton) CreateSessionButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleCreateSessionClicked);
	if (JoinSessionButton) JoinSessionButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleJoinSessionClicked);
	if (OptionsButton) OptionsButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleOptionsClicked);
	if (ExitButton) ExitButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleExitClicked);
	if (ApplyButton) ApplyButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleApplyClicked);
	if (BackButton) BackButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleBackClicked);

	AnimatedButtons.Reset();
	ButtonHoverAlphas.Reset();
	ButtonScaleValues.Reset();
	RegisterAnimatedButton(CreateSessionButton);
	RegisterAnimatedButton(JoinSessionButton);
	RegisterAnimatedButton(OptionsButton);
	RegisterAnimatedButton(ExitButton);
	RegisterAnimatedButton(ApplyButton);
	RegisterAnimatedButton(BackButton);
	if (MasterVolumeSlider) MasterVolumeSlider->OnValueChanged.AddUniqueDynamic(this, &ThisClass::HandleMasterVolumeChanged);
	if (MusicVolumeSlider) MusicVolumeSlider->OnValueChanged.AddUniqueDynamic(this, &ThisClass::HandleMusicVolumeChanged);
	if (SfxVolumeSlider) SfxVolumeSlider->OnValueChanged.AddUniqueDynamic(this, &ThisClass::HandleSfxVolumeChanged);
	bEventsBound = true;
}

void UHronoMainMenuWidget::RegisterAnimatedButton(UButton* Button)
{
	if (!Button)
	{
		return;
	}

	Button->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	Button->SetRenderScale(FVector2D(1.0f));
	Button->SetRenderTranslation(FVector2D::ZeroVector);
	Button->SetRenderOpacity(1.0f);
	Button->ForceVolatile(false);
	if (UTextBlock* ButtonText = Cast<UTextBlock>(Button->GetContent()))
	{
		ButtonText->SetVisibility(ESlateVisibility::HitTestInvisible);
		ButtonText->ForceVolatile(false);
		ButtonText->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
		ButtonText->SetRenderOpacity(ButtonIdleOpacity);
		ButtonText->SetShadowOffset(FVector2D::ZeroVector);
		ButtonText->SetShadowColorAndOpacity(FLinearColor::Transparent);
		ButtonText->InvalidateLayoutAndVolatility();
	}
	Button->OnPressed.AddUniqueDynamic(this, &ThisClass::HandleButtonPressed);
	AnimatedButtons.Add(Button);
	ButtonHoverAlphas.Add(0.0f);
	ButtonScaleValues.Add(1.0f);
}

void UHronoMainMenuWidget::PopulateGraphicsSettings()
{
	UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!Settings)
	{
		return;
	}

	if (ResolutionCombo)
	{
		ResolutionCombo->ClearOptions();
		TArray<FIntPoint> Resolutions;
		UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);
		const FIntPoint Current = Settings->GetScreenResolution();
		if (!Resolutions.Contains(Current))
		{
			Resolutions.Add(Current);
		}
		Resolutions.Sort([](const FIntPoint& A, const FIntPoint& B)
		{
			return A.X == B.X ? A.Y < B.Y : A.X < B.X;
		});
		for (const FIntPoint& Resolution : Resolutions)
		{
			ResolutionCombo->AddOption(FString::Printf(TEXT("%d x %d"), Resolution.X, Resolution.Y));
		}
		ResolutionCombo->SetSelectedOption(FString::Printf(TEXT("%d x %d"), Current.X, Current.Y));
	}

	if (WindowModeCombo)
	{
		const TCHAR* Mode = TEXT("Windowed");
		if (Settings->GetFullscreenMode() == EWindowMode::Fullscreen) Mode = TEXT("Fullscreen");
		else if (Settings->GetFullscreenMode() == EWindowMode::WindowedFullscreen) Mode = TEXT("Borderless");
		WindowModeCombo->SetSelectedOption(Mode);
	}

	if (QualityCombo)
	{
		static const TCHAR* QualityNames[] = { TEXT("Low"), TEXT("Medium"), TEXT("High"), TEXT("Epic"), TEXT("Cinematic") };
		const int32 Quality = Settings->GetOverallScalabilityLevel();
		QualityCombo->SetSelectedOption(Quality >= 0 && Quality <= 4 ? QualityNames[Quality] : TEXT("Custom"));
	}

	if (FrameRateCombo)
	{
		const float Limit = Settings->GetFrameRateLimit();
		const FString SelectedLimit = Limit <= 0.0f ? TEXT("Unlimited") : FString::FromInt(FMath::RoundToInt(Limit));
		if (FrameRateCombo->FindOptionIndex(SelectedLimit) == INDEX_NONE)
		{
			FrameRateCombo->AddOption(SelectedLimit);
		}
		FrameRateCombo->SetSelectedOption(SelectedLimit);
	}

	if (VSyncCheckBox)
	{
		VSyncCheckBox->SetIsChecked(Settings->IsVSyncEnabled());
	}
}

void UHronoMainMenuWidget::LoadAudioSettings()
{
	if (UGameplayStatics::DoesSaveGameExist(AudioSettingsSlot, 0))
	{
		if (const UHronoMenuSettingsSaveGame* Save = Cast<UHronoMenuSettingsSaveGame>(
			UGameplayStatics::LoadGameFromSlot(AudioSettingsSlot, 0)))
		{
			MasterVolume = FMath::Clamp(Save->MasterVolume, 0.0f, 1.0f);
			MusicVolume = FMath::Clamp(Save->MusicVolume, 0.0f, 1.0f);
			SfxVolume = FMath::Clamp(Save->SfxVolume, 0.0f, 1.0f);
		}
	}

	OriginalMasterVolume = MasterVolume;
	OriginalMusicVolume = MusicVolume;
	OriginalSfxVolume = SfxVolume;
	if (MasterVolumeSlider) MasterVolumeSlider->SetValue(MasterVolume);
	if (MusicVolumeSlider) MusicVolumeSlider->SetValue(MusicVolume);
	if (SfxVolumeSlider) SfxVolumeSlider->SetValue(SfxVolume);
}

void UHronoMainMenuWidget::ApplyAudioSettings(float FadeTime)
{
	USoundMix* ActiveMix = MenuSoundMix;
	if (!ActiveMix)
	{
		ActiveMix = Cast<USoundMix>(GetDefault<UAudioSettings>()->DefaultBaseSoundMix.TryLoad());
	}
	if (!ActiveMix)
	{
		if (!RuntimeSoundMix)
		{
			RuntimeSoundMix = NewObject<USoundMix>(this, TEXT("RuntimeMenuSoundMix"));
		}
		ActiveMix = RuntimeSoundMix;
	}

	USoundClass* ActiveMaster = MasterSoundClass;
	if (!ActiveMaster)
	{
		ActiveMaster = Cast<USoundClass>(GetDefault<UAudioSettings>()->DefaultSoundClassName.TryLoad());
	}

	if (!ActiveMix || !ActiveMaster)
	{
		return;
	}

	if (!bSoundMixPushed)
	{
		UGameplayStatics::PushSoundMixModifier(this, ActiveMix);
		bSoundMixPushed = true;
	}
	UGameplayStatics::SetSoundMixClassOverride(this, ActiveMix, ActiveMaster, MasterVolume, 1.0f, FadeTime, true);
	if (MusicSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(this, ActiveMix, MusicSoundClass, MusicVolume, 1.0f, FadeTime, false);
	}
	if (SfxSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(this, ActiveMix, SfxSoundClass, SfxVolume, 1.0f, FadeTime, false);
	}
}

void UHronoMainMenuWidget::SaveAudioSettings()
{
	UHronoMenuSettingsSaveGame* Save = Cast<UHronoMenuSettingsSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UHronoMenuSettingsSaveGame::StaticClass()));
	if (!Save)
	{
		return;
	}
	Save->MasterVolume = MasterVolume;
	Save->MusicVolume = MusicVolume;
	Save->SfxVolume = SfxVolume;
	UGameplayStatics::SaveGameToSlot(Save, AudioSettingsSlot, 0);
}

void UHronoMainMenuWidget::ShowMainMenu()
{
	if (PageSwitcher && PageSwitcher->GetNumWidgets() > 0)
	{
		PageSwitcher->SetActiveWidgetIndex(0);
	}
}

void UHronoMainMenuWidget::ShowOptions()
{
	PopulateGraphicsSettings();
	OriginalMasterVolume = MasterVolume;
	OriginalMusicVolume = MusicVolume;
	OriginalSfxVolume = SfxVolume;
	RefreshControlsList();
	if (PageSwitcher && PageSwitcher->GetNumWidgets() > 1)
	{
		PageSwitcher->SetActiveWidgetIndex(1);
	}
}

void UHronoMainMenuWidget::ApplySettings()
{
	UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (Settings)
	{
		if (ResolutionCombo)
		{
			TArray<FString> Parts;
			ResolutionCombo->GetSelectedOption().ParseIntoArray(Parts, TEXT("x"), true);
			if (Parts.Num() == 2)
			{
				Settings->SetScreenResolution(FIntPoint(FCString::Atoi(*Parts[0]), FCString::Atoi(*Parts[1])));
			}
		}
		if (WindowModeCombo)
		{
			const FString Mode = WindowModeCombo->GetSelectedOption();
			Settings->SetFullscreenMode(Mode == TEXT("Fullscreen") ? EWindowMode::Fullscreen :
				Mode == TEXT("Borderless") ? EWindowMode::WindowedFullscreen : EWindowMode::Windowed);
		}
		if (QualityCombo)
		{
			const TMap<FString, int32> QualityLevels = {
				{ TEXT("Low"), 0 }, { TEXT("Medium"), 1 }, { TEXT("High"), 2 },
				{ TEXT("Epic"), 3 }, { TEXT("Cinematic"), 4 }
			};
			if (const int32* Level = QualityLevels.Find(QualityCombo->GetSelectedOption()))
			{
				Settings->SetOverallScalabilityLevel(*Level);
			}
		}
		if (FrameRateCombo)
		{
			const FString Selected = FrameRateCombo->GetSelectedOption();
			Settings->SetFrameRateLimit(Selected == TEXT("Unlimited") ? 0.0f : FCString::Atof(*Selected));
		}
		if (VSyncCheckBox)
		{
			Settings->SetVSyncEnabled(VSyncCheckBox->IsChecked());
		}
		Settings->ApplySettings(false);
		Settings->SaveSettings();
	}

	ApplyAudioSettings(0.1f);
	SaveAudioSettings();
	OriginalMasterVolume = MasterVolume;
	OriginalMusicVolume = MusicVolume;
	OriginalSfxVolume = SfxVolume;
}

void UHronoMainMenuWidget::CancelSettings()
{
	if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		Settings->ResetToCurrentSettings();
	}
	MasterVolume = OriginalMasterVolume;
	MusicVolume = OriginalMusicVolume;
	SfxVolume = OriginalSfxVolume;
	if (MasterVolumeSlider) MasterVolumeSlider->SetValue(MasterVolume);
	if (MusicVolumeSlider) MusicVolumeSlider->SetValue(MusicVolume);
	if (SfxVolumeSlider) SfxVolumeSlider->SetValue(SfxVolume);
	ApplyAudioSettings(0.1f);
	PopulateGraphicsSettings();
}

FText UHronoMainMenuWidget::GetFriendlyActionName(FName ActionName) const
{
	if (const FText* Override = ControlDisplayNameOverrides.Find(ActionName))
	{
		return *Override;
	}

	FString Name = ActionName.ToString();
	Name.RemoveFromStart(TEXT("IA_"));
	Name.ReplaceInline(TEXT("_"), TEXT(" "));
	return FText::FromString(Name);
}

void UHronoMainMenuWidget::RefreshControlsList()
{
	if (!ControlsList || !WidgetTree)
	{
		return;
	}
	ControlsList->ClearChildren();

	TMap<FName, TArray<FString>> Bindings;
	for (const TSoftObjectPtr<UInputMappingContext>& ContextReference : ControlMappingContexts)
	{
		const UInputMappingContext* Context = ContextReference.LoadSynchronous();
		if (!Context)
		{
			continue;
		}
		for (const FEnhancedActionKeyMapping& Mapping : Context->GetMappings())
		{
			if (!Mapping.Action || !Mapping.Key.IsValid())
			{
				continue;
			}
			Bindings.FindOrAdd(Mapping.Action->GetFName()).AddUnique(Mapping.Key.GetDisplayName().ToString());
		}
	}

	TArray<FName> ActionNames;
	Bindings.GetKeys(ActionNames);
	ActionNames.Sort(FNameLexicalLess());
	for (const FName ActionName : ActionNames)
	{
		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
		if (UVerticalBoxSlot* PanelSlot = ControlsList->AddChildToVerticalBox(Row))
		{
			PanelSlot->SetPadding(FMargin(0.0f, 5.0f));
		}
		UTextBlock* ActionLabel = HronoMenu::MakeText(WidgetTree, MenuFont, GetFriendlyActionName(ActionName), 16);
		if (UHorizontalBoxSlot* PanelSlot = Row->AddChildToHorizontalBox(ActionLabel))
		{
			PanelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		}
		UTextBlock* KeysLabel = HronoMenu::MakeText(
			WidgetTree, MenuFont, FText::FromString(FString::Join(Bindings[ActionName], TEXT(" / "))), 16);
		KeysLabel->SetJustification(ETextJustify::Right);
		if (UHorizontalBoxSlot* PanelSlot = Row->AddChildToHorizontalBox(KeysLabel))
		{
			PanelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		}
	}

	if (Bindings.IsEmpty())
	{
		ControlsList->AddChildToVerticalBox(HronoMenu::MakeText(
			WidgetTree, MenuFont, FText::FromString(TEXT("Assign an Input Mapping Context in the widget defaults.")), 15));
	}
}

void UHronoMainMenuWidget::HandleCreateSessionClicked()
{
	BeginSessionRequest(true);
}

void UHronoMainMenuWidget::HandleJoinSessionClicked()
{
	BeginSessionRequest(false);
}

void UHronoMainMenuWidget::BeginSessionRequest(bool bCreateSession)
{
	if (bSessionRequestPending)
	{
		return;
	}

	bSessionRequestPending = true;
	bPendingCreateSession = bCreateSession;

	if (bPreloadBeforeSessionRequest)
	{
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (UHronoLoadingSubsystem* LoadingSubsystem = GameInstance->GetSubsystem<UHronoLoadingSubsystem>())
			{
				LoadingSubsystem->PreloadGameplayContent(
					GameplayMapToPreload, AdditionalGameplayAssetsToPreload);
				return;
			}
		}
	}

	DispatchPendingSessionRequest();
}

void UHronoMainMenuWidget::HandleGameplayPreloadCompleted(bool bSuccess)
{
	if (!bSessionRequestPending)
	{
		return;
	}

	if (!bSuccess)
	{
		bSessionRequestPending = false;
		UE_LOG(LogTemp, Error,
			TEXT("Session request cancelled because gameplay content failed to preload."));
		return;
	}

	DispatchPendingSessionRequest();
}

void UHronoMainMenuWidget::DispatchPendingSessionRequest()
{
	if (!bSessionRequestPending)
	{
		return;
	}

	const bool bCreateSession = bPendingCreateSession;
	bSessionRequestPending = false;

	if (bCreateSession)
	{
		OnCreateSessionRequested.Broadcast();
		BP_CreateSessionRequested();
	}
	else
	{
		OnJoinSessionRequested.Broadcast();
		BP_JoinSessionRequested();
	}
}

void UHronoMainMenuWidget::HandleOptionsClicked()
{
	ShowOptions();
}

void UHronoMainMenuWidget::HandleExitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

void UHronoMainMenuWidget::HandleApplyClicked()
{
	ApplySettings();
}

void UHronoMainMenuWidget::HandleBackClicked()
{
	CancelSettings();
	ShowMainMenu();
}

void UHronoMainMenuWidget::HandleButtonPressed()
{
	if (ButtonPressSound)
	{
		UGameplayStatics::PlaySound2D(this, ButtonPressSound, ButtonPressSoundVolume);
	}
}

void UHronoMainMenuWidget::HandleMasterVolumeChanged(float Value)
{
	MasterVolume = Value;
	ApplyAudioSettings(0.05f);
}

void UHronoMainMenuWidget::HandleMusicVolumeChanged(float Value)
{
	MusicVolume = Value;
	ApplyAudioSettings(0.05f);
}

void UHronoMainMenuWidget::HandleSfxVolumeChanged(float Value)
{
	SfxVolume = Value;
	ApplyAudioSettings(0.05f);
}
